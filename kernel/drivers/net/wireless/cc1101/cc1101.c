/*
 * Copyright (C) 2019 Chirdtech Semiconductor
 *
 * Liangyh <liangyh@chird.cn>
 *
 * Release under GPLv2
 *
 */

#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <soc/gpio.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/cdev.h>
#include <linux/efi.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/spi/spi.h>
#include "cc1101.h"


static DECLARE_WAIT_QUEUE_HEAD(g_read_waitqueue);

#if 0
unsigned char txrxpower[8] = {0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04 ,0x04}; // -30dB
unsigned char txrxpower[8] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60}; // 0dB
#else
unsigned char txrxpower[8] = {0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0 ,0xC0}; // +10dB
#endif


unsigned char CC1101_RF_REG[] = {
	CCxxx0_IOCFG2,	0x2E,
	CCxxx0_IOCFG1,	0x2E,
	CCxxx0_IOCFG0,	0x06,
	CCxxx0_FIFOTHR,	0x06,//
	CCxxx0_SYNC1,	0xD3,
	CCxxx0_SYNC0,	0x91,
	CCxxx0_PKTLEN,	0x25,
	CCxxx0_PKTCTRL1,0x04,
	CCxxx0_PKTCTRL0,0x05,
	CCxxx0_ADDR,	0x09,
	CCxxx0_CHANNR,	0xCC,
	CCxxx0_FSCTRL1,	0x08,
	CCxxx0_FSCTRL0,	0x00,
	CCxxx0_FREQ2,	0x21,
	CCxxx0_FREQ1,	0xD8,
	CCxxx0_FREQ0,	0x9D,
	CCxxx0_MDMCFG4,	0x9A,
	CCxxx0_MDMCFG3,	0x83,
	CCxxx0_MDMCFG2,	0x13,
	CCxxx0_MDMCFG1,	0x22,
	CCxxx0_MDMCFG0,	0xF8,
	CCxxx0_DEVIATN,	0x47,
	CCxxx0_MCSM2,	0x07,
	CCxxx0_MCSM1,	0x00,
	CCxxx0_MCSM0,	0x18,
	CCxxx0_FOCCFG,	0x1D,
	CCxxx0_BSCFG,	0x1C,
	CCxxx0_AGCCTRL2,0xC7,
	CCxxx0_AGCCTRL1,0x00,
	CCxxx0_AGCCTRL0,0xB2,
	CCxxx0_WOREVT1,	0x87,
	CCxxx0_WOREVT0,	0x6B,
	CCxxx0_WORCTRL,	0xF8,
	CCxxx0_FREND1,	0x56,
	CCxxx0_FREND0,	0x10,
	CCxxx0_FSCAL3,	0xEA,//
	CCxxx0_FSCAL2,	0x2A,
	CCxxx0_FSCAL1,	0x00,
	CCxxx0_FSCAL0,	0x11,//
	CCxxx0_RCCTRL1,	0x0D,
	CCxxx0_RCCTRL0,	0x0D,
	CCxxx0_FSTEST,	0x59,
	CCxxx0_PTEST,	0x3F,
	CCxxx0_AGCTEST,	0xE7,
	CCxxx0_TEST2,	0x81,
	CCxxx0_TEST1,	0x35,
	CCxxx0_TEST0,	0x0B//
};

/*******************************************************************************
函数功能：SPI 数据传输(每次写入和读取的数据长度一致，即. 交换数据)
*******************************************************************************/
static int spi_transfer(struct cc1101_data *cc_data,
						 unsigned char *tx_buf,
						 unsigned char *rx_buf,
						 int len)
{
	int err = 0;
	struct spi_device *spi = cc_data->spi;

	mutex_lock(&cc_data->dev_lock);

	cc_data->xfer.rx_buf    	= rx_buf;
	cc_data->xfer.tx_buf		= tx_buf;
	cc_data->xfer.len			= len;
	cc_data->xfer.bits_per_word	= 8;
	cc_data->xfer.delay_usecs	= 1;
	
	spi_message_init(&cc_data->msg);
	spi_message_add_tail(&cc_data->xfer, &cc_data->msg);
	err = spi_sync(cc_data->spi, &cc_data->msg);
	if (err < 0) {
		dev_err(&spi->dev, "spi transfer failed error [%d]!\n", err);
	}
	mutex_unlock(&cc_data->dev_lock);

	return err;
}

