/*
 * Copyright (C) 2016 Ingenic Semiconductor
 *
 * YangHuanHuan <huanhuan.yang@ingenic.com>
 *
 * Release under GPLv2
 *
 */

#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <soc/gpio.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/efi.h>
#include <linux/mm.h>
#include <linux/fb.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#include <linux/spi/spi.h>
#include <linux/spi/microarray.h>
#include "ma_ioctl_cmd.h"


//macro settings
#define FINGERPRINT_BUF_LENGTH          (32*1024)
#define FINGERPRINT_KERNEL_MEM_SIZE     (128*1024)

#define SPI_SPEED                       (6*1000000)

//表面类型
#define COVER_T                         (1)
#define COVER_N                         (2)
#define COVER_M                         (3)
#define COVER_NUM                       COVER_N

/*
 * key define   just modify the KEY_FN_* for different platform
 */
#define FINGERPRINT_SWIPE_UP            KEY_FN_F1//827
#define FINGERPRINT_SWIPE_DOWN          KEY_FN_F2//828
#define FINGERPRINT_SWIPE_LEFT          KEY_FN_F3//829
#define FINGERPRINT_SWIPE_RIGHT         KEY_FN_F4//830
#define FINGERPRINT_TAP                 KEY_FN_F5// 831
#define FINGERPRINT_DTAP                KEY_FN_F6//     832
#define FINGERPRINT_LONGPRESS           KEY_FN_F7//833


static DECLARE_WAIT_QUEUE_HEAD(g_screen_waitqueue);
static DECLARE_WAIT_QUEUE_HEAD(g_waitqueue);
static DECLARE_WAIT_QUEUE_HEAD(g_u1_waitqueue);
static DECLARE_WAIT_QUEUE_HEAD(g_u2_waitqueue);
#ifdef COMPATIBLE_VERSION3
static DECLARE_WAIT_QUEUE_HEAD(g_drv_waitqueue);
#endif

enum fp_power {
    FINGERPRINT_POWER_OFF = 0,
    FINGERPRINT_POWER_ON,
};

struct finger_print_dev {
    dev_t idd;
    int major;
    int minor;
    struct cdev chd;
    struct class *cls;
    struct device *dev;
};

/*
 * finger print_spi struct use to save the value
 */
struct microarray_fp_data {
    int power_2v8;
    int power_1v8;
    int power_en;
    int reset;
    int gpio_int;
    int irq;
    struct finger_print_dev fp_dev;
    struct microarray_platform_data *pdata;

    int value;
    unsigned char           do_what;    //工作内容
    unsigned char           f_wake;     //唤醒标志
    volatile unsigned char  f_irq;      //中断标志
    volatile unsigned char  u1_flag;    //reserve for ours thread interrupt
    volatile unsigned char  u2_flag;    //reserve for ours thread interrupt
    volatile unsigned char  f_repo;     //上报开关

    spinlock_t          spi_lock;
    struct spi_device   *spi;
    struct list_head    dev_entry;
    struct spi_message  msg;
    struct spi_transfer xfer;
    struct input_dev    *input;
    struct work_struct  work;
    struct mutex        dev_lock;
    struct mutex        ioctl_lock;
    struct wake_lock    wl;
    unsigned char       spi_tx_buffer[FINGERPRINT_BUF_LENGTH];
    unsigned char       spi_rx_buffer[FINGERPRINT_BUF_LENGTH];
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend suspend;
#endif

    struct wake_lock        process_wakelock;
    struct workqueue_struct *workqueue;
    struct notifier_block   fp_notifier;
    unsigned int            is_screen_on;  // =1: screen is power on   =0: screen is power down
    unsigned int            screen_flag;
    unsigned int            drv_reg;

    void * kernel_memaddr;
    unsigned long kernel_memesize;
};

void microarray_enable_spi(struct spi_device *spi){
    //mt_spi_enable_clk(spi_master_get_devdata(spi->master));
}

void microarray_disable_spi(struct spi_device *spi){
    //mt_spi_disable_clk(spi_master_get_devdata(spi->master));
}

/*
 *  set spi speed, often we must check whether the setting is efficient
 */
void microarray_spi_change(struct microarray_fp_data *fp_data, unsigned int  speed)
{
    int ret = 0;

    fp_data->spi->max_speed_hz  = speed;
    if ( (ret = spi_setup(fp_data->spi)) < 0) {
        dev_err(&fp_data->spi->dev,"change the spi speed[%d] error [%d]!\n", speed, ret);
    }
}

/*
 * microarray write and read data
 * @tx_buf/rx_buf data to write/read
 * @len :data length
 * @retval：=0:   success
 *          =-1   failed
 */
