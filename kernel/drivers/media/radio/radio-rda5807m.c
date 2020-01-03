#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <media/v4l2-device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <asm/unaligned.h>

extern void rda5807m_power_on(bool on);

#define DRIVER_VERSION        "0.0.2"
#define DRIVER_AUTHOR        "XinShuan <shuan.xin@ingenic.com>"
#define DRIVER_DESC              "RDA5807M radio driver"

#define PINFO(format, ...)\
    printk(KERN_INFO"RDA5807M : "DRIVER_VERSION ": " format "\n", ## __VA_ARGS__)
#define PWARN(format, ...)\
    printk(KERN_WARNING"RDA5807M : "DRIVER_VERSION ": " format "\n", ## __VA_ARGS__)

#define WRITE_REG_NUM        39
#define READ_REG_NUM          7

/* Frequency limits in MHz  */
#define FREQ_MIN  76
#define FREQ_MAX 108
/*
 * The frequency is set in units of 62.5 Hz when using V4L2_TUNER_CAP_LOW,
 * 62.5 kHz otherwise.
 * The tuner is able to have a channel spacing of 50, 100 or 200 kHz.
 * tuner->capability is therefore set to V4L2_TUNER_CAP_LOW
 * The FREQ_MUL is then: 1 MHz / 62.5 Hz = 16000
 */
#define FREQ_MUL 16000

#define RAD5807M_CHIPID                0x5808

#define RAD5807M_SCAN_TIME        0x0200

#define RAD5807M_VOLUME             0x000f
#define RAD5807M_MUTE                  0x4000
#define RAD5807M_MONOSELECT   0x2000
#define RAD5807M_POWER               0x0001
#define RAD5807M_SPACE                 0x0003
#define RAD5807M_BAND                 0x000c
#define RAD5807M_CHANNEL_CHAN    0xffc0
#define RAD5807M_CHANNEL_TUNE     0x0010
#define RAD5807M_STATUSRSSI_STC      0x4000
#define RAD5807M_READCHAN              0x03ff
#define RAD5807M_SEEK                          0x0100
#define RAD5807M_SEEKUP                     0x0200
#define RAD5807M_SKSEEK                      0x0080
#define RAD5807M_STATUSRSSI_SF        0x2000
#define RAD5807M_RBDS                        0x0008
#define RAD5807M_RBDS_OR_RDS        0x2000
#define RAD5807M_RBDS_READY           0x8000
#define RAD5807M_E_FOUND                0x0800
#define RAD5807M_ABCD_E                   0x0010
#define RAD5807M_BLERA                      0x000c
#define RAD5807M_BLERB                      0x0003
#define RAD5807M_BLERC                      0xc000
#define RAD5807M_BLERD                      0x3000
#define RAD5807M_AFCD                       0x0100
#define RAD5807M_RSSI                         0xfe00
#define RAD5807M_FM_READY              0x0080
#define RAD5807M_ST                             0x0400

/* Radio Nr */
static int radio_nr = -1;
module_param(radio_nr, int, 0444);
MODULE_PARM_DESC(radio_nr, "Radio Nr");

#ifdef CONFIG_RDA5807M_RDS
/* RDS buffer blocks */
static unsigned int rds_buf = 100;
module_param(rds_buf, uint, 0444);
MODULE_PARM_DESC(rds_buf, "RDS buffer entries: *100*");

/* RDS maximum block errors */
static unsigned short max_rds_errors = 1;
/* 0 means   0  errors requiring correction */
/* 1 means 1-2  errors requiring correction (used by original USBRadio.exe) */
/* 2 means 3-5  errors requiring correction */
/* 3 means   6+ errors or errors in checkword, correction not possible */
module_param(max_rds_errors, ushort, 0644);
MODULE_PARM_DESC(max_rds_errors, "RDS maximum block errors: *1*");

#endif

/* Tune timeout 3000*/
static unsigned int tune_timeout = 5000;
module_param(tune_timeout, uint, 0644);
MODULE_PARM_DESC(tune_timeout, "Tune timeout: *5000*");

/* Seek timeout 5000*/
static unsigned int seek_timeout = 10000;
module_param(seek_timeout, uint, 0644);
MODULE_PARM_DESC(seek_timeout, "Seek timeout: *10000*");

static const unsigned short wrtie_reg_init[WRITE_REG_NUM] = { 0xc00d, 0x0000,
        0x0400, 0xc6ef, 0x6000, 0x721a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0019, 0x2a11, 0xb042, 0x2a11, 0xb831,
        0xc000, 0x2a91, 0x9400, 0x00a8, 0xc400, 0xf7cf, 0x2414, 0x806f, 0x4608,
        0x0086, 0x0661, 0x0000, 0x109e, 0x23c8, 0x0406, 0x0e1c };

struct rda5807m_device {
    struct i2c_client *i2c_client;
    struct video_device *videodev;
    unsigned short write_reg[WRITE_REG_NUM];
    unsigned short read_reg[READ_REG_NUM];
    struct completion completion;
    struct mutex mutex;
    struct mutex iic_mutex;
#ifdef CONFIG_RDA5807M_RDS
    wait_queue_head_t wait_queue;
    struct mutex buffer_mutex;
    unsigned char *buffer; /* size is always multiple of three */
    unsigned int buf_size;
    unsigned int rd_index;
    unsigned int wr_index;
    bool rds_scan;
#endif
    bool stc_scan;
    bool power;
    struct delayed_work rbds_work;
};

static const struct v4l2_frequency_band bands[] = {
    {
        .type = V4L2_TUNER_RADIO,
        .index = 0,
        .capability = V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_STEREO | V4L2_TUNER_CAP_RDS | V4L2_TUNER_CAP_RDS_BLOCK_IO | V4L2_TUNER_CAP_FREQ_BANDS | V4L2_TUNER_CAP_HWSEEK_BOUNDED | V4L2_TUNER_CAP_HWSEEK_WRAP,
        .rangelow = 87000 * 16,
        .rangehigh = 108000 * 16,
        .modulation = V4L2_BAND_MODULATION_FM,
    },
    {
        .type = V4L2_TUNER_RADIO,
        .index = 1,
        .capability = V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_STEREO | V4L2_TUNER_CAP_RDS | V4L2_TUNER_CAP_RDS_BLOCK_IO | V4L2_TUNER_CAP_FREQ_BANDS | V4L2_TUNER_CAP_HWSEEK_BOUNDED | V4L2_TUNER_CAP_HWSEEK_WRAP,
        .rangelow = 76000 * 16,
        .rangehigh = 91000 * 16,
        .modulation = V4L2_BAND_MODULATION_FM,
    },
    {
        .type = V4L2_TUNER_RADIO,
        .index = 2,
        .capability = V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_STEREO | V4L2_TUNER_CAP_RDS | V4L2_TUNER_CAP_RDS_BLOCK_IO | V4L2_TUNER_CAP_FREQ_BANDS | V4L2_TUNER_CAP_HWSEEK_BOUNDED | V4L2_TUNER_CAP_HWSEEK_WRAP,
        .rangelow = 76000 * 16,
        .rangehigh = 108000 * 16,
        .modulation = V4L2_BAND_MODULATION_FM,
    },
};

/*
 * rda5807m_get_register - read register
 * num :  0 < num < READ_REG_NUM
 */
int rda5807m_get_register(struct rda5807m_device *radio, unsigned char num) {
    int i;
    u16 buf[READ_REG_NUM];
    struct i2c_msg msgs[1] = {
         {
            .addr = radio->i2c_client->addr,
            .flags = I2C_M_RD,
            .len = sizeof(u16) * num,
            .buf = (void *) buf
        },
    };
    mutex_lock(&radio->iic_mutex);
    if (i2c_transfer(radio->i2c_client->adapter, msgs, 1) != 1) {
        mutex_unlock(&radio->iic_mutex);
        return -EIO;
    }
    mutex_unlock(&radio->iic_mutex);
    for (i = 0; i < num; i++)
        radio->read_reg[i] = (buf[i] >> 8) | (buf[i] << 8);
    return 0;
}

/*
 * rda5807m_set_register - write register
 * num :  0 < num < WRITE_REG_NUM
 */
int rda5807m_set_register(struct rda5807m_device *radio, unsigned char num) {
    int i;
    u16 buf[WRITE_REG_NUM];
    struct i2c_msg msgs[1] = {
        {
            .addr = radio->i2c_client->addr,
            .len = sizeof(u16) * num,
            .buf = (void *) buf
        },
    };
    for (i = 0; i < num; i++)
        buf[i] = (radio->write_reg[i] >> 8) | (radio->write_reg[i] << 8);

    mutex_lock(&radio->iic_mutex);
    if (i2c_transfer(radio->i2c_client->adapter, msgs, 1) != 1) {
        mutex_unlock(&radio->iic_mutex);
        return -EIO;
    }
    mutex_unlock(&radio->iic_mutex);
    return 0;
}

/* V4L2 code related */
static const struct v4l2_queryctrl radio_qctrl[] = {
    {
        .id = V4L2_CID_AUDIO_MUTE,
        .name = "Mute",
        .minimum = 0,
        .maximum = 1,
        .default_value = 1,
        .type = V4L2_CTRL_TYPE_BOOLEAN,
    },
    {
        .id =V4L2_CID_AUDIO_VOLUME,
        .name = "Volume",
        .minimum = 0,
        .maximum = 15,
        .default_value = 15,
        .type = V4L2_CTRL_TYPE_INTEGER,
    }
};

static int rda5807m_power_down(struct rda5807m_device *radio) {
    int ret = 0;
    radio->write_reg[0] &= ~0xc001;
    ret = rda5807m_set_register(radio, 1);
    if(ret)
    PWARN("rda5807m_power_down error !!!");
    return ret;
}

static int rda5807m_power_up(struct rda5807m_device *radio) {
    int ret;
    radio->write_reg[0] = 0x0002;
    ret = rda5807m_set_register(radio, 1);
    if (ret)
        goto reset_error;
    msleep(50);
    radio->write_reg[0] = 0xc001;
    ret = rda5807m_set_register(radio, 1);
    if (ret)
        goto reset_error;
    msleep(600);
    return 0;

reset_error:
    PWARN("rda5807m_init error %d\n", ret);
    return ret;
}

static int rda5807m_check_id(struct rda5807m_device *radio) {
    int ret;
    radio->write_reg[0] = 0x0002;
    ret = rda5807m_set_register(radio, 1);
    if (ret)
        return ret;
    msleep(50);
    ret = rda5807m_get_register(radio, READ_REG_NUM);
    if (ret) {
        PWARN("rda5807m_check_id error %d\n", ret);
        return ret;
    }
    PINFO("read ID : %d",radio->read_reg[4]);
    if (radio->read_reg[4] != RAD5807M_CHIPID) {
        PWARN("rda5807m ID : %d ,read id not same\n", RAD5807M_CHIPID);
        return ENODEV;
    }
    return 0;
}

static int rda5807m_set_band(struct rda5807m_device *radio, u8 band) {
    int ret;
    u16 tmp = radio->write_reg[1];
    if (((radio->write_reg[1] & RAD5807M_BAND) >> 2) == band)
        return 0;

    radio->write_reg[1] &= ~RAD5807M_BAND;
    radio->write_reg[1] |= band << 2;
    ret = rda5807m_set_register(radio, 2);
    if (ret) {
        PWARN("rda5807m_set_band error !!!");
        radio->write_reg[1] = tmp;
    }
    return ret;
}

static unsigned int rda5807m_get_step(struct rda5807m_device *radio) {
    /* Spacing (kHz) */
    switch (radio->write_reg[1] & RAD5807M_SPACE) {
        case 0:
            return 100 * 16;
        case 1:
            return 200 * 16;
        case 2:
            return 50 * 16;
        case 3:
            return 25 * 16;
    };
    return 0;
}

static int rda5807m_set_freq(struct rda5807m_device *radio, unsigned int freq) {
    int ret;
    unsigned short chan;
    int band = (radio->write_reg[1] & RAD5807M_BAND) >> 2;
    /* Chan = [ Freq (Mhz) - Bottom of Band (MHz) ] / Spacing (kHz) */
    chan = (freq - bands[band].rangelow) / rda5807m_get_step(radio);

    radio->write_reg[1] &= ~RAD5807M_CHANNEL_CHAN;
    radio->write_reg[1] |= RAD5807M_CHANNEL_TUNE | (chan << 6);
    ret = rda5807m_set_register(radio, 2);
    if (ret) {
        PWARN("rda5807m_set_freq error !!!");
        return ret;
    }

#if 0
    radio->stc_scan = 1;
    schedule_delayed_work(&radio->rbds_work, RAD5807M_SCAN_TIME);
    INIT_COMPLETION(radio->completion);
    ret = wait_for_completion_timeout(&radio->completion, msecs_to_jiffies(tune_timeout));
    if (!ret) {
        PWARN("rda5807m  tune freq timeout !!!");
        ret = -EAGAIN;
    }
    radio->stc_scan = 0;
    radio->write_reg[1] &= ~RAD5807M_CHANNEL_TUNE;
    rda5807m_set_register(radio, 2);
#endif

    return ret;
}

static int rda5807m_get_freq(struct rda5807m_device *radio, unsigned int *freq) {
    int chan, ret;
    int band = (radio->write_reg[1] & RAD5807M_BAND) >> 2;
    /* read channel */
    ret = rda5807m_get_register(radio, 1);
    if(ret)
    PWARN("rda5807m_get_freq error !!!");
    chan = radio->read_reg[0] & RAD5807M_READCHAN;
    /* Frequency (MHz) = Spacing (kHz) x Channel + Bottom of Band (MHz) */
    *freq = chan * rda5807m_get_step(radio) + bands[band].rangelow;
    return ret;
}

static int rda5807m_set_seek(struct rda5807m_device *radio, const struct v4l2_hw_freq_seek *seek) {
    int band, retval;
    unsigned int freq;
    /* set band */
    if (seek->rangelow || seek->rangehigh) {
        for (band = 0; band < ARRAY_SIZE(bands); band++) {
        if (bands[band].rangelow == seek->rangelow && bands[band].rangehigh == seek->rangehigh)
            break;
        }
        if (band == ARRAY_SIZE(bands))
            return -EINVAL; /* No matching band found */
    }
    else
        band = 0; /* If nothing is specified seek 87 - 108 Mhz */

    if ((radio->write_reg[1] & RAD5807M_BAND) >> 2 != band) {
        retval = rda5807m_get_freq(radio, &freq);
        if (retval)
            return retval;
        retval = rda5807m_set_band(radio, band);
        if (retval)
            return retval;
        retval = rda5807m_set_freq(radio, freq);
        if (retval)
        return retval;
    }

    /* start seeking */
    radio->write_reg[0] |= RAD5807M_SEEK;

    if (seek->wrap_around)
        radio->write_reg[0] &= ~RAD5807M_SKSEEK;
    else
        radio->write_reg[0] |= RAD5807M_SKSEEK;

    if (seek->seek_upward)
        radio->write_reg[0] |= RAD5807M_SEEKUP;
    else
        radio->write_reg[0] &= ~RAD5807M_SEEKUP;

    retval = rda5807m_set_register(radio, 1);
    if (retval)
        return retval;

    radio->stc_scan = 1;
    schedule_delayed_work(&radio->rbds_work, RAD5807M_SCAN_TIME);
    INIT_COMPLETION(radio->completion);
    retval = wait_for_completion_timeout(&radio->completion, msecs_to_jiffies(seek_timeout));
    if (!retval) {
        PWARN("rda5807m seek timeout !!!");
        retval = -EAGAIN;
    }
    if (radio->read_reg[0] & RAD5807M_STATUSRSSI_SF) {
        PWARN("rda5807m seek failure !!!");
        retval = -EAGAIN;
    }
    radio->stc_scan = 0;
    radio->write_reg[0] &= ~RAD5807M_SEEK;
    rda5807m_set_register(radio, 1);
    return retval;
}

static int rda5807m_mute(struct rda5807m_device *radio, int on) {
    int ret;
    u16 tmp = radio->write_reg[0];
    if (on)
        radio->write_reg[0] &= ~RAD5807M_MUTE;
    else
        radio->write_reg[0] |= RAD5807M_MUTE;

    ret = rda5807m_set_register(radio, 1);
    if (ret) {
        radio->write_reg[0] = tmp;
    PWARN("rda5807m_mute %d error !!!", ret);
    }
    return ret;
}

static bool rda5807m_is_muted(struct rda5807m_device *radio) {
    return !(radio->write_reg[0] & RAD5807M_MUTE);
}

static int rda5807m_get_volume(struct rda5807m_device *radio) {
    return radio->write_reg[3] & RAD5807M_VOLUME;
}

static int rda5807m_set_volume(struct rda5807m_device *radio, int data) {
    int ret;
    u16 tmp = radio->write_reg[3];
    if ((data < 0) || (data > 15))
        return -EINVAL;

    radio->write_reg[3] &= ~RAD5807M_VOLUME;
    radio->write_reg[3] |= data;

    ret = rda5807m_set_register(radio, 4);
    if (ret) {
        radio->write_reg[3] = tmp;
        PWARN("rda5807m_set_volume %d error !!!", ret);
    }
    return ret;
}
#ifdef CONFIG_RDA5807M_RDS
static int rda5807m_rds_on(struct rda5807m_device *radio) {
    int retval;
    radio->write_reg[0] |= RAD5807M_RBDS;
    retval = rda5807m_set_register(radio, 1);
    if (retval) {
    radio->write_reg[0] &= ~RAD5807M_RBDS;
    PWARN("rda5807m_rds_on error !!!");
    }
    return retval;
}
#endif
int rda5807m_start(struct rda5807m_device *radio) {
    int retval, i;
    rda5807m_power_on(true);
    retval = rda5807m_power_up(radio);
    if (retval)
        goto start_error;

    for (i = 0; i < WRITE_REG_NUM; i++)
        radio->write_reg[i] = wrtie_reg_init[i];

    retval = rda5807m_set_register(radio, WRITE_REG_NUM);
    if (retval)
        goto start_error;

    msleep(50);
#ifdef CONFIG_RDA5807M_RDS
    radio->rds_scan = 1;
    schedule_delayed_work(&radio->rbds_work, RAD5807M_SCAN_TIME);
#endif
    return 0;

start_error:
    PWARN("rda5807m_start error !!!");
    return retval;
}

int rda5807m_stop(struct rda5807m_device *radio) {
    rda5807m_power_down(radio);
    rda5807m_power_on(false);
#ifdef CONFIG_RDA5807M_RDS
    radio->rds_scan = 0;
#endif
    radio->stc_scan = 0;
    cancel_delayed_work_sync(&radio->rbds_work);

    return 0;
}
#ifdef CONFIG_RDA5807M_RDS
static ssize_t rda5807m_fops_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct rda5807m_device *radio = video_drvdata(file);
    int retval = 0;
    unsigned int block_count = 0;

    /* switch on rds reception */
    if (!(radio->write_reg[0] & RAD5807M_RBDS))
    rda5807m_rds_on(radio);

    /* block if no new data available */
    if (radio->wr_index == radio->rd_index) {
        if (file->f_flags & O_NONBLOCK) {
        retval = -EWOULDBLOCK;
        goto done;
        }
        if (wait_event_interruptible(radio->wait_queue, radio->wr_index != radio->rd_index) < 0) {
            retval = -EINTR;
            goto done;
        }
    }
    /* calculate block count from byte count */
    count /= 3;
    mutex_lock(&radio->buffer_mutex);

    /* copy RDS block out of internal buffer and to user buffer */
    while (block_count < count) {
        if (radio->rd_index == radio->wr_index)
            break;

        /* always transfer rds complete blocks */
        if (copy_to_user(buf, &radio->buffer[radio->rd_index], 3))
            break;

        /* increment and wrap read pointer */
        radio->rd_index += 3;
        if (radio->rd_index >= radio->buf_size)
            radio->rd_index = 0;

        /* increment counters */
        block_count++;
        buf += 3;
        retval += 3;
    }
    mutex_unlock(&radio->buffer_mutex);

done:
    return retval;
}

static unsigned int rda5807m_fops_poll(struct file *file, struct poll_table_struct *pts) {
    struct rda5807m_device *radio = video_drvdata(file);
    unsigned long req_events = poll_requested_events(pts);
    int retval = v4l2_ctrl_poll(file, pts);

    if (req_events & (POLLIN | POLLRDNORM)) {
        /* switch on rds reception */
        if (!(radio->read_reg[0] & RAD5807M_RBDS_READY))
            rda5807m_rds_on(radio);

        poll_wait(file, &radio->wait_queue, pts);

        if (radio->rd_index != radio->wr_index)
            retval |= POLLIN | POLLRDNORM;
    }
    return retval;
}
#endif
int rda5807m_fops_open(struct file *file) {
    struct rda5807m_device *radio = video_drvdata(file);
    int retval = v4l2_fh_open(file);
    if (retval)
        return retval;

    if (v4l2_fh_is_singular_file(file)) {
        /* start radio */
        if (radio->power) {
            retval = -EBUSY;
            goto done;
        }
        retval = rda5807m_start(radio);
        if (retval < 0)
        goto done;
        radio->power = true;
    }

done:
    if (retval)
    v4l2_fh_release(file);
    return retval;
}

int rda5807m_fops_release(struct file *file) {
    struct rda5807m_device *radio = video_drvdata(file);
    if (v4l2_fh_is_singular_file(file)) {
        /* stop radio */
        radio->power = false;
        rda5807m_stop(radio);
    }
    return v4l2_fh_release(file);
}

/* V4L2 vidioc */
static int rda5807m_vidioc_querycap(struct file *file, void *priv, struct v4l2_capability *capability) {
    struct rda5807m_device *radio = video_drvdata(file);
    struct video_device *dev = radio->videodev;

    strlcpy(capability->driver, dev->name, sizeof(capability->driver));
    strlcpy(capability->card, dev->name, sizeof(capability->card));
    snprintf(capability->bus_info, sizeof(capability->bus_info), "I2C:%s", dev_name(&dev->dev));

    capability->device_caps = V4L2_CAP_HW_FREQ_SEEK | V4L2_CAP_READWRITE | V4L2_CAP_TUNER | V4L2_CAP_RADIO | V4L2_CAP_RDS_CAPTURE;
    capability->capabilities = capability->device_caps | V4L2_CAP_DEVICE_CAPS;
    return 0;
}

static int rda5807m_vidioc_g_tuner(struct file *file, void *priv, struct v4l2_tuner *tuner) {
    struct rda5807m_device *radio = video_drvdata(file);
    int retval = 0;
    if (tuner->index != 0)
        return -EINVAL;

    retval = rda5807m_get_register(radio, 2);
    if (retval)
        return retval;

    memset(tuner, 0, sizeof(*tuner));
    strcpy(tuner->name, "FM");
    tuner->type = V4L2_TUNER_RADIO;
    tuner->rangelow = FREQ_MIN * FREQ_MUL;
    tuner->rangehigh = FREQ_MAX * FREQ_MUL;
    tuner->capability = V4L2_TUNER_CAP_LOW | V4L2_TUNER_CAP_STEREO | V4L2_TUNER_CAP_RDS | V4L2_TUNER_CAP_RDS_BLOCK_IO | V4L2_TUNER_CAP_HWSEEK_BOUNDED | V4L2_TUNER_CAP_HWSEEK_WRAP;
    if (radio->read_reg[0] & RAD5807M_ST) {
        tuner->rxsubchans = V4L2_TUNER_SUB_STEREO;
        tuner->audmode = V4L2_TUNER_MODE_STEREO;
    }
    else {
        tuner->rxsubchans = V4L2_TUNER_SUB_MONO;
        tuner->audmode = V4L2_TUNER_MODE_MONO;
    }
    tuner->signal = ((radio->read_reg[1] & RAD5807M_RSSI) >> 9) * 258;
    return 0;
}

static int rda5807m_vidioc_s_tuner(struct file *file, void *priv, const struct v4l2_tuner *tuner) {
    struct rda5807m_device *radio = video_drvdata(file);
    int ret;
    if (tuner->index != 0)
        return -EINVAL;

    if (tuner->audmode == V4L2_TUNER_MODE_MONO)
        radio->write_reg[0] |= RAD5807M_MONOSELECT;
    else
        radio->write_reg[0] &= ~RAD5807M_MONOSELECT;

    ret = rda5807m_set_register(radio, 1);
    if(ret)
        PWARN("rda5807m_vidioc_s_tuner error !!!");
    return ret;
}

static int rda5807m_vidioc_s_frequency(struct file *file, void *priv, const struct v4l2_frequency *freq) {
    struct rda5807m_device *radio = video_drvdata(file);
    u16 band = (radio->write_reg[1] & RAD5807M_BAND) >> 2;
    if (freq->tuner != 0 || freq->type != V4L2_TUNER_RADIO)
        return -EINVAL;

    if (freq->frequency < bands[band].rangelow || freq->frequency > bands[band].rangehigh)
        return -EINVAL;

    return rda5807m_set_freq(radio, freq->frequency);
}

static int rda5807m_vidioc_g_frequency(struct file *file, void *priv, struct v4l2_frequency *freq) {
    struct rda5807m_device *radio = video_drvdata(file);
    if (freq->tuner != 0)
        return -EINVAL;
    freq->type = V4L2_TUNER_RADIO;
    return rda5807m_get_freq(radio, &freq->frequency);
}

static int rda5807m_vidioc_s_hw_freq_seek(struct file *file, void *priv, const struct v4l2_hw_freq_seek *seek) {
    struct rda5807m_device *radio = video_drvdata(file);
    if (seek->tuner != 0)
        return -EINVAL;

    if (file->f_flags & O_NONBLOCK)
        return -EWOULDBLOCK;

    return rda5807m_set_seek(radio, seek);
}

static int rda5807m_vidioc_enum_freq_bands(struct file *file, void *priv, struct v4l2_frequency_band *band) {
    if (band->tuner != 0)
        return -EINVAL;
    if (band->index >= ARRAY_SIZE(bands))
        return -EINVAL;
    *band = bands[band->index];
    return 0;
}

static int rda5807m_vidioc_queryctrl(struct file *file, void *priv, struct v4l2_queryctrl *queryctrl) {
    int i;
    for (i = 0; i < ARRAY_SIZE(radio_qctrl); i++) {
        if (queryctrl->id && queryctrl->id == radio_qctrl[i].id) {
            memcpy(queryctrl, &(radio_qctrl[i]), sizeof(*queryctrl));
            return 0;
        }
    }
    return -EINVAL;
}

static int rda5807m_vidioc_g_ctrl(struct file *file, void *priv, struct v4l2_control *ctrl) {
    struct rda5807m_device *radio = video_drvdata(file);
    switch (ctrl->id) {
        case V4L2_CID_AUDIO_MUTE:
            ctrl->value = rda5807m_is_muted(radio);
            return 0;
        case V4L2_CID_AUDIO_VOLUME:
            ctrl->value = rda5807m_get_volume(radio);
            return 0;
    }
    return -EINVAL;
}

static int rda5807m_vidioc_s_ctrl(struct file *file, void *priv, struct v4l2_control *ctrl) {
    struct rda5807m_device *radio = video_drvdata(file);
    switch (ctrl->id) {
        case V4L2_CID_AUDIO_MUTE:
            return rda5807m_mute(radio, ctrl->value);
        case V4L2_CID_AUDIO_VOLUME:
            return rda5807m_set_volume(radio, ctrl->value);
    }
    return -EINVAL;
}

static inline struct rda5807m_device *to_rda5807m_device(struct delayed_work *work) {
    return container_of(work, struct rda5807m_device, rbds_work);
}

static void rbds_work_function(struct work_struct *work) {
    int retval = 0;
#ifdef CONFIG_RDA5807M_RDS
    int tmp = 0;
    unsigned char blocknum;
    unsigned short bler; /* rds block errors */
    unsigned short rds;
    unsigned char tmpbuf[3];
#endif
    struct delayed_work *dw = to_delayed_work(work);
    struct rda5807m_device *radio = to_rda5807m_device(dw);

    retval = rda5807m_get_register(radio, 2);
    if (retval)
        goto done;

    if ((radio->read_reg[0] & RAD5807M_STATUSRSSI_STC) && (radio->read_reg[1] & RAD5807M_FM_READY))
        complete(&radio->completion);
#ifdef CONFIG_RDA5807M_RDS
    if (radio->read_reg[0] & RAD5807M_RBDS_READY) {
        tmp = radio->wr_index + 3;
        if ((tmp < radio->buf_size) && (tmp != radio->rd_index)) {}
        else if ((tmp >= radio->buf_size) && (radio->rd_index != 0))
            tmp = 0;
        else
            goto done;

        retval = rda5807m_get_register(radio, READ_REG_NUM);
        if (retval)
            goto done;

        for (blocknum = 0; blocknum < 4; blocknum++) {
            switch (blocknum) {
                case 0:
                    bler = (radio->read_reg[1] & RAD5807M_BLERA) >> 2;
                    rds = radio->read_reg[2];
                    break;
                case 1:
                    bler = (radio->read_reg[1] & RAD5807M_BLERB);
                    rds = radio->read_reg[3];
                    break;
                case 2:
                    bler = (radio->read_reg[6] & RAD5807M_BLERC) >> 14;
                    rds = radio->read_reg[4];
                    break;
                case 3:
                    bler = (radio->read_reg[6] & RAD5807M_BLERD) >> 12;
                    rds = radio->read_reg[5];
                    break;
            }

            /* Fill the V4L2 RDS buffer */
            put_unaligned_le16(rds, &tmpbuf);
            tmpbuf[2] = blocknum; /* offset name */
            tmpbuf[2] |= blocknum << 3; /* received offset */
            if (bler > max_rds_errors)
                tmpbuf[2] |= 0x80; /* uncorrectable errors */
            else if (bler > 0)
                tmpbuf[2] |= 0x40; /* corrected error(s) */

            mutex_lock(&radio->buffer_mutex);
            /* copy RDS block to internal buffer */
            memcpy(&radio->buffer[radio->wr_index], &tmpbuf, 3);
            radio->wr_index = tmp;
            mutex_unlock(&radio->buffer_mutex);
            wake_up_interruptible(&radio->wait_queue);
        }
    }
#endif
done:
    if (radio->stc_scan)
        schedule_delayed_work(dw, msecs_to_jiffies(RAD5807M_SCAN_TIME));
#ifdef CONFIG_RDA5807M_RDS
    else if(radio->rds_scan)
        schedule_delayed_work(dw, msecs_to_jiffies(RAD5807M_SCAN_TIME));
#endif
}

/* File system interface */
static const struct v4l2_file_operations rda5807m_fops = {
    .owner = THIS_MODULE,
#ifdef CONFIG_RDA5807M_RDS
    .read = rda5807m_fops_read,
    .poll = rda5807m_fops_poll,
#endif
    .unlocked_ioctl = video_ioctl2,
    .open = rda5807m_fops_open,
    .release = rda5807m_fops_release,
};

static const struct v4l2_ioctl_ops rda5807m_ioctl_ops = {
    .vidioc_querycap = rda5807m_vidioc_querycap,
    .vidioc_g_tuner = rda5807m_vidioc_g_tuner,
    .vidioc_s_tuner = rda5807m_vidioc_s_tuner,
    .vidioc_g_frequency = rda5807m_vidioc_g_frequency,
    .vidioc_s_frequency = rda5807m_vidioc_s_frequency,
    .vidioc_s_hw_freq_seek = rda5807m_vidioc_s_hw_freq_seek,
    .vidioc_enum_freq_bands = rda5807m_vidioc_enum_freq_bands,
    .vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
    .vidioc_unsubscribe_event = v4l2_event_unsubscribe,
    .vidioc_queryctrl = rda5807m_vidioc_queryctrl,
    .vidioc_g_ctrl =rda5807m_vidioc_g_ctrl,
    .vidioc_s_ctrl = rda5807m_vidioc_s_ctrl,
};

/* V4L2 interface */
static struct video_device rda5807m_radio_template = {
    .name = "RDA5807M FM-Radio",
    .fops = &rda5807m_fops,
    .ioctl_ops = &rda5807m_ioctl_ops,
    .release = video_device_release,
};

/* I2C probe: check if the device exists and register with v4l if it is */
static int rda5807m_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct rda5807m_device *radio;
    int ret;
    radio = kzalloc(sizeof(struct rda5807m_device), GFP_KERNEL);
    if (radio == NULL)
        return -ENOMEM;

    radio->i2c_client = client;
    mutex_init(&radio->mutex);
    mutex_init(&radio->iic_mutex);
    init_completion(&radio->completion);
    INIT_DELAYED_WORK(&radio->rbds_work, rbds_work_function);
#ifdef CONFIG_RDA5807M_RDS
    init_waitqueue_head(&radio->wait_queue);
    mutex_init(&radio->buffer_mutex);
    /* rds buffer allocation */
    radio->buf_size = rds_buf * 3;
    radio->buffer = kzalloc(radio->buf_size, GFP_KERNEL);
    if (radio->buffer == NULL) {
        ret = -ENOMEM;
        goto errfree;
    }
#endif
    radio->videodev = video_device_alloc();
    if (radio->videodev == NULL) {
       ret = -ENOMEM;
        goto errfr;
    }
    memcpy(radio->videodev, &rda5807m_radio_template, sizeof(rda5807m_radio_template));
    video_set_drvdata(radio->videodev, radio);
    radio->videodev->lock = &radio->mutex;

    rda5807m_power_on(true);
    ret = rda5807m_check_id(radio);
    PINFO("read_reg : %d``%d``%d``%d``%d``%d` `%d",
        radio->read_reg[0],radio->read_reg[1],radio->read_reg[2],radio->read_reg[3],radio->read_reg[4],radio->read_reg[5],radio->read_reg[6]);
    if (ret)
        goto errrel;
    rda5807m_power_on(false);

    ret = video_register_device(radio->videodev, VFL_TYPE_RADIO, radio_nr);
    if (ret) {
        PWARN("Could not register video device!");
        goto errrel;
    }

    i2c_set_clientdata(client, radio);
    PINFO("rda5807m_i2c_probe success");
    return 0;
errrel:
    video_device_release(radio->videodev);
errfr:
#ifdef CONFIG_RDA5807M_RDS
    kfree(radio->buffer);
errfree:
#endif
    kfree(radio);
    rda5807m_power_on(false);
    return ret;
}