/*******************************************************************************
函数功能：写寄存器
*******************************************************************************/
int halSpiStrobe(struct cc1101_data *cc_data, unsigned char dat)
{
	unsigned char txbuf = dat;
	unsigned char rxbuf;
	
	return spi_transfer(cc_data, &txbuf, &rxbuf, 1);
}

/*******************************************************************************
函数功能：设置状态寄存器的值
*******************************************************************************/
int halSpiWriteReg(struct cc1101_data *cc_data, unsigned char addr, unsigned char val)
{
	unsigned char txbuf[2] = {0};
	unsigned char rxbuf[2] = {0};
	txbuf[0] = addr;
	txbuf[1] = val;
	
	return spi_transfer(cc_data, txbuf, rxbuf, 2);
}

/*******************************************************************************
函数功能：连续写 配置寄存器或者写入发送数据
*******************************************************************************/
int halSpiWriteBurstReg(struct cc1101_data *cc_data, unsigned char addr, unsigned char *databuf, unsigned char datalen)
{
#if 1
	unsigned char rxbuf[CC1101_SPI_BUFLEN];

	memset(cc_data->spi_txbuf, 0, CC1101_SPI_BUFLEN);
	cc_data->spi_txbuf[0] = (addr | WRITE_BURST);
	cc_data->spi_txbuf[1] = datalen;
	memcpy(cc_data->spi_txbuf+2, databuf, datalen);
	return spi_transfer(cc_data, cc_data->spi_txbuf, rxbuf, datalen+2);
#else
	unsigned char txbuf[datalen+2];	
	memset(txbuf, 0, datalen+2);
	txbuf[0] = (addr | WRITE_BURST);
	txbuf[1] = datalen;
	memcpy(txbuf+2, databuf, datalen);
	return spi_transfer(cc_data, txbuf, NULL, datalen+2);
#endif

}

/*******************************************************************************
函数功能：读取寄存器的值
*******************************************************************************/
unsigned char halSpiReadReg(struct cc1101_data *cc_data, unsigned char addr)
{
	unsigned char txbuf[2];
	unsigned char rxbuf[2];
	
	txbuf[0] = (addr | READ_SINGLE);

	spi_transfer(cc_data, txbuf, rxbuf, 2);
	
	return rxbuf[1];
}

/*******************************************************************************
函数功能：连续读，读取寄存器的值或者接收数据
*******************************************************************************/
int halSpiReadBurstReg(struct cc1101_data *cc_data, unsigned char addr, unsigned char *rxbuf, unsigned char datalen)
{
	unsigned char txbuf[CC1101_SPI_BUFLEN];
	
	memset(txbuf, 0, CC1101_SPI_BUFLEN);
	
	txbuf[0] = (addr | READ_BURST);

	if((datalen + 1) > CC1101_SPI_BUFLEN ){
		dev_err(&cc_data->spi->dev, " data length to large, must <= %d !\n", CC1101_SPI_BUFLEN);
		return -1;
	}
	
	spi_transfer(cc_data, txbuf, rxbuf, datalen + 1);
	
	return 0;
}

/*******************************************************************************
函数功能：读取状态寄存器的值
*******************************************************************************/
unsigned char halSpiReadStatus(struct cc1101_data *cc_data, unsigned char addr) 
{
	unsigned char txbuf[2];
	unsigned char rxbuf[2];
	
	txbuf[0] = (addr | READ_BURST);

	spi_transfer(cc_data, txbuf, rxbuf, 2);
	
	return rxbuf[1];
}

/*******************************************************************************
函数功能：复位cc1101
*******************************************************************************/
void RESET_CC1100(struct cc1101_data *cc_data)
{ 
	halSpiStrobe(cc_data, CCxxx0_SRES); 
}


/*******************************************************************************
函数功能：初始化 CC1101 RF 参数，设置一系列状态寄存器
*******************************************************************************/
void halRfWriteRfSettings(struct cc1101_data *cc_data)
{
	halSpiStrobe(cc_data, CCxxx0_SRES);  // 0x30
	halSpiStrobe(cc_data, CCxxx0_SIDLE); // 0x36
	halSpiStrobe(cc_data, CCxxx0_SCAL);  // 0x33

	spi_transfer(cc_data, CC1101_RF_REG, NULL, sizeof(CC1101_RF_REG));

	halSpiWriteReg(cc_data, CCxxx0_PATABLE,	0xCD); //
	halSpiStrobe(cc_data, CCxxx0_SPWD);  // 0x39
}