static int microarray_spi_transfer(struct microarray_fp_data *fp_data,
            unsigned char *tx_buf, unsigned char *rx_buf, int len)
{
    int err = 0;
    struct spi_device *spi = fp_data->spi;

    mutex_lock(&fp_data->dev_lock);

    fp_data->xfer.tx_buf    = tx_buf;
    fp_data->xfer.rx_buf    = rx_buf;
    fp_data->xfer.len       = len;
    fp_data->xfer.bits_per_word = spi->bits_per_word;
    fp_data->xfer.speed_hz  = fp_data->spi->max_speed_hz;
    fp_data->xfer.delay_usecs = 1;

    spi_message_init(&fp_data->msg);
    spi_message_add_tail(&fp_data->xfer, &fp_data->msg);
    err = spi_sync(fp_data->spi, &fp_data->msg);
    if (err < 0) {
        dev_err(&spi->dev,"spi transfer failed error [%d]!\n", err);
    }

    mutex_unlock(&fp_data->dev_lock);

    return err;
}

static void microarray_fp_work(struct work_struct *work)
{
    struct microarray_fp_data *fp_data = \
                         container_of(work, struct microarray_fp_data, work);

    fp_data->f_irq = 1;
    wake_up(&g_waitqueue);

#ifdef COMPATIBLE_VERSION3
    wake_up(&g_drv_waitqueue);
#endif
}

static irqreturn_t microarray_fp_interrupt(int irq, void *dev_id)
{
    struct microarray_fp_data *fp_data = dev_id;

    queue_work(fp_data->workqueue, &fp_data->work);

    return IRQ_HANDLED;
}

static int microarray_fp_power_on(struct microarray_fp_data *fp_data, int on_off)
{
    if (on_off) {
        /* micro array power on */
        if (fp_data->power_2v8 >= 0) {
            gpio_direction_output(fp_data->power_2v8, 1);
        }

        if (fp_data->power_1v8 >= 0) {
            gpio_direction_output(fp_data->power_1v8, 1);
        }

        if (fp_data->power_en >= 0) {
            gpio_direction_output(fp_data->power_en, 0);
        }

    } else {
        /* micro array power off */
        if (fp_data->power_2v8 >= 0) {
            gpio_direction_output(fp_data->power_2v8, 0);
        }

        if (fp_data->power_1v8 >= 0) {
            gpio_direction_output(fp_data->power_1v8, 0);
        }

        if (fp_data->power_en >= 0) {
            gpio_direction_output(fp_data->power_en, 1);
        }
    }

    return 0;
}

static int microarray_fp_reset(struct microarray_fp_data *fp_data)
{
    if (fp_data->reset >= 0) {
        gpio_direction_output(fp_data->reset, 0);
        msleep(1);
        gpio_direction_output(fp_data->reset, 1);
        msleep(1);
    }

    return 0;
}

/*
 * this is a demo function,if the power on-off switch by other way
 * modify it as the right way
 * on_off 1 on   0 off
 */
static int microarray_switch_power(struct microarray_fp_data *fp_data, unsigned int on_off)
{
    microarray_fp_power_on(fp_data, on_off);

    return 0;
}

static int microarray_get_interrupt_status(struct microarray_fp_data *fp_data)
{
    unsigned int finger_int_pin = fp_data->gpio_int;

    return gpio_get_value(finger_int_pin);
}


#ifdef COMPATIBLE_VERSION3
static int microarray_fp_version3_ioctl(struct microarray_fp_data *fp_data, int cmd, int arg)
{
    int ret = 0;
    struct spi_device *spi = fp_data->spi;

    dev_dbg(&spi->dev, "%s: start cmd=0x%.3x arg=%d\n", __func__, cmd, arg);

    switch (cmd) {
        case IOCTL_DEBUG:
            //sdeb = (u8) arg;
            break;

        case IOCTL_IRQ_ENABLE:
            break;

        case IOCTL_SPI_SPEED:
            fp_data->spi->max_speed_hz = (u32) arg;
            spi_setup(fp_data->spi);
            break;

        case IOCTL_COVER_NUM:
            ret = COVER_NUM;
            break;

        case IOCTL_GET_VDATE:
            ret = 20160425;
            break;

        case IOCTL_CLR_INTF:
            fp_data->f_irq = FALSE;
            break;

        case IOCTL_GET_INTF:
            ret = fp_data->f_irq;
            break;
        case IOCTL_REPORT_FLAG:
            fp_data->f_repo = arg;
            break;

        case IOCTL_REPORT_KEY:
            input_report_key(fp_data->input, arg, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, arg, 0);
            input_sync(fp_data->input);
            break;

        case IOCTL_SET_WORK:
            fp_data->do_what = arg;
            break;

        case IOCTL_GET_WORK:
            ret = fp_data->do_what;
            break;

        case IOCTL_SET_VALUE:
            fp_data->value = arg;
            break;

        case IOCTL_GET_VALUE:
            ret = fp_data->value;
            break;

        case IOCTL_TRIGGER:
            fp_data->f_wake = TRUE;
            wake_up_interruptible(&g_drv_waitqueue);
            break;

        case IOCTL_WAKE_LOCK:
            if (!wake_lock_active(&fp_data->wl)) {
                wake_lock(&fp_data->wl);
            }
            break;

        case IOCTL_WAKE_UNLOCK:
            if (wake_lock_active(&fp_data->wl)) {
                wake_unlock(&fp_data->wl);
            }
            break;

        case IOCTL_KEY_DOWN:
            input_report_key(fp_data->input, KEY_F11, 1);
            input_sync(fp_data->input);
            break;

        case IOCTL_KEY_UP:
            input_report_key(fp_data->input, KEY_F11, 0);
            input_sync(fp_data->input);
            break;
    }

    dev_dbg(&spi->dev, "%s: end. ret=%d f_irq=%d, f_repo=%d\n", __func__, ret, fp_data->f_irq, fp_data->f_repo);

    return ret;

}
#endif


