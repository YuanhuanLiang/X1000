/* 
 * File:   cc110x.h
 * Author: elinux
 *
 * Created on 2015年4月7日, 上午10:32
 */

#ifndef _LINUX_CC1101_H
#define _LINUX_CC1101_H

#define CC1101_DRV_NAME     "cc1101"
#define CC1101_CHN_SET		0xE001
#define CC1101_CHN_GET		0xE002

// 配置寄存器定义
#define CCxxx0_IOCFG2       0x00 // GDO2输出脚配置. GDO2 output pin configuration  
#define CCxxx0_IOCFG1       0x01 // GDO1输出脚配置. GDO1 output pin configuration  
#define CCxxx0_IOCFG0       0x02 // GDO0输出脚配置. GDO0 output pin configuration  
#define CCxxx0_FIFOTHR      0x03 // RX FIFO和TX FIFO阈值. RX FIFO and TX FIFO thresholds  
#define CCxxx0_SYNC1        0x04 // 同步字,高8位. Sync word, high u8  
#define CCxxx0_SYNC0        0x05 // 同步字,低8位. Sync word, low u8  
#define CCxxx0_PKTLEN       0x06 // 数据包长度. Packet length  
#define CCxxx0_PKTCTRL1     0x07 // 数据包自动控制1. Packet automation control  
#define CCxxx0_PKTCTRL0     0x08 // 数据包自动控制0. Packet automation control  
#define CCxxx0_ADDR         0x09 // 设备地址. Device address  
#define CCxxx0_CHANNR       0x0A // 信道数. Channel number  
#define CCxxx0_FSCTRL1      0x0B // 频率合成器控制1. Frequency synthesizer control  
#define CCxxx0_FSCTRL0      0x0C // 频率合成器控制0. Frequency synthesizer control  
#define CCxxx0_FREQ2        0x0D // 频率控制字，高8位. Frequency control word, high u8  
#define CCxxx0_FREQ1        0x0E // 频率控制字，中8位. Frequency control word, middle u8  
#define CCxxx0_FREQ0        0x0F // 频率控制字，低8位. Frequency control word, low u8  
#define CCxxx0_MDMCFG4      0x10 // 调制解调器配置4. Modem configuration  
#define CCxxx0_MDMCFG3      0x11 // 调制解调器配置3. Modem configuration  
#define CCxxx0_MDMCFG2      0x12 // 调制解调器配置2. Modem configuration  
#define CCxxx0_MDMCFG1      0x13 // 调制解调器配置1. Modem configuration  
#define CCxxx0_MDMCFG0      0x14 // 调制解调器配置0. Modem configuration  
#define CCxxx0_DEVIATN      0x15 // 调制解调器偏差设定. Modem deviation setting  
#define CCxxx0_MCSM2        0x16 // 主通信控制状态机配置2. Main Radio Control State Machine configuration  
#define CCxxx0_MCSM1        0x17 // 主通信控制状态机配置1. Main Radio Control State Machine configuration  
#define CCxxx0_MCSM0        0x18 // 主通信控制状态机配置0. Main Radio Control State Machine configuration  
#define CCxxx0_FOCCFG       0x19 // 频率偏移补偿配置. Frequency Offset Compensation configuration  
#define CCxxx0_BSCFG        0x1A // 位同步配置. Bit Synchronization configuration  
#define CCxxx0_AGCCTRL2     0x1B // AGC控制2. AGC control  
#define CCxxx0_AGCCTRL1     0x1C // AGC控制1. AGC control  
#define CCxxx0_AGCCTRL0     0x1D // AGC控制0. AGC control  
#define CCxxx0_WOREVT1      0x1E // 高8位 事件0超时. High u8 Event 0 timeout  
#define CCxxx0_WOREVT0      0x1F // 低8位 事件0超时. Low u8 Event 0 timeout  
#define CCxxx0_WORCTRL      0x20 // 激活无线电控制. Wake On Radio control  
#define CCxxx0_FREND1       0x21 // 前端RX配置1. Front end RX configuration  
#define CCxxx0_FREND0       0x22 // 前端RX配置0. Front end TX configuration  
#define CCxxx0_FSCAL3       0x23 // 频率合成器校准3. Frequency synthesizer calibration  
#define CCxxx0_FSCAL2       0x24 // 频率合成器校准2. Frequency synthesizer calibration  
#define CCxxx0_FSCAL1       0x25 // 频率合成器校准1. Frequency synthesizer calibration  
#define CCxxx0_FSCAL0       0x26 // 频率合成器校准0. Frequency synthesizer calibration  
#define CCxxx0_RCCTRL1      0x27 // RC振荡器配置1. RC oscillator configuration  
#define CCxxx0_RCCTRL0      0x28 // RC振荡器配置1. RC oscillator configuration  
#define CCxxx0_FSTEST       0x29 // 频率合成器校准控制. Frequency synthesizer calibration control  
#define CCxxx0_PTEST        0x2A // 产品测试. Production test  
#define CCxxx0_AGCTEST      0x2B // AGC测试. AGC test  
#define CCxxx0_TEST2        0x2C // 各种测试设置. Various test settings  
#define CCxxx0_TEST1        0x2D // Various test settings  
#define CCxxx0_TEST0        0x2E // Various test settings  
// Strobe commands   命令滤波定义
#define CCxxx0_SRES         0x30 // (重启芯片)Reset chip
#define CCxxx0_SFSTXON      0x31 // (开启和校准频率合成器)Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). 
// If in RX/TX: Go to a wait state where only the synthesizer is  
// running (for quick RX / TX turnaround).  
#define CCxxx0_SXOFF        0x32 // (关闭晶体振荡器)Turn off crystal oscillator.
#define CCxxx0_SCAL         0x33 // (校准频率合成器并将其关闭)Calibrate frequency synthesizer and turn it off  
// (enables quick start).  
#define CCxxx0_SRX          0x34 // (启用RX)Enable RX. Perform calibration first if coming from IDLE and  
// MCSM0.FS_AUTOCAL=1.  
#define CCxxx0_STX          0x35 // (空闲状态：启用TX)In IDLE state: Enable TX. Perform calibration first if  
// MCSM0.FS_AUTOCAL=1. If in RX state and CCA is enabled:  
// Only go to TX if channel is clear.  
#define CCxxx0_SIDLE        0x36 // (离开RX/TX)Exit RX / TX, turn off frequency synthesizer and exit  
// Wake-On-Radio mode if applicable.  
#define CCxxx0_SAFC         0x37 // (频率合成器的AFC调节)Perform AFC adjustment of the frequency synthesizer  
#define CCxxx0_SWOR         0x38 // (自动RX选举序列（电磁波激活）)Start automatic RX polling sequence (Wake-on-Radio)  
#define CCxxx0_SPWD         0x39 // (当CSn为高时进入功率降低模式 )Enter power down mode when CSn goes high.  
#define CCxxx0_SFRX         0x3A // (冲洗RX FIFO缓冲)Flush the RX FIFO buffer.  
#define CCxxx0_SFTX         0x3B // (冲洗TX FIFO缓冲)Flush the TX FIFO buffer.  
#define CCxxx0_SWORRST      0x3C // (重新设置真实时间时钟)Reset real time clock.  
#define CCxxx0_SNOP         0x3D // (无操作)No operation. May be used to pad strobe commands to two  
// u8s for simpler software.  
//**状态寄存器定义**
#define CCxxx0_PARTNUM      0x30 // CC2550的组成部分数目************/ 
#define CCxxx0_VERSION      0x31 // 当前版本数**********************/ 
#define CCxxx0_FREQEST      0x32 // 率偏移估计**********************/ 
#define CCxxx0_LQI          0x33 // 接质量的解调器估计**************/ 
#define CCxxx0_RSSI         0x34 // 接收信号强度指示****************/ 
#define CCxxx0_MARCSTATE    0x35 // 控制状态机状态******************/ 
#define CCxxx0_WORTIME1     0x36 // WOR计时器高字节*****************/
#define CCxxx0_WORTIME0     0x37 // WOR计时器低字节*****************/ 
#define CCxxx0_PKTSTATUS    0x38 // 当前GDOx状态和数据包状态********/ 
#define CCxxx0_VCO_VC_DAC   0x39 // PLL校准模块的当前设定***********/ 
#define CCxxx0_TXBYTES      0x3A // TX FIFO中的下溢和比特数*********/ 
#define CCxxx0_RXBYTES      0x3B // RX FIFO中的下溢和比特数*********/ 
#define CCxxx0_PATABLE      0x3E // 设置发射功率, 有一个8字节的表
#define CCxxx0_TXFIFO       0x3F // 单字节访问 TX FIFO
#define CCxxx0_RXFIFO       0x3F // 单字节访问 TX FIFO