static int rda5807m_i2c_remove(struct i2c_client *client) {
    struct rda5807m_device *radio = i2c_get_clientdata(client);
    if (radio) {
        video_unregister_device(radio->videodev);
#ifdef CONFIG_RDA5807M_RDS
        kfree(radio->buffer);
#endif
        kfree(radio);
    }
    return 0;
}

/* I2C subsystem interface */
static const struct i2c_device_id rda5807m_id[] = {
    { "radio-rda5807m", 0 }, { }
};
MODULE_DEVICE_TABLE(i2c, rda5807m_id);

static int rda5807m_i2c_suspend(struct i2c_client *client, pm_message_t mesg) {
    struct rda5807m_device *radio = i2c_get_clientdata(client);
    if (radio->power)
        rda5807m_stop(radio);
    return 0;
}

static int rda5807m_i2c_resume(struct i2c_client *client) {
    struct rda5807m_device *radio = i2c_get_clientdata(client);
    if (radio->power)
        rda5807m_start(radio);
    return 0;
}

static struct i2c_driver rda5807m_i2c_driver = {
    .driver = {
        .name = "radio-rda5807m",
        .owner = THIS_MODULE,
    },
    .probe = rda5807m_i2c_probe,
    .remove = rda5807m_i2c_remove,
    .suspend = rda5807m_i2c_suspend,
    .resume = rda5807m_i2c_resume,
    .id_table = rda5807m_id,
};

static int __init rda5807m_init(void)
{
    return i2c_add_driver(&rda5807m_i2c_driver);
}

static void __exit rda5807m_exit(void)\
{
    i2c_del_driver(&rda5807m_i2c_driver);
}
late_initcall(rda5807m_init);
module_exit(rda5807m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);