/*
 * microarray write data
 * @return :count: success
 *         -1    : count to large，
 *         -2    : copy failed
 */
static ssize_t microarray_fp_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct microarray_fp_data *fp_data = filp->private_data;
    int val = 0;
    int ret = 0;

    if (count == 6) {
        /*
         * cmd ioctl, old version used the write interface to do ioctl, this is only for the old version
         * */
        int cmd, arg;
        unsigned char tmp[6];

        ret = copy_from_user(tmp, buf, count);

        cmd = tmp[0];
        cmd <<= 8;
        cmd += tmp[1];
        arg = tmp[2];
        arg <<= 8;
        arg += tmp[3];
        arg <<= 8;
        arg += tmp[4];
        arg <<= 8;
        arg += tmp[5];
#ifdef COMPATIBLE_VERSION3
        val = (int)microarray_fp_version3_ioctl(fp_data, (unsigned int)cmd, (unsigned long)arg);
#endif

    } else {
        memset(fp_data->spi_tx_buffer, 0, FINGERPRINT_BUF_LENGTH);
        ret = copy_from_user(fp_data->spi_tx_buffer, buf, count);
        if (ret) {
            dev_err(&fp_data->spi->dev, "copy form user failed");
            val = -2;
        } else {
            val = count;
        }
    }

    return val;
}

/*
 * micro array_read data
 * @return : count: success,
 *           -1     count too large，
 *           -2     Communication failure,
 *           -3     copy failed
 */
static ssize_t microarray_fp_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct microarray_fp_data *fp_data = filp->private_data;
    int val, ret = 0;
    unsigned char *rx_buf = fp_data->spi_rx_buffer;
    unsigned char *tx_buf = fp_data->spi_tx_buffer;

    ret = microarray_spi_transfer(fp_data, tx_buf, rx_buf, count);
    if(ret) {
        dev_err(&fp_data->spi->dev, "microarray read failed.");
        return -2;
    }

    ret = copy_to_user(buf, rx_buf, count);
    if(!ret)
        val = count;
    else {
        val = -3;
        dev_err(&fp_data->spi->dev, "copy_to_user failed.");
    }


    return val;
}

