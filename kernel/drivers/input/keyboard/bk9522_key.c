#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/regulator/consumer.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <jz_notifier.h>
#include <linux/switch.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/input/bk9522_key.h>

struct bk9522_key_data {
    struct input_dev *input_dev;
    struct bk9522_platform_data *pdata;
    struct delayed_work delay_work;
    struct miscdevice mdev;
    struct mutex mutex;
    struct switch_dev sdev;
    unsigned int open_flag;
};

struct bk9522_init_data {
    char reg_addr;
    unsigned int writebuf;
};

struct bk9522_user_freq {
    unsigned int freq;
    unsigned int id;
};

enum {
    USER_FREQ_1 = 0,
    USER_FREQ_2,
    USER_FREQ_3,
    USER_FREQ_4,
    USER_BASE_FREQ,
};

static int sda_gpio;
static int sck_gpio;
/* USER ID FREQ */
static struct bk9522_user_freq user_id_freq[4];
/* search user band flag */
static unsigned int search_band_flag = 0;
/* disconnect link count */
static unsigned int disconnect_count = 0;
/* frequency adjustment time counter */
static unsigned int work_time_count = 0;
/* old key data copy*/
static unsigned char key_data_old = 0;
/* old key code copy*/
static unsigned int key_code_old = KEY_MAX;
/* mic switch state */
static unsigned char mic_switch_state = 0;


/* 0x00 -- 0x0B */
static unsigned int bk9522_only_write_reg[] = {
    0xDFFFFFF8,
    0x04D28057,
    0x8990E028,
    0x0412069F,
    0x50880044,
    0x00280380,
    0x5BEDFB00,
    0x1C2EC5AA,
    0xEFF1194C,
    0x085113A2,
    0x006F006F,
    0x1BD25863,
};

/* 0x1A -- 0x0x3C */
static unsigned int bk9522_read_write_reg[] = {
    0x000032C0,
    0x54B051EC,
    0x0007FCFF,
    0x00900080,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x22843D38,
    0x00D7D5F7,
    0x00000000,
    0x00000000,
    0x03460202,
    0x90F6AA3C,
    0x1F801E07,
    0x00000000,
    0x00000040,
    0x00786400,
    0x00000000,
};

static int protocol_get_ack(void)
{
    int ack_bit;
    gpio_direction_input(sda_gpio);
    gpio_set_value(sck_gpio, 1);

    if (gpio_get_value(sda_gpio)) {
        ack_bit = -1;
    } else {
        ack_bit = 0;
    }

    gpio_set_value(sck_gpio, 0);
    gpio_direction_output(sda_gpio, 1);
    return ack_bit;

}

static void protocol_start(void)
{
    gpio_direction_output(sck_gpio, 1);
    gpio_direction_output(sda_gpio, 1);
    gpio_set_value(sda_gpio, 0);
    gpio_set_value(sck_gpio, 0);
}

static void protocol_stop(void)
{
    gpio_set_value(sda_gpio, 0);
    gpio_set_value(sck_gpio, 1);
    gpio_set_value(sda_gpio, 1);
}

static void protocol_put_ack(void)
{
    gpio_set_value(sda_gpio, 0);
    gpio_set_value(sck_gpio, 1);
    gpio_set_value(sck_gpio, 0);
    gpio_set_value(sda_gpio, 1);
}

static void protocol_no_ack(void)
{
    gpio_set_value(sda_gpio, 1);
    gpio_set_value(sck_gpio, 1);
    gpio_set_value(sck_gpio, 0);
}

static unsigned char protocol_readdata(void)
{
    unsigned char i;
    unsigned char x = 0;

    gpio_direction_input(sda_gpio);
    for (i = 0; i < 8; i++) {
        gpio_set_value(sck_gpio, 1);
        x <<= 1;
        if (gpio_get_value(sda_gpio))
            x |= 0x01;

        gpio_set_value(sck_gpio, 0);
    }
    gpio_direction_output(sda_gpio, 1);
    return x;
}