/*******************************************************************************
函数功能：发送数据
*******************************************************************************/
int halRfSendPacket(struct cc1101_data *cc_data, unsigned char *txbuf, unsigned char size)
{
	halSpiStrobe(cc_data, CCxxx0_SIDLE); // 进入空闲模式
	halSpiStrobe(cc_data, CCxxx0_SFTX);  // 清空发射缓冲区
	halSpiWriteBurstReg(cc_data, CCxxx0_TXFIFO, txbuf, size);  //写入要发送的数据
	halSpiStrobe(cc_data, CCxxx0_STX);		          //进入发送模式发送数据

	// when send, high time is 5ms 当开始发送时,高电平持续5ms
	int i, flag = 0;
	
	for(i = 0; i < 50; i++){// Wait for GDO0 -> start send, GD0 set high 等待GD0高电平
		if(gpio_get_value(cc_data->gpio_rdy)){
			flag = 1;
			break;
		}
		usleep_range(100, 120);
	}
	
	if(flag == 1){ // 获取到高电平了，说明已经开始发送了
		for(i = 0; i < 100; i++){ // Wait for GDO0 -> end send,GD0 set low 等待发送结束,GD0 恢复低电平
			if(!gpio_get_value(cc_data->gpio_rdy)){// 恢复低电平, break
				break;
			}
			usleep_range(100, 120);
		}
	}

	halSpiStrobe(cc_data, CCxxx0_SPWD);//0x39

	if(flag == 1){
		return 0;
	}
	else{
		return -EAGAIN; // 发送失败，再次尝试一次
	}
}

/*******************************************************************************
函数功能：接收数据
*******************************************************************************/
unsigned char halRfReceivePacket(struct cc1101_data *cc_data, unsigned char *rxbuf, int *count)
{
	unsigned char status[2];
	unsigned char pkglen ;		// 帧数据长度


	if(halSpiReadStatus(cc_data, CCxxx0_RXBYTES)&BYTES_IN_RXFIFO) {//如果接的字节数不为0
		pkglen = halSpiReadReg(cc_data, CCxxx0_RXFIFO); //读出第一个字节，此字节为该帧数据长度
		*count = pkglen;
		/* 注意: 接收到的第一个数据不知道从哪里来，导致少接收了一个字节
		因此pkglen+1，还需要测试，可能是电脑端测试软件的问题 */
		halSpiReadBurstReg(cc_data, CCxxx0_RXFIFO, rxbuf, pkglen+1); //读出所有接收到的数据

		// Read the 2 appended status bytes (status[0] = RSSI, status[1] = LQI)
		halSpiReadBurstReg(cc_data, CCxxx0_RXFIFO, status, 2);	//读出CRC校验位 
		
		/* reset RX mode */
		halSpiStrobe(cc_data, CCxxx0_SIDLE); // 进入空闲
		halSpiStrobe(cc_data, CCxxx0_SFRX);  // clear RX 清空接受缓冲区
		halSpiStrobe(cc_data, CCxxx0_SRX);   // 进入接收状态
		
	    return (status[1] & CRC_OK); 
    }
	
	return 0;
}

static void cc1101_work(struct work_struct *work)
{
    struct cc1101_data *cc_data = container_of(work, struct cc1101_data, work);

    int ret;
	memset(cc_data->spi_rxbuf, 0, CC1101_SPI_BUFLEN);
	
	ret = halRfReceivePacket(cc_data, cc_data->spi_rxbuf, &cc_data->spi_recvlen);
	if(ret != 0){
		cc_data->recvflag = cc_data->spi_recvlen;
	}
	else{
		cc_data->recvflag = -1;
	}

	cc_data->alreadySent = 1;

	wake_up(&g_read_waitqueue);	 // 唤醒read 函数
}



static irqreturn_t cc1101_rcv_interrupt(int irq, void *dev_id)
{   
    struct cc1101_data *cc_data = (struct cc1101_data *)dev_id ;
	
    queue_work(cc_data->workqueue, &cc_data->work); /* 将工作加入到工作队列中，最终唤醒内核线程 */

    return IRQ_HANDLED;
}