static long microarray_fp_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct microarray_fp_data *fp_data = filp->private_data;
    unsigned int int_pin_state;
    unsigned int microarray_spi_speed;
    int ret = 0;

    switch (cmd) {
        case TIMEOUT_WAKELOCK:
            //timeout lock
            wake_lock_timeout(&fp_data->process_wakelock, 5*HZ);
            break;

        case SLEEP:
            //remove the process out of the runqueue
            fp_data->f_irq = 0;
            ret = wait_event_freezable(g_waitqueue, fp_data->f_irq != 0);
            break;

        case WAKEUP:
            //wake up, schedule the process into the runqueue
            fp_data->f_irq = 1;
            wake_up(&g_waitqueue);
            break;

        case ENABLE_CLK:
            //if the spi clock is not opening always, do this methods
            microarray_enable_spi(fp_data->spi);
            break;

        case DISABLE_CLK:
            //disable the spi clock
            microarray_disable_spi(fp_data->spi);
            break;

        case ENABLE_INTERRUPT:
            //enable the irq,in fact, you can make irq enable always
            enable_irq(fp_data->irq);
            break;

        case DISABLE_INTERRUPT:
            //disable the irq
            disable_irq(fp_data->irq);
            break;

        case TAP_DOWN:
            input_report_key(fp_data->input, FINGERPRINT_TAP, 1);
            input_sync(fp_data->input);
            break;

        case TAP_UP:
            input_report_key(fp_data->input, FINGERPRINT_TAP, 0);
            input_sync(fp_data->input);
            break;

        case SINGLE_TAP:
            input_report_key(fp_data->input, FINGERPRINT_TAP, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_TAP, 0);
            input_sync(fp_data->input);
            break;

        case DOUBLE_TAP:
            input_report_key(fp_data->input, FINGERPRINT_DTAP, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_DTAP, 0);
            input_sync(fp_data->input);
            break;

        case LONG_TAP:
            input_report_key(fp_data->input, FINGERPRINT_LONGPRESS, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_LONGPRESS, 0);
            input_sync(fp_data->input);
            break;

        case MA_KEY_UP:
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_UP, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_UP, 0);
            input_sync(fp_data->input);
            break;

        case MA_KEY_LEFT:
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_LEFT, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_LEFT, 0);
            input_sync(fp_data->input);
            break;

        case MA_KEY_DOWN:
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_DOWN, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_DOWN, 0);
            input_sync(fp_data->input);
            break;

        case MA_KEY_RIGHT:
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_RIGHT, 1);
            input_sync(fp_data->input);
            input_report_key(fp_data->input, FINGERPRINT_SWIPE_RIGHT, 0);
            input_sync(fp_data->input);
            break;

        case SET_MODE:
            mutex_lock(&fp_data->ioctl_lock);
            ret = copy_from_user(&fp_data->drv_reg, (unsigned int*)arg, sizeof(unsigned int));
            mutex_unlock(&fp_data->ioctl_lock);
            break;

        case GET_MODE:
            mutex_lock(&fp_data->ioctl_lock);
            ret = copy_to_user((unsigned int*)arg, &fp_data->drv_reg, sizeof(unsigned int));
            mutex_unlock(&fp_data->ioctl_lock);
            break;

        case MA_IOC_GVER:
            mutex_lock(&fp_data->ioctl_lock);
            *((unsigned int*)arg) = MA_DRV_VERSION;
            mutex_unlock(&fp_data->ioctl_lock);
            break;

        case SCREEN_ON:
            microarray_switch_power(fp_data, FINGERPRINT_POWER_ON);
            break;

        case SCREEN_OFF:
            microarray_switch_power(fp_data, FINGERPRINT_POWER_OFF);
            break;

        case SET_SPI_SPEED:
            ret = copy_from_user(&microarray_spi_speed, (unsigned int*)arg, sizeof(unsigned int));
            microarray_spi_change(fp_data, microarray_spi_speed);
            break;

        case WAIT_FACTORY_CMD:
            fp_data->u2_flag = 0;
            ret = wait_event_freezable(g_u2_waitqueue, fp_data->u2_flag != 0);
            break;

        case WAKEUP_FINGERPRINTD:
            fp_data->u2_flag = 1;
            wake_up(&g_u2_waitqueue);
            break;

        case WAIT_FINGERPRINTD_RESPONSE:
            fp_data->u1_flag = 0;
            ret = wait_event_freezable(g_u1_waitqueue,  fp_data->u1_flag != 0);
            mutex_lock(&fp_data->ioctl_lock);
            ret = copy_to_user((unsigned int*)arg, &fp_data->drv_reg, sizeof(unsigned int));
            mutex_unlock(&fp_data->ioctl_lock);
            break;

        case WAKEUP_FACTORY_TEST_SEND_FINGERPRINTD_RESPONSE:
            mutex_lock(&fp_data->ioctl_lock);
            ret = copy_from_user(&fp_data->drv_reg, (unsigned int*)arg, sizeof(unsigned int));
            mutex_unlock(&fp_data->ioctl_lock);
            msleep(4);
            fp_data->u1_flag = 1;
            wake_up(&g_u1_waitqueue);
            break;

        case WAIT_SCREEN_STATUS_CHANGE:
            fp_data->screen_flag = 0;
            ret = wait_event_freezable(g_screen_waitqueue, fp_data->screen_flag != 0);
            mutex_lock(&fp_data->ioctl_lock);
            ret = copy_to_user((unsigned int*)arg, &fp_data->is_screen_on, sizeof(unsigned int));
            mutex_unlock(&fp_data->ioctl_lock);
            break;

        case GET_INTERRUPT_STATUS:
            int_pin_state = microarray_get_interrupt_status(fp_data);
            if (int_pin_state == 0 || int_pin_state == 1) {
                mutex_lock(&fp_data->ioctl_lock);
                ret = copy_to_user((unsigned int*)arg, &int_pin_state, sizeof(unsigned int));
                mutex_unlock(&fp_data->ioctl_lock);
            }
            break;

        case GET_SCREEN_STATUS:
            mutex_lock(&fp_data->ioctl_lock);
            ret = copy_to_user((unsigned int*)arg, &fp_data->is_screen_on, sizeof(unsigned int));
            mutex_unlock(&fp_data->ioctl_lock);
            break;

        default:
            ret = -EINVAL;
            dev_err(&fp_data->spi->dev, "mas_ioctl no such cmd");
    }

    return ret;
}