static void protocol_writedata(unsigned char data)
{
    unsigned char i;

    for (i = 0; i < 8; i++) {
        if (data & 0x80)
            gpio_set_value(sda_gpio, 1);
        else
            gpio_set_value(sda_gpio, 0);

        gpio_set_value(sck_gpio, 1);
        gpio_set_value(sck_gpio, 0);
        data <<= 1;
    }
}

static int bk9522_reg_read(char reg_addr, unsigned int *readbuf)
{
    int i, err = 0;
    unsigned char *read_data = (unsigned char *) readbuf;
    *readbuf = 0;

    protocol_start( );

    protocol_writedata(BK9522_I2C_ADDR);
    if (protocol_get_ack( )) {
        printk("bk9522_reg_read : write device addr error !!!\n");
        err = -EBUSY;
        goto bk9522_reg_read_err;
    }

    protocol_writedata((reg_addr << 1) + 1);
    if (protocol_get_ack( )) {
        printk("bk9522_reg_read : write reg addr error !!!\n");
        err = -EBUSY;
        goto bk9522_reg_read_err;
    }

    for (i = 3; i > 0; i--) {
        read_data[i] = protocol_readdata( );
        protocol_put_ack( );
    }
    read_data[i] = protocol_readdata( );
    protocol_no_ack( );

    bk9522_reg_read_err:
    protocol_stop( );
    return err;
}

static int bk9522_reg_write(char reg_addr, unsigned int writebuf)
{
    int i, err = 0;
    unsigned char *write_data = (unsigned char *) &writebuf;
    protocol_start( );

    protocol_writedata(BK9522_I2C_ADDR);
    if (protocol_get_ack( )) {
        printk("bk9522_reg_write : write device addr error !!!\n");
        err = -EBUSY;
        goto bk9522_reg_write_err;
    }

    protocol_writedata(reg_addr << 1);
    if (protocol_get_ack( )) {
        printk("bk9522_reg_write : write reg addr error !!!\n");
        err = -EBUSY;
        goto bk9522_reg_write_err;
    }

    for (i = 3; i >= 0; i--) {
        protocol_writedata(write_data[i]);
        if (protocol_get_ack( )) {
            printk("bk9522_reg_write : write data error !!!\n");
            err = -EBUSY;
            goto bk9522_reg_write_err;
        }
    }
    bk9522_reg_write_err:
    protocol_stop( );
    return err;
}

static int bk9522_calibration_clock(void)
{
    int err = 0;

    /* enable calibration clock */
    bk9522_only_write_reg[BK9522_REG_LDO] |= (1 << 25);
    err |= bk9522_reg_write(BK9522_REG_LDO, bk9522_only_write_reg[BK9522_REG_LDO]);

    /* calibrate RF VCO */
    bk9522_only_write_reg[BK9522_REG_DIV_FREQ] &= ~(1 << 22);
    err |= bk9522_reg_write(BK9522_REG_DIV_FREQ, bk9522_only_write_reg[BK9522_REG_DIV_FREQ]);
    bk9522_only_write_reg[BK9522_REG_DIV_FREQ] |= (1 << 22);
    err |= bk9522_reg_write(BK9522_REG_DIV_FREQ, bk9522_only_write_reg[BK9522_REG_DIV_FREQ]);

    msleep(5);

    /* calibrate digital VCO */
    bk9522_only_write_reg[BK9522_REG_FUNCTION0] &= ~(1 << 25);
    err |= bk9522_reg_write(BK9522_REG_FUNCTION0, bk9522_only_write_reg[BK9522_REG_FUNCTION0]);
    bk9522_only_write_reg[BK9522_REG_FUNCTION0] |= (1 << 25);
    err |= bk9522_reg_write(BK9522_REG_FUNCTION0, bk9522_only_write_reg[BK9522_REG_FUNCTION0]);

    /* disable calibration clock */
    bk9522_only_write_reg[BK9522_REG_LDO] &= ~(1 << 25);
    err |= bk9522_reg_write(BK9522_REG_LDO, bk9522_only_write_reg[BK9522_REG_LDO]);

    return err;
}