static long cc1101_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct cc1101_data *cc_data = filp->private_data;
    int channel;
    int ret = 0;

    switch (cmd) {
        case CC1101_CHN_SET:
			ret = copy_from_user(&channel, (unsigned int*)arg, sizeof(unsigned int));
			halSpiWriteReg(cc_data, CCxxx0_CHANNR, (channel&0xFF)); 
			break;
		case CC1101_CHN_GET:
			channel = halSpiReadReg(cc_data, CCxxx0_CHANNR); 
			ret = copy_to_user((unsigned int*)arg, &channel, sizeof(unsigned int));
			break;
        default:
            ret = -EINVAL;
            dev_err(&cc_data->spi->dev, "cc1101_ioctl no such cmd.");
    }

	return ret;
}


static ssize_t cc1101_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct cc1101_data *cc_data = (struct cc1101_data *)filp->private_data;
	int ret = 0;
	
	unsigned char tx_buf[CC1101_SPI_BUFLEN-1] = {0};

	ret = copy_from_user(tx_buf, buf, count);
	if(ret){
		dev_err(&cc_data->spi->dev, "copy form user failed");
        ret = -EFAULT;
	}
	
    ret = halRfSendPacket(cc_data, tx_buf, count);
	if(ret == 0){
		ret = count;
	}
	
    return ret;
}


static ssize_t cc1101_waitqueue_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct cc1101_data *cc_data = (struct cc1101_data *)filp->private_data;
    int ret = 0;

	cc_data->spi_recvlen = count; // 设置接受长度
	
	/* reset RX mode */
	halSpiStrobe(cc_data, CCxxx0_SIDLE); // 进入空闲
	halSpiStrobe(cc_data, CCxxx0_SFRX);  // clear RX 清空接受缓冲区
	halSpiStrobe(cc_data, CCxxx0_SRX);   // 进入接收状态

	// enable_irq(cc_data->irq);
	// 初始化中断
	ret = request_irq(cc_data->irq, cc1101_rcv_interrupt, IRQF_TRIGGER_FALLING, "cc1101_gd0_irq", (void *)cc_data);
    if (ret) {
		dev_err(&cc_data->spi->dev, "cc1101 %s request_irq failed!\n", __func__);
        return ret;
    }
	
	ret = wait_event_freezable(g_read_waitqueue, cc_data->alreadySent != 0);	 //等待 被cc1101_work()唤醒
	/* 内核空间->用户空间 */
    ret = copy_to_user(buf, cc_data->spi_rxbuf+1, cc_data->spi_recvlen);
	if(!ret){
		if(cc_data->recvflag < 0){ // cc1101 RX 接收不到数据, crc 校准错误
			ret = -3;
		}
		else{
        	ret = cc_data->spi_recvlen;
		}
	}
	else{
		ret = -3;
		dev_err(&cc_data->spi->dev, "copy_to_user failed.");
	}
	cc_data->alreadySent = 0;

	// disable_irq(cc_data->irq);
	free_irq(cc_data->irq, cc_data); /* 读取完毕 释放中断 */

   return ret;
}

static int cc1101_open(struct inode *inode, struct file *filp)
{
	struct cdev *cdev = inode->i_cdev;
	struct cc1101_dev *cc_dev = container_of(cdev, struct cc1101_dev, chd);
    struct cc1101_data *cc_data = container_of(cc_dev, struct cc1101_data, cc_dev);
    
    filp->private_data = cc_data;

	halSpiStrobe(cc_data, CCxxx0_SIDLE); // 进入空闲
	halSpiStrobe(cc_data, CCxxx0_SFRX);  // clear RX 清空接受缓冲区
	halSpiStrobe(cc_data, CCxxx0_SRX);   // 进入接收状态

    printk("cc1101 open succeed!\n");
    return 0;
}

static int cc1101_close(struct inode *inode, struct file *filp)
{
	struct cc1101_data *cc_data = (struct cc1101_data *)filp->private_data;

	halSpiStrobe(cc_data, CCxxx0_SIDLE); // 进入空闲
	halSpiStrobe(cc_data, CCxxx0_SFRX);  // clear RX 清空接受缓冲区
	
	filp->private_data = NULL;

	printk("cc1101 close!\n");
	return 0;
}