int microarray_fp_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct microarray_fp_data *fp_data = filp->private_data;
    unsigned long page;

    if ( !fp_data->kernel_memaddr ) {
        fp_data->kernel_memaddr = kmalloc(FINGERPRINT_KERNEL_MEM_SIZE, GFP_KERNEL);
        if( !fp_data->kernel_memaddr ) {
                return -1;
        }
    }

    page = virt_to_phys((void *)fp_data->kernel_memaddr) >> PAGE_SHIFT;
    vma->vm_page_prot=pgprot_noncached(vma->vm_page_prot);
    if ( remap_pfn_range(vma, vma->vm_start, page,
                        (vma->vm_end - vma->vm_start),
                        vma->vm_page_prot) ) {
        return -1;
    }

    /* do not know vm_flags VM_RESERVED is used for */
    //vma->vm_flags |= VM_RESERVED;
    dev_err(&fp_data->spi->dev,"remap_pfn_rang page:[%lu] ok.\n", page);

    return 0;
}

#ifdef CONFIG_COMPAT
static long microarray_fp_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return filp->f_op->unlocked_ioctl(filp, cmd, arg);
}
#endif

#ifdef COMPATIBLE_VERSION3
static unsigned int microarray_fp_poll(struct file *filp, struct poll_table_struct *wait)
{
    struct microarray_fp_data *fp_data = filp->private_data;
    struct spi_device *spi = fp_data->spi;
    unsigned int mask = 0;

    dev_err(&spi->dev, "%s: start. f_irq=%d f_repo=%d f_wake=%d\n",
                    __func__, fp_data->f_irq, fp_data->f_repo, fp_data->f_wake);

    poll_wait(filp, &g_drv_waitqueue, wait);

    if (fp_data->f_irq && fp_data->f_repo) {
        fp_data->f_repo = FALSE;
        mask |= POLLIN | POLLRDNORM;
    } else if ( fp_data->f_wake ) {
        fp_data->f_wake = FALSE;
        mask |= POLLPRI;
    }

    dev_err(&spi->dev, "%s: end. mask=%d\n", __func__, mask);

    return mask;
}
#endif

static int microarray_fp_open(struct inode *inode, struct file *filp)
{
    struct cdev *cdev = inode->i_cdev;
    struct finger_print_dev *fp_dev = container_of(cdev, struct finger_print_dev, chd);
    struct microarray_fp_data *fp_data =
            container_of(fp_dev, struct microarray_fp_data, fp_dev);

    filp->private_data = fp_data;
    microarray_fp_power_on(fp_data, FINGERPRINT_POWER_ON);

    microarray_fp_reset(fp_data);

    return 0;
}

static int microarray_fp_close(struct inode *inode, struct file *filp)
{
    struct microarray_fp_data *fp_data = (struct microarray_fp_data *)filp->private_data;;

    microarray_fp_power_on(fp_data, FINGERPRINT_POWER_OFF);
    filp->private_data = NULL;

    return 0;
}

static const struct file_operations microarray_fp_fops = {
    .owner                  = THIS_MODULE,
    .write                  = microarray_fp_write,
    .read                   = microarray_fp_read,
    .unlocked_ioctl         = microarray_fp_ioctl,
    .mmap                   = microarray_fp_mmap,
#ifdef CONFIG_COMPAT
    .compat_ioctl           = microarray_fp_compat_ioctl,
#endif
#ifdef COMPATIBLE_VERSION3
    .poll                   = microarray_fp_poll,
#endif
    .open                   = microarray_fp_open,
    .release                = microarray_fp_close,
};

static int microarray_fb_notifier_callback(struct notifier_block *self, unsigned long event, void *data)
{
    struct microarray_fp_data *fp_data;
    struct fb_event *evdata = data;
    unsigned int blank;

    fp_data = container_of(self, struct microarray_fp_data, fp_notifier);

    if (event != FB_EVENT_BLANK) {
        return 0;
    }

    blank = *(int *)evdata->data;
    switch (blank) {
        case FB_BLANK_UNBLANK:
            fp_data->is_screen_on = 1;
            break;

        case FB_BLANK_POWERDOWN:
            fp_data->is_screen_on = 0;
            break;

        default:
            break;
    }

    fp_data->screen_flag = 1;
    wake_up(&g_screen_waitqueue);

    return 0;
}

static void microarray_register_notifier(struct microarray_fp_data *fp_data)
{
    memset(&fp_data->fp_notifier, 0, sizeof(fp_data->fp_notifier));

    fp_data->fp_notifier.notifier_call = microarray_fb_notifier_callback;
    fb_register_client(&fp_data->fp_notifier);
    fp_data->is_screen_on = 1;
}