#define WRITE_BURST         0x40 //连续写入
#define READ_SINGLE         0x80 //读
#define READ_BURST          0xC0 //连续读
#define BYTES_IN_RXFIFO     0x7F //接收缓冲区的有效字节数
#define CRC_OK              0x80 //CRC校验通过位标志

#define CC1101_SPI_BUFLEN 	64

struct cc1101_dev{
	dev_t ccid;
	int major;
	int minor;
    struct cdev chd;
	struct class *cls; 
	struct device *dev;
};
/*
 * Some registers must be read back to modify.
 * To save time we cache them here in memory.
 */
struct cc1101_data {
    int gpio_rdy;	// 连 cc1101 的 GD0 注册为 中断管脚
    int irq;
	int recvflag;	// crc 校验值
	int workflag; 	// work 执行标志
	int alreadySent;	// 是否发送过数据的标记
	
	struct cc1101_dev   cc_dev;
    struct spi_device   *spi;
	struct spi_message  msg;
    struct spi_transfer xfer;
	struct mutex        dev_lock;

	struct work_struct work;
	struct workqueue_struct *workqueue;
	unsigned char spi_rxbuf[CC1101_SPI_BUFLEN];
	unsigned char spi_txbuf[CC1101_SPI_BUFLEN];

	int spi_recvlen; // spi接受多少字节	
};

#endif    /* CC110X_H */