static int bk9522_set_id_freq(unsigned int id, unsigned int freq)
{
    int err = 0;
    unsigned short freq_data = freq / 100;
    unsigned int tmp;

    /* set UBAND and freq*/
    bk9522_only_write_reg[BK9522_REG_DIV_FREQ] &= ~(1 << 21);
    bk9522_only_write_reg[BK9522_REG_DIV_FREQ] |= (1 << 20);
    bk9522_only_write_reg[BK9522_REG_DIV_FREQ] &= ~(7 << 13);
    if (freq_data < 7100)
        bk9522_only_write_reg[BK9522_REG_DIV_FREQ] |= (1 << 13);

    err |= bk9522_reg_write(BK9522_REG_DIV_FREQ, bk9522_only_write_reg[BK9522_REG_DIV_FREQ]);

    /* enable UBAND LDO */
    bk9522_only_write_reg[BK9522_REG_LDO] |= (7 << 17);
    err |= bk9522_reg_write(BK9522_REG_LDO, bk9522_only_write_reg[BK9522_REG_LDO]);

    /* select PLL UBAND */
    bk9522_only_write_reg[BK9522_REG_PLL] &= ~(3 << 18);
    bk9522_only_write_reg[BK9522_REG_PLL] |= (3 << 20);
    err |= bk9522_reg_write(BK9522_REG_PLL, bk9522_only_write_reg[BK9522_REG_PLL]);

    /* set user freq */
    if (freq_data >= 7100) {
        tmp = (freq_data - 7100) / 3 * 0x64000 + 0x39CB147A;
        tmp += ((freq_data - 7100) % 3) * 136533;
    } else {
        tmp = (freq_data - 5000) * 0x32000 + 0x3D0E1EB8;
    }
    err |= bk9522_reg_write(BK9522_REG_CHANNEL_FREQ, tmp);

    /* set user id */
    err |= bk9522_reg_write(BK9522_REG_MATCH_ID, id);

    /* calibration_clock */
    err |= bk9522_calibration_clock( );

    return err;
}

static int bk9522_audio_mute(unsigned char flag)
{
    int err = 0;
    unsigned int bk9522_reg_value;
    err |= bk9522_reg_read(BK9522_REG_EQ_CONTROL, &bk9522_reg_value);

    if (flag) {
        bk9522_reg_value &= ~(1 << 20);
    } else {
        bk9522_reg_value |= (1 << 20);
    }

    err |= bk9522_reg_write(BK9522_REG_EQ_CONTROL, bk9522_reg_value);
    return err;
}

static int bk9522_tune_user_freq(unsigned char index)
{
    if (index >= USER_BASE_FREQ)
        return bk9522_set_id_freq(BASE_USER_ID, BASE_FREQ_START);
    else
        return bk9522_set_id_freq(user_id_freq[index].id, user_id_freq[index].freq);
}

static int bk9522_restart_receive(void)
{
    int err = 0;
    unsigned int bk9522_reg_value;
    /* restart receive */
    err |= bk9522_reg_read(BK9522_REG_RECEIVE_CONTROL, &bk9522_reg_value);
    bk9522_reg_value &= ~(1 << 3);
    bk9522_reg_value &= ~(0xF << 12);
    err |= bk9522_reg_write(BK9522_REG_RECEIVE_CONTROL, bk9522_reg_value);
    msleep(3);
    err |= bk9522_reg_read(BK9522_REG_RECEIVE_CONTROL, &bk9522_reg_value);
    bk9522_reg_value &= ~(1 << 4);
    bk9522_reg_value |= (0x3 << 12);
    err |= bk9522_reg_write(BK9522_REG_RECEIVE_CONTROL, bk9522_reg_value);
    bk9522_reg_value |= (1 << 3);
    bk9522_reg_value |= (1 << 4);
    err |= bk9522_reg_write(BK9522_REG_RECEIVE_CONTROL, bk9522_reg_value);
    msleep(3);

    /* clean sync irq */
    err |= bk9522_reg_read(BK9522_REG_DATA, &bk9522_reg_value);
    bk9522_reg_value |= (1 << 2);
    err |= bk9522_reg_write(BK9522_REG_DATA, bk9522_reg_value);
    return err;
}