static int microarray_fp_gpio_init(struct microarray_fp_data *fp_data)
{
    int err = 0;
    struct microarray_platform_data *pdata = fp_data->pdata;
    struct spi_device *spi = fp_data->spi;

    if (!pdata) {
        return 0;
    }

    fp_data->power_2v8  = pdata->power_2v8;
    fp_data->power_1v8  = pdata->power_1v8;
    fp_data->power_en   = pdata->power_en;
    fp_data->gpio_int   = pdata->gpio_int;
    fp_data->reset      = pdata->reset;


    if (gpio_is_valid(fp_data->power_2v8)) {
        err = gpio_request(fp_data->power_2v8, "fp power 2v8");
        if (err < 0) {
            dev_err(&spi->dev,"finger print: %s power 2v8[%d] gpio_requests failed.\n",
                    __func__, fp_data->power_2v8);
            goto gpio_power_2v8_failed;
        }
    }

    if (gpio_is_valid(fp_data->power_1v8)) {
        err = gpio_request(fp_data->power_1v8, "fp power 1v8");
        if (err < 0) {
            dev_err(&spi->dev,"finger print: %s power 1v8[%d] gpio_requests failed.\n",
                    __func__, fp_data->power_1v8);
            goto gpio_power_1v8_failed;
        }
    }

    if (gpio_is_valid(fp_data->power_en)) {
        err = gpio_request(fp_data->power_en, "fp power en");
        if (err < 0) {
            dev_err(&spi->dev,"finger print: %s power en[%d] gpio_requests failed.\n",
                    __func__, fp_data->power_en);
            goto gpio_power_en_failed;
        }
        gpio_direction_output(fp_data->power_en, 0);
    }

    if (gpio_is_valid(fp_data->reset)) {
        err = gpio_request(fp_data->reset, "fp reset");
        if (err < 0) {
            dev_err(&spi->dev,"finger print: %s reset[%d] gpio_requests failed.\n",
                    __func__, fp_data->reset);
            goto gpio_reset_failed;
        }
    }

    if (gpio_is_valid(fp_data->gpio_int)) {
        err = gpio_request(fp_data->gpio_int, "fp interrupt");
        if (err < 0) {
            dev_err(&spi->dev,"finger print: %s interrupt[%d] gpio_requests failed.\n",
                    __func__, fp_data->gpio_int);
            goto gpio_interrupt_failed;
        }

        gpio_direction_input(fp_data->gpio_int);
    }

    return 0;

gpio_interrupt_failed:
    if (gpio_is_valid(fp_data->reset))
        gpio_free(fp_data->reset);
gpio_reset_failed:
    if (gpio_is_valid(fp_data->power_en))
        gpio_free(fp_data->power_en);
gpio_power_en_failed:
    if (gpio_is_valid(fp_data->power_1v8))
        gpio_free(fp_data->power_1v8);
gpio_power_1v8_failed:
    if (gpio_is_valid(fp_data->power_2v8))
        gpio_free(fp_data->power_2v8);
gpio_power_2v8_failed:
    return err;
}

static int microarray_fp_setup_input(struct microarray_fp_data *fp_data)
{
    struct input_dev *input = NULL;
    struct spi_device *spi = fp_data->spi;
    int err = 0;

    input = input_allocate_device();
    if (!input) {
        dev_err(&spi->dev, "input_allocate_device failed.\n");
        return -1;
    }

    set_bit(EV_KEY, input->evbit);
    set_bit(EV_ABS, input->evbit);
    set_bit(EV_SYN, input->evbit);

    set_bit(FINGERPRINT_SWIPE_UP,       input->keybit); //单触
    set_bit(FINGERPRINT_SWIPE_DOWN,     input->keybit);
    set_bit(FINGERPRINT_SWIPE_LEFT,     input->keybit);
    set_bit(FINGERPRINT_SWIPE_RIGHT,    input->keybit);
    set_bit(FINGERPRINT_TAP,            input->keybit);
    set_bit(FINGERPRINT_DTAP,           input->keybit);
    set_bit(FINGERPRINT_LONGPRESS,      input->keybit);
    set_bit(KEY_POWER,                  input->keybit);

    input->name         = "microarray_fp";
    input->id.bustype   = BUS_SPI;

    err = input_register_device(input);
    if (err) {
        input_free_device(input);
        dev_err(&spi->dev, "failed to register input device.\n");
        return -1;
    }

    fp_data->input = input;

    return 0;
}

/*
 * check whether the chip is microarray's
 * @return =-1 failed
 *         =0  sucess
 */