static const struct file_operations cc1101_fops = {
	.owner 			= THIS_MODULE,
	.unlocked_ioctl	= cc1101_ioctl,
	.write			= cc1101_write,
	.read			= cc1101_waitqueue_read,
	.open			= cc1101_open,
	.release		= cc1101_close,
};

static int cc1101_gpio_init(struct cc1101_data *cc_data)
{
    struct spi_device *spi = cc_data->spi;
    int err = 0;

    if (gpio_is_valid(cc_data->gpio_rdy)) {
        err = gpio_request(cc_data->gpio_rdy, "GD0_Reday");
        if (err < 0) {
            dev_err(&spi->dev,"cc1101: [%d] gpio_requests failed.\n", cc_data->gpio_rdy);
        }
		//gpio_direction_input(cc_data->gpio_rdy);
    }

    return err;
}

static int cc1101_suspend(struct spi_device *spi, pm_message_t mesg)
{
#if 0
    struct cc1101_data *cc_data = dev_get_drvdata(&spi->dev);

    cc1101_power_on(cc_data, CC1101_POWER_OFF);
#endif

    return 0;
}

static int cc1101_resume(struct spi_device *spi)
{
#if 0
    struct cc1101_data *cc_data = dev_get_drvdata(&spi->dev);

    cc1101_power_on(cc_data, CC1101_POWER_ON);
#endif
    return 0;
}

/* 初始化 cc1101 */
static int cc1101_setup(struct cc1101_data *cc_data)
{    
    RESET_CC1100(cc_data);				// 上电复位  
    halRfWriteRfSettings(cc_data);		// 配置寄存器
    halSpiWriteBurstReg(cc_data, CCxxx0_PATABLE, txrxpower, 8); // 配置收发功率增益
	halSpiStrobe(cc_data, CCxxx0_SIDLE);//CC1100进入空闲
    return 0;
}