static int bk9522_freq_finetune(void)
{
    int err = 0;
    int deltaf, x_val;
    unsigned char fifo_num, ctune;
    unsigned int bk9522_reg_value;

    static int deltaf_old;
    static int finetune_count;

    err |= bk9522_reg_read(BK9522_REG_FIFO, &bk9522_reg_value);
    fifo_num = (bk9522_reg_value >> 16) & 0x3F;
    /* frequency deviation */
    deltaf = (bk9522_reg_value & 0x7FFF) / 128;

    if ((deltaf - deltaf_old) < 3 && (deltaf - deltaf_old) > -3) {
        deltaf_old = deltaf;
    } else {
        deltaf_old = deltaf;
        deltaf = 0;
    }

    if ((deltaf >= 6) || (deltaf <= -6)) {
        x_val = deltaf;
        finetune_count = 10;
    } else {
        if (finetune_count > 3) {
            if (fifo_num > 20)
                x_val = 1;
            else if (fifo_num < 12 && deltaf <= 0)
                x_val = -1;
            else
                x_val = 0;

            finetune_count = 1;
        } else {
            x_val = 0;
            finetune_count++;
        }
    }

    err |= bk9522_reg_read(BK9522_REG_RESET, &bk9522_reg_value);
    ctune = bk9522_reg_value & 0x7F;
    x_val = ctune - x_val;

    if (x_val > 0x7F)
        x_val = 0x7F;
    else if (x_val < 0)
        x_val = 0x0;

    bk9522_reg_value |= (1 << 7) | x_val;
    err |= bk9522_reg_write(BK9522_REG_RESET, bk9522_reg_value);

    if (finetune_count == 10) {
        finetune_count = 0;
        err |= bk9522_restart_receive( );
    }

    return err;
}

static int bk9522_soft_reset(void)
{
    int err = 0;
    unsigned int bk9522_reg_value;

    err |= bk9522_reg_read(BK9522_REG_RESET, &bk9522_reg_value);
    bk9522_reg_value &= ~(1 << 7);
    err |= bk9522_reg_write(BK9522_REG_RESET, bk9522_reg_value);
    bk9522_reg_value = (1 << 7);
    err |= bk9522_reg_write(BK9522_REG_RESET, bk9522_reg_value);

    return err;
}

static int bk9522_check_id(void)
{
    int i;
    unsigned int bk9522_reg_value;

    for (i = 0; i < CHECK_ID_COUNT; i++) {
        bk9522_reg_read(BK9522_REG_ID, &bk9522_reg_value);

        if ((bk9522_reg_value & 0xFFFF) == 0x9522) {
            return 0;
        }

    }

    return -1;
}

static int bk9522_init(void)
{
    int i;

    if (bk9522_check_id( ))
        return -ENODEV;

    for (i = 0; i < ARRAY_SIZE(bk9522_only_write_reg); i++) {
        if (bk9522_reg_write(i, bk9522_only_write_reg[i]))
            return -ENODEV;
    }
    for (i = 0; i < ARRAY_SIZE(bk9522_read_write_reg); i++) {
        if (bk9522_reg_write(i + 0x1A, bk9522_read_write_reg[i]))
            return -ENODEV;
    }

    msleep(10);

    if (bk9522_soft_reset( ))
        return -ENODEV;

    msleep(5);
    return 0;
}