static int microarray_fp_check_id(struct microarray_fp_data *fp_data)
{
    struct spi_device *spi = fp_data->spi;
    unsigned char tx_buffer[4];
    unsigned char rx_buffer[4];
    int retry = 4;
    int err = 0;
    while (retry--) {
        /* soft reset  */
        tx_buffer[0] = 0x8c;
        tx_buffer[1] = 0xff;
        tx_buffer[2] = 0xff;
        tx_buffer[3] = 0xff;
        err = microarray_spi_transfer(fp_data, tx_buffer, rx_buffer, 4);
        msleep(8);

        /* read chip ID */
        tx_buffer[0] = 0x00;
        tx_buffer[1] = 0xff;
        tx_buffer[2] = 0xff;
        tx_buffer[3] = 0xff;
        err = microarray_spi_transfer(fp_data, tx_buffer, rx_buffer, 4);
        if (err != 0)
            dev_err(&spi->dev, "do init_connect failed, retry again [%d] times!", retry);

        if (rx_buffer[3] == 0x41) {
            return 0;
        } else {
            dev_err(&spi->dev, "the chip id is not match."
                    "rx_buffer[3]=0x%x rx_buffer[2]=0x%x\n", rx_buffer[3], rx_buffer[2]);
        }
    }

    return -1;
}

static int microarray_fp_suspend(struct spi_device *spi, pm_message_t mesg)
{
    struct microarray_fp_data *fp_data = dev_get_drvdata(&spi->dev);

    microarray_fp_power_on(fp_data, FINGERPRINT_POWER_OFF);

    return 0;
}

static int microarray_fp_resume(struct spi_device *spi)
{
    struct microarray_fp_data *fp_data = dev_get_drvdata(&spi->dev);

    microarray_fp_power_on(fp_data, FINGERPRINT_POWER_ON);

    return 0;
}

static int microarray_fp_probe(struct spi_device *spi)
{
    struct microarray_fp_data *fp_data = NULL;
    int err = -1;

    fp_data = kzalloc(sizeof(struct microarray_fp_data), GFP_KERNEL);
    if (fp_data == NULL) {
        dev_err(&spi->dev, "micro array finger print kmalloc failed.\n");
        err = -ENOMEM;
        goto exit_alloc_data_failed;
    }

    dev_set_drvdata(&spi->dev, fp_data);

    fp_data->kernel_memaddr = NULL;
    fp_data->kernel_memesize = 0;
    fp_data->pdata  = spi->dev.platform_data;
    fp_data->spi    = spi;
    mutex_init(&fp_data->dev_lock);
    mutex_init(&fp_data->ioctl_lock);

    err = microarray_fp_gpio_init(fp_data);
    if (err < 0) {
        dev_err(&spi->dev, "micro array gpio init failed.\n");
        goto exit_gpio_init_failed;
    }

    fp_data->fp_dev.minor = 0;
    err = alloc_chrdev_region(&fp_data->fp_dev.idd,
                               fp_data->fp_dev.minor,
                               1,
                               MICROARRAY_DRV_NAME);
    if (err < 0) {
        dev_err(&spi->dev, "alloc_chrdev_region failed!");
        goto exit_alloc_chr_dev_failed;
    }

    cdev_init(&fp_data->fp_dev.chd, &microarray_fp_fops);
    fp_data->fp_dev.chd.owner  = THIS_MODULE;
    fp_data->fp_dev.major = MAJOR(fp_data->fp_dev.idd);
    cdev_add(&fp_data->fp_dev.chd, fp_data->fp_dev.idd, 1);

    fp_data->fp_dev.cls = class_create(THIS_MODULE, MICROARRAY_DRV_NAME);
    if (IS_ERR(fp_data->fp_dev.cls)) {
        dev_err(&spi->dev, "class create failed!");
        goto exit_cls_create_failed;
    }

    fp_data->fp_dev.dev = device_create(fp_data->fp_dev.cls,  NULL,
            fp_data->fp_dev.idd, NULL, MICROARRAY_DRV_NAME);
    err = IS_ERR(fp_data->fp_dev.dev) ? PTR_ERR(fp_data->fp_dev.dev) : 0;
    if (err) {
        dev_err(&spi->dev,"device_create failed. err = %d", err);
        goto exit_device_create_failed;
    }

    fp_data->spi->max_speed_hz = SPI_SPEED;
    err = spi_setup(spi);

    err = microarray_fp_check_id(fp_data);
    if (err < 0) {
        dev_err(&spi->dev,"check id failed. err = %d", err);
        goto exit_check_id_failed;
    }

    err = microarray_fp_setup_input(fp_data);
    if (err < 0) {
        dev_err(&spi->dev, "microarray setup input failed.\n");
        goto exit_setup_input_failed;
    }


    wake_lock_init(&fp_data->process_wakelock, WAKE_LOCK_SUSPEND,"microarray_process_wakelock");
    INIT_WORK(&fp_data->work, microarray_fp_work);
    fp_data->workqueue = create_singlethread_workqueue("mas_workqueue");
    if (!fp_data->workqueue) {
        dev_err(&spi->dev, "create_single_workqueue error!\n");
        err = -1;
        goto exit_create_workqueu_failed;
    }

    fp_data->irq = gpio_to_irq(fp_data->gpio_int);
    err = request_irq(fp_data->irq, microarray_fp_interrupt,
                IRQF_TRIGGER_RISING, "microarray irq", fp_data);
    if (err < 0) {
        dev_err(&spi->dev,"request_irq failed err=%d.", err);
        goto exit_request_irq_failed;
    }

    microarray_register_notifier(fp_data);

    dev_err(&spi->dev, "microarray finger print register sucessfull.\n");
    return 0;

exit_request_irq_failed:
    destroy_workqueue(fp_data->workqueue);
exit_create_workqueu_failed:
    wake_lock_destroy(&fp_data->process_wakelock);
    input_unregister_device(fp_data->input);
    input_free_device(fp_data->input);
exit_setup_input_failed:
exit_check_id_failed:
    device_destroy(fp_data->fp_dev.cls, fp_data->fp_dev.idd);
exit_device_create_failed:
    class_destroy(fp_data->fp_dev.cls);
exit_cls_create_failed:
    cdev_del(&fp_data->fp_dev.chd);
    unregister_chrdev_region(fp_data->fp_dev.idd, 1);
exit_alloc_chr_dev_failed:
    if (gpio_is_valid(fp_data->gpio_int))
        gpio_free(fp_data->gpio_int);
    if (gpio_is_valid(fp_data->reset))
        gpio_free(fp_data->reset);
    if (gpio_is_valid(fp_data->power_en))
        gpio_free(fp_data->power_en);
    if (gpio_is_valid(fp_data->power_1v8))
        gpio_free(fp_data->power_1v8);
    if (gpio_is_valid(fp_data->power_2v8))
        gpio_free(fp_data->power_2v8);
exit_gpio_init_failed:
    kfree(fp_data);
    fp_data = NULL;
exit_alloc_data_failed:
    return err;
}