static int cc1101_probe(struct spi_device *spi)
{
	struct cc1101_data *cc_data = NULL;
	int err = -1;

	cc_data = kzalloc(sizeof(struct cc1101_data), GFP_KERNEL);
	if (cc_data == NULL) {
		dev_err(&spi->dev, "cc1101 kmalloc failed.\n");
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	dev_set_drvdata(&spi->dev, cc_data);

	cc_data->spi	  = spi;
	cc_data->gpio_rdy = GPIO_PD(5);
	mutex_init(&cc_data->dev_lock);
	
	err = cc1101_gpio_init(cc_data);
	if (err < 0) {
		dev_err(&spi->dev, "cc1101 gpio init failed.\n");
		goto exit_gpio_init_failed;
	}
	cc_data->irq = gpio_to_irq(cc_data->gpio_rdy);
	
	cc_data->cc_dev.minor = 0;
	err = alloc_chrdev_region(&cc_data->cc_dev.ccid, cc_data->cc_dev.minor, 1, CC1101_DRV_NAME);
	if (err < 0) {
		dev_err(&spi->dev, "cc1101 alloc_chrdev_region failed!");
		goto exit_alloc_chrdev_failed;
	}
	
	cdev_init(&cc_data->cc_dev.chd, &cc1101_fops);
	cc_data->cc_dev.chd.owner = THIS_MODULE;
	cc_data->cc_dev.major = MAJOR(cc_data->cc_dev.ccid);
    cdev_add(&cc_data->cc_dev.chd, cc_data->cc_dev.ccid, 1);

	cc_data->cc_dev.cls = class_create(THIS_MODULE, CC1101_DRV_NAME);
	if (IS_ERR(cc_data->cc_dev.cls)) {
		dev_err(&spi->dev, "cc1101 class create failed!");
		goto exit_class_create_failed;
	}

	cc_data->cc_dev.dev = device_create(cc_data->cc_dev.cls,  NULL, cc_data->cc_dev.ccid, NULL, CC1101_DRV_NAME);
	err = IS_ERR(cc_data->cc_dev.dev) ? PTR_ERR(cc_data->cc_dev.dev) : 0;
	if (err) {
		dev_err(&spi->dev, "cc1101 device_create failed. err = %d", err);
		goto exit_device_create_failed;
	}

	cc_data->spi->max_speed_hz = 1000000; // 通信时钟最大速率
	err = spi_setup(spi);  // 设置SPI
	if(err != 0){
		dev_err(&spi->dev, "cc1101 spi_setup error.\n");
		goto exit_spi_setup_failed;
	}

	//setup cc1101
    cc1101_setup(cc_data);

	/* init work queue for irq */
	INIT_WORK(&cc_data->work, cc1101_work); /* 初始化工作work_struct，指定工作函数 */
	cc_data->workqueue = create_singlethread_workqueue("cc1101_workqueue");/*创建工作队列,该函数会为cpu创建内核线程*/
	if (!cc_data->workqueue) {
		dev_err(&spi->dev, "create_single_workqueue error!\n");
		err = -1;
		goto exit_create_workqueue_failed;
	}	

#if 0
	//IRQF_TRIGGER_FALLING 下降触发   IRQF_TRIGGER_RISING 上升触发
    err = request_irq(cc_data->irq, cc1101_rcv_interrupt, IRQF_TRIGGER_FALLING, "cc1101_gd0_irq", (void *)cc_data);
    if (err) {
		dev_err(&cc_data->spi->dev, "cc1101 %s request_irq failed!\n", __func__);
        goto exit_request_irq_failed;
    }

	disable_irq(cc_data->irq); /* 禁止中断 */
#endif
    printk("[%s] cc1101 init sucessfull.\n", __func__);
    return 0;
#if 0
exit_request_irq_failed:
	destroy_workqueue(cc_data->workqueue);
#endif
exit_create_workqueue_failed:
exit_spi_setup_failed:
    device_destroy(cc_data->cc_dev.cls, cc_data->cc_dev.ccid);
exit_device_create_failed:
    class_destroy(cc_data->cc_dev.cls);
exit_class_create_failed:
    cdev_del(&cc_data->cc_dev.chd);
    unregister_chrdev_region(cc_data->cc_dev.ccid, 1);
exit_alloc_chrdev_failed:
	if (gpio_is_valid(cc_data->gpio_rdy)){
		gpio_free(cc_data->gpio_rdy);
	}
exit_gpio_init_failed:
    kfree(cc_data);
    cc_data = NULL;
exit_alloc_data_failed:
    return err;
}


static int cc1101_remove(struct spi_device *spi)
{
	struct cc1101_data *cc_data = NULL;
	cc_data = spi_get_drvdata(spi);

	disable_irq(cc_data->irq);
#if 0
	free_irq(cc_data->irq, cc_data);
#endif	
	destroy_workqueue(cc_data->workqueue);
	
	device_destroy(cc_data->cc_dev.cls, cc_data->cc_dev.ccid);
	class_destroy(cc_data->cc_dev.cls);
	cdev_del(&cc_data->cc_dev.chd);
	unregister_chrdev_region(cc_data->cc_dev.ccid, 1);
	
	if (gpio_is_valid(cc_data->gpio_rdy)){
		gpio_free(cc_data->gpio_rdy);
	}
	kfree(cc_data);
	cc_data = NULL;

	return 0;
}


/**
 *  the spi struct date start,for getting the spi_device to set the spi clock enable start
 */

struct spi_device_id cc1101_dev_id = {CC1101_DRV_NAME, 0};

struct spi_driver cc1101_drv = {
    .probe          = cc1101_probe,
    .remove         = cc1101_remove,
    .suspend        = cc1101_suspend,
    .resume         = cc1101_resume,
    .id_table       = &cc1101_dev_id,
    .driver = {
        .name       = CC1101_DRV_NAME,
        .bus        = &spi_bus_type,
        .owner      = THIS_MODULE,
    },
};


static int __init cc1101_init(void)
{
    int ret = 0;

    ret = spi_register_driver(&cc1101_drv);
    if(ret) {
        printk("cc1101 spi_register_driver failed.\n");
    }

    return ret;
}

static void __exit cc1101_exit(void)
{
    spi_unregister_driver(&cc1101_drv);
}

module_init(cc1101_init);
module_exit(cc1101_exit);

MODULE_AUTHOR("Ingenic");
MODULE_DESCRIPTION("Driver for microarray fingerprint sensor");
MODULE_LICENSE("GPL");