static int bk9522_pair(unsigned char *freq_id_data)
{
    int count = 0, tmp = 0, success_falg = 0;
    unsigned char data[32] = { 0 };
    unsigned char rx_data = 0;
    unsigned char rx_state = STATE_TsdIdle;
    unsigned char check_data;
    unsigned char data_num;
    unsigned int bk9522_reg_value;

    if (bk9522_tune_user_freq(USER_BASE_FREQ))
        return -ENODEV;

    msleep(2);

    if (bk9522_soft_reset( ))
        return -ENODEV;

    if (bk9522_calibration_clock( ))
        return -ENODEV;

    msleep(5);

    count = PAIR_TIME * 1000;
    while (count--) {
        if (bk9522_reg_read(BK9522_REG_DATA, &bk9522_reg_value))
            return -ENODEV;

        rx_data = (bk9522_reg_value >> 8) & 0xFF;

        if (rx_data == LEAD_TsdStart) {
            check_data = 0;
            data_num = 0;
            rx_state = STATE_TsdDataLo;
        }

        switch (rx_state) {
            case STATE_TsdIdle:
                break;
            case STATE_TsdStart:
                if (rx_data == LEAD_TsdStart) {
                    check_data = 0;
                    data_num = 0;
                    rx_state = STATE_TsdDataLo;
                }
                break;
            case STATE_TsdDataLo:
                if ((rx_data & 0xF0) == LEAD_TsdDataLo) {
                    data[data_num] = rx_data & 0x0F;
                    rx_state = STATE_TsdDataHi;
                }
                break;
            case STATE_TsdDataHi:
                if ((rx_data & 0xF0) == LEAD_TsdDataHi) {
                    data[data_num] += ((rx_data & 0x0F) << 4);
                    check_data += data[data_num];
                    if (++data_num < sizeof(data))
                        rx_state = STATE_TsdDataLo;
                    else
                        rx_state = STATE_TsdChkLo;
                }
                break;
            case STATE_TsdChkLo:
                if ((rx_data & 0xF0) == LEAD_TsdChkLo) {
                    tmp = rx_data & 0x0F;
                    rx_state = STATE_TsdChkHi;
                }
                break;
            case STATE_TsdChkHi:
                if ((rx_data & 0xF0) == LEAD_TsdChkHi) {
                    tmp += ((rx_data & 0x0F) << 4);
                    rx_state = STATE_TsdIdle;
                    if (check_data == tmp) {
                        success_falg = 1;
                        goto read_pair_success;
                    }
                }
                break;
            default:
                break;
        }
        usleep_range(1000, 1500);
    }

    read_pair_success:
    if (success_falg) {
        memcpy(freq_id_data, data, sizeof(data));
        return 0;
    }

    return -EAGAIN;
}