static int microarray_fp_remove(struct spi_device *spi)
{
    struct microarray_fp_data *fp_data = NULL;
    fp_data = dev_get_drvdata(&spi->dev);

    fb_unregister_client(&fp_data->fp_notifier);

    disable_irq(fp_data->irq);
    free_irq(fp_data->irq, fp_data);

    destroy_workqueue(fp_data->workqueue);
    wake_lock_destroy(&fp_data->process_wakelock);
    input_unregister_device(fp_data->input);
    input_free_device(fp_data->input);

    device_destroy(fp_data->fp_dev.cls, fp_data->fp_dev.idd);
    class_destroy(fp_data->fp_dev.cls);
    cdev_del(&fp_data->fp_dev.chd);
    unregister_chrdev_region(fp_data->fp_dev.idd, 1);

    if (gpio_is_valid(fp_data->gpio_int))
        gpio_free(fp_data->power_2v8);
    if (gpio_is_valid(fp_data->reset))
        gpio_free(fp_data->power_2v8);
    if (gpio_is_valid(fp_data->power_en))
        gpio_free(fp_data->power_en);
    if (gpio_is_valid(fp_data->power_1v8))
        gpio_free(fp_data->power_2v8);
    if (gpio_is_valid(fp_data->power_2v8))
        gpio_free(fp_data->power_2v8);

    kfree(fp_data);
    fp_data = NULL;

    return 0;
}


/**
 *  the spi struct date start,for getting the spi_device to set the spi clock enable start
 */

struct spi_device_id fp_dev_id = {MICROARRAY_DRV_NAME, 0};

struct spi_driver microarray_fp_drv = {
    .probe          = microarray_fp_probe,
    .remove         = microarray_fp_remove,
    .suspend        = microarray_fp_suspend,
    .resume         = microarray_fp_resume,
    .id_table       = &fp_dev_id,
    .driver = {
        .name       = MICROARRAY_DRV_NAME,
        .bus        = &spi_bus_type,
        .owner      = THIS_MODULE,
    },
};


static int __init microarray_init(void)
{
    int ret = 0;

    ret = spi_register_driver(&microarray_fp_drv);
    if(ret) {
        printk("spi_register_driver failed.\n");
    }

    return ret;
}

static void __exit microarray_exit(void)
{
    spi_unregister_driver(&microarray_fp_drv);
}

module_init(microarray_init);
module_exit(microarray_exit);

MODULE_AUTHOR("Ingenic");
MODULE_DESCRIPTION("Driver for microarray fingerprint sensor");
MODULE_LICENSE("GPL");