static void bk9522_work_handler(struct work_struct *work)
{
    int i;
    unsigned char key_data;
    unsigned int user_band_num;
    unsigned int bk9522_reg_value;
    unsigned int tmp;
    unsigned int signal_strength = 0;
    unsigned int match_successful_flag = 0;
    struct delayed_work *dwork = to_delayed_work(work);
    struct bk9522_key_data *bk9522_key = container_of(dwork, struct bk9522_key_data, delay_work);
    struct bk9522_platform_data *pdata = bk9522_key->pdata;

    /* search user band */
    if (search_band_flag) {
        for (i = 0; i < USER_BASE_FREQ; i++) {
            bk9522_tune_user_freq(i);
            msleep(600);
            bk9522_reg_read(BK9522_REG_DATA, &bk9522_reg_value);
            if (bk9522_reg_value & (1 << 5)) {
                tmp = (bk9522_reg_value >> 16) & 0xFF;
                if (tmp > signal_strength) {
                    signal_strength = tmp;
                    user_band_num = i;
                    match_successful_flag = 1;
                }
            }
        }

        if (match_successful_flag) {
            bk9522_tune_user_freq(user_band_num);
            search_band_flag = 0;
            printk("bk9522 search user band success: use band %d\n", user_band_num);
            if(mic_switch_state == 0) {
                bk9522_audio_mute(0);
                mic_switch_state = 1;
                switch_set_state(&bk9522_key->sdev, 1);
            }
        } else {
            if(mic_switch_state == 1) {
                bk9522_audio_mute(1);
                mic_switch_state = 0;
                switch_set_state(&bk9522_key->sdev, 0);
            }
        }

    } else {
        /* get data */
        bk9522_reg_read(BK9522_REG_DATA, &bk9522_reg_value);
        /* connection status */
        if (bk9522_reg_value & (1 << 5)) {
            disconnect_count = 0;
            key_data = (bk9522_reg_value >> 8) & 0xFF;
        } else {
            disconnect_count++;
            key_data = 0;
            if (disconnect_count > DISCONNECT_COUNT_MAX) {
                disconnect_count = 0;
                search_band_flag = 1;
            }
        }

        /* input event  */
        if(key_data != 0) {
            if(key_data_old == key_data) {
                if(key_code_old == KEY_MAX) {
                    for (i = 0; i < pdata->key_num; i++) {
                        if (key_data == pdata->keys[i].data) {
                            key_code_old = pdata->keys[i].code;
                            input_event(bk9522_key->input_dev, EV_KEY, key_code_old, 1);
                            input_sync(bk9522_key->input_dev);
                            break;
                        }
                    }
                }
            }
        } else {
            if(key_code_old != KEY_MAX) {
                input_event(bk9522_key->input_dev, EV_KEY, key_code_old, 0);
                input_sync(bk9522_key->input_dev);
                key_code_old = KEY_MAX;
            }
        }
        key_data_old = key_data;

        /* fifo overflow */
        bk9522_reg_read(BK9522_REG_FIFO, &bk9522_reg_value);
        if (((bk9522_reg_value >> 16) & 0x3F) >= FIFO_DATA_THRESHOLD) {
            bk9522_restart_receive( );
        }

        /* temperature drift */
        bk9522_reg_read(BK9522_REG_TEMP_DRIFT, &bk9522_reg_value);
        if (bk9522_reg_value & 0x03) {
            bk9522_calibration_clock( );
        }

        /* Dynamically adjust the frequency */
        work_time_count++;
        if (work_time_count >= 100) {
            work_time_count = 0;

            /* enable int1 irq */
            bk9522_reg_read(BK9522_REG_RECEIVE_CONTROL, &tmp);
            tmp |= (1 << 17);
            bk9522_reg_write(BK9522_REG_RECEIVE_CONTROL, tmp);

            msleep(2);

            /* read int1 irq falg */
            bk9522_reg_read(BK9522_REG_DATA, &bk9522_reg_value);

            /* distable int1 irq */
            tmp &= ~(1 << 17);
            bk9522_reg_write(BK9522_REG_RECEIVE_CONTROL, tmp);

            if (bk9522_reg_value & (1 << 1)) {
                /* clean int1 irq flag */
                bk9522_reg_value = (1 << 1);
                bk9522_reg_write(BK9522_REG_DATA, bk9522_reg_value);

                bk9522_freq_finetune( );
            }

        }
    }

    schedule_delayed_work(dwork, msecs_to_jiffies(WORK_DELAY_TIME));
}
static int bk9522_key_open(struct inode *inode, struct file *file)
{
    int err = 0;
    struct miscdevice *mdev = file->private_data;
    struct bk9522_key_data *bk9522_key = container_of(mdev, struct bk9522_key_data, mdev);

    mutex_lock(&bk9522_key->mutex);

    if (bk9522_key->open_flag) {
        err = -EBUSY;
        goto bk9522_key_open_error;
    }

    if (bk9522_key->pdata->power != -1) {
        gpio_set_value(bk9522_key->pdata->power, bk9522_key->pdata->power_level_en);
        msleep(100);
    }

    /*check ID */
    err = bk9522_init( );
    if (err) {
        if (bk9522_key->pdata->power != -1)
            gpio_set_value(bk9522_key->pdata->power, !bk9522_key->pdata->power_level_en);
        goto bk9522_key_open_error;
    }

    bk9522_key->open_flag = 1;
    mutex_unlock(&bk9522_key->mutex);
    return 0;

bk9522_key_open_error:
    mutex_unlock(&bk9522_key->mutex);
    printk("bk9522_key_open error\n");
    return err;
}

static int bk9522_key_release(struct inode *inode, struct file *file)
{
    struct miscdevice *mdev = file->private_data;
    struct bk9522_key_data *bk9522_key = container_of(mdev, struct bk9522_key_data, mdev);
    struct bk9522_platform_data *pdata = bk9522_key->pdata;

    mutex_lock(&bk9522_key->mutex);

    bk9522_key->open_flag = 0;

    cancel_delayed_work_sync(&bk9522_key->delay_work);

    if(key_code_old != KEY_MAX) {
        input_event(bk9522_key->input_dev, EV_KEY, key_code_old, 0);
        input_sync(bk9522_key->input_dev);
        key_code_old = KEY_MAX;
    }

    if (pdata->power != -1)
        gpio_set_value(pdata->power, !pdata->power_level_en);

    mutex_unlock(&bk9522_key->mutex);
    return 0;
}

static long bk9522_key_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int err = 0;
    void __user *argp = (void __user *) arg;
    struct miscdevice *mdev = file->private_data;
    struct bk9522_key_data *bk9522_key = container_of(mdev, struct bk9522_key_data, mdev);

    mutex_lock(&bk9522_key->mutex);

    cancel_delayed_work_sync(&bk9522_key->delay_work);
    if(key_code_old != KEY_MAX) {
        input_event(bk9522_key->input_dev, EV_KEY, key_code_old, 0);
        input_sync(bk9522_key->input_dev);
        key_code_old = KEY_MAX;
    }

    if(mic_switch_state == 1) {
        bk9522_audio_mute(1);
        mic_switch_state = 0;
        switch_set_state(&bk9522_key->sdev, 0);
    }

    switch (cmd) {
        case BK9522_PAIR:
            err = bk9522_pair((unsigned char *) user_id_freq);
            if (err) {
                printk("bk9522_key_ioctl: bk9522_pair error\n");
                goto bk9522_key_ioctl_error;
            }

            copy_to_user(argp, user_id_freq, sizeof(user_id_freq));
            break;

        case BK9522_START:
            copy_from_user(user_id_freq, argp, sizeof(user_id_freq));
            search_band_flag = 1;
            disconnect_count = 0;
            work_time_count = 0;
            schedule_delayed_work(&bk9522_key->delay_work, 0);
            break;

        case BK9522_STOP:
            break;

        default:
            err = -EINVAL;
    }

bk9522_key_ioctl_error:
    mutex_unlock(&bk9522_key->mutex);
    return err;
}

static struct file_operations bk9522_fops = {
    .owner = THIS_MODULE,
    .open = bk9522_key_open,
    .release = bk9522_key_release,
    .unlocked_ioctl = bk9522_key_ioctl,
};

static ssize_t switch_mic_print_name(struct switch_dev *sdev, char *buf)
{
        return sprintf(buf,"%s.\n",sdev->name);
}

static ssize_t switch_mic_print_state(struct switch_dev *sdev, char *buf)
{
        char *state[2] = {"0", "1"};
        unsigned int state_val = switch_get_state(sdev);

        if (state_val == 1)
                return sprintf(buf, "%s\n", state[1]);
        else
                return sprintf(buf, "%s\n", state[0]);
}

static int bk9522_key_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct bk9522_platform_data *pdata = dev_get_platdata(dev);
    struct bk9522_key_data *bk9522_key;
    struct input_dev *input_dev;
    int i, err = 0;

    err = gpio_request_one(pdata->sda_gpio, GPIOF_OUT_INIT_HIGH, "bk9522_sda");
    if (err) {
        dev_err(dev, "bk9522_key_probe: failed to request GPIO %d, error %d\n", pdata->sda_gpio, err);
        goto exit_request_gpio_failed;
    }

    err = gpio_request_one(pdata->sck_gpio, GPIOF_OUT_INIT_HIGH, "bk9522_sck");
    if (err) {
        dev_err(dev, "bk9522_key_probe: failed to request GPIO %d, error %d\n", pdata->sck_gpio, err);
        gpio_free(pdata->sda_gpio);
        goto exit_request_gpio_failed;
    }

    sda_gpio = pdata->sda_gpio;
    sck_gpio = pdata->sck_gpio;

    bk9522_key = kzalloc(sizeof(struct bk9522_key_data), GFP_KERNEL);
    if (!bk9522_key) {
        err = -ENOMEM;
        goto exit_alloc_data_failed;
    }

    if (pdata->power != -1) {
        err = gpio_request(pdata->power, "bk9522_ce");
        if (err) {
            dev_err(dev, "bk9522_key_probe: failed to request power %d, error %d\n", pdata->power, err);
            goto exit_request_power_failed;
        }
        gpio_direction_output(pdata->power, !pdata->power_level_en);
    }

    bk9522_key->pdata = pdata;
    INIT_DELAYED_WORK(&bk9522_key->delay_work, bk9522_work_handler);


    bk9522_key->sdev.name = BK9522_SWITCH_NAME;
    bk9522_key->sdev.print_state = switch_mic_print_state;
    bk9522_key->sdev.print_name  = switch_mic_print_name;

    err = switch_dev_register(&bk9522_key->sdev);
    if (err < 0) {
        dev_err(dev, "bk9522_key_probe: wl_mic switch dev register fail\n");
        goto exit_switch_dev_register_failed;
    }

    input_dev = input_allocate_device( );
    if (!input_dev) {
        err = -ENOMEM;
        dev_err(dev, "bk9522_key_probe: failed to allocate input device\n");
        goto exit_input_dev_alloc_failed;
    }

    bk9522_key->input_dev = input_dev;
    input_set_drvdata(input_dev, bk9522_key);
    platform_set_drvdata(pdev, bk9522_key);

    __set_bit(EV_KEY, input_dev->evbit);
    __set_bit(EV_SYN, input_dev->evbit);
    for (i = 0; i < pdata->key_num; i++)
        input_set_capability(input_dev, EV_KEY, pdata->keys[i].code);

    input_dev->name = BK9522_NAME;
    input_dev->id.bustype = BUS_HOST;
    input_dev->id.vendor = 0x0010;
    input_dev->id.product = 0x0001;
    input_dev->id.version = 0x0100;

    err = input_register_device(input_dev);
    if (err) {
        dev_err(dev, "bk9522_key_probe: failed to register input device\n");
        goto exit_input_register_device_failed;
    }

    mutex_init(&bk9522_key->mutex);
    bk9522_key->mdev.minor = MISC_DYNAMIC_MINOR;
    bk9522_key->mdev.name = BK9522_NAME;
    bk9522_key->mdev.fops = &bk9522_fops;
    err = misc_register(&bk9522_key->mdev);
    if (err) {
        dev_err(dev, "bk9522_key_probe: misc_register failed\n");
        goto exit_misc_register_failed;
    }

    return 0;

exit_misc_register_failed:
    input_unregister_device(input_dev);
exit_input_register_device_failed:
    input_free_device(input_dev);
exit_input_dev_alloc_failed:
    switch_dev_unregister(&bk9522_key->sdev);
exit_switch_dev_register_failed:
    if (pdata->power != -1)
        gpio_free(pdata->power);
exit_request_power_failed:
    kfree(bk9522_key);
exit_alloc_data_failed:
    gpio_free(sck_gpio);
    gpio_free(sda_gpio);
exit_request_gpio_failed:
    platform_set_drvdata(pdev, NULL);
    return err;
}

static int bk9522_key_remove(struct platform_device *pdev)
{
    struct bk9522_key_data *bk9522_key = platform_get_drvdata(pdev);

    misc_deregister(&bk9522_key->mdev);
    input_unregister_device(bk9522_key->input_dev);
    input_free_device(bk9522_key->input_dev);
    switch_dev_unregister(&bk9522_key->sdev);
    if (bk9522_key->pdata->power != -1)
        gpio_free(bk9522_key->pdata->power);
    kfree(bk9522_key);
    gpio_free(sck_gpio);
    gpio_free(sda_gpio);
    platform_set_drvdata(pdev, NULL);
    return 0;
}

static struct platform_driver bk9522_key_driver = {
    .probe = bk9522_key_probe,
    .remove = bk9522_key_remove,
    .driver = {
        .name = BK9522_NAME,
        .owner = THIS_MODULE,
    },
};

static int __init bk9522_key_init(void)
{
    return platform_driver_register(&bk9522_key_driver);
}

static void __exit bk9522_key_exit(void)
{
    platform_driver_unregister(&bk9522_key_driver);
}

late_initcall(bk9522_key_init);
module_exit(bk9522_key_exit);

MODULE_AUTHOR("<shuan.xin@ingenic.com>");
MODULE_DESCRIPTION("BK9522 Key Driver");
MODULE_LICENSE("GPL");
