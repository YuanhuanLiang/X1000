/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.

	File: sa_playback.c

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/07/27 16:32:54
      Add the select lineout and spdif out function
      Add the interface for the gain, balance for left and right

*******************************************************************************/

#include <linux/module.h>
#include <linux/device.h>
#include <sound/soc.h>
#include "sa_sound_setting.h"


//#define printk_debug    printk
#define printk_debug(...)

/*******************************************************************************
left right balance
negative means the left 
positive means the right
*******************************************************************************/
static long lr_balance = 0;

//external function
int sa_sound_setting_lr_balance(void)
{
    return lr_balance;
}

static ssize_t get_lr_balance(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
    return sprintf(buf, "%ld\n", lr_balance);
}

static ssize_t set_lr_balance(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t len)
{
	if (strict_strtol(buf, 0, &lr_balance))
		return -EINVAL;

	return len;
}

/*******************************************************************************
DSD Gain setting
*******************************************************************************/
static long dsd_gain = 0;

//external function
int sa_sound_set_dsd_gain(void)
{
    return dsd_gain;
}

static ssize_t get_dsd_gain(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
    return sprintf(buf, "%ld\n", dsd_gain);
}

static ssize_t set_dsd_gain(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t len)
{
	if (strict_strtol(buf, 0, &dsd_gain))
		return -EINVAL;

	return len;
}

/*******************************************************************************
gain select
*******************************************************************************/
static int gain_select_flag = GAIN_SELECT_HIGH;

//external function
int sa_sound_setting_gain_select(void)
{
    return gain_select_flag;
}

static ssize_t get_gain_select(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
    if(gain_select_flag == GAIN_SELECT_LOW)
    {
        return sprintf(buf, "low");
    }
    else if(gain_select_flag == GAIN_SELECT_MIDDLE)
    {
        return sprintf(buf, "middle");
    }
    else if(gain_select_flag == GAIN_SELECT_HIGH)
    {
        return sprintf(buf, "high");
    }
    return sprintf(buf, "unknown");
}

static ssize_t set_gain_select(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t len)
{
    if(!strncmp( buf, "low", 3 ))
    {
        gain_select_flag = GAIN_SELECT_LOW;
    }
    else if(!strncmp( buf, "middle", 6 ))
    {
        gain_select_flag = GAIN_SELECT_MIDDLE;
    }
    else if(!strncmp( buf, "high", 4 ))
    {
        gain_select_flag = GAIN_SELECT_HIGH;
    }
    printk_debug("==================================set_gain_select %s\n", buf);
	return len;
}

/*******************************************************************************
Line out select for lineout and spdif
*******************************************************************************/
static int lineout_select_flag = LINEOUT_SELECT_LO; //spdif and lineout using the same output target

//external function
int sa_sound_setting_lineout_select(void)
{
    return lineout_select_flag;
}

static ssize_t get_lineout_select(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
    if(lineout_select_flag == LINEOUT_SELECT_SPDIF)
    {
        return sprintf(buf, "spdif\n");
    }
    else if(lineout_select_flag == LINEOUT_SELECT_LO)
    {
        return sprintf(buf, "lineout\n");
    }
    return sprintf(buf, "unknown\n");
}

static ssize_t set_lineout_select(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t len)
{
    if(!strncmp( buf, "spdif", 5 ))
    {
        lineout_select_flag = LINEOUT_SELECT_SPDIF;
    }
    else if(!strncmp( buf, "lineout", 6 ))
    {
        lineout_select_flag = LINEOUT_SELECT_LO;
    }
    printk_debug("==================================set_lineout_select %s\n", buf);
	return len;
}

/*******************************************************************************
Digital Filter setting
*******************************************************************************/
static int digital_filter_flag = DIGITAL_FILTER_SHORT_DELAY_ROLL_OFF;

//external function
extern int sa_sound_set_digital_filter(const char* filter);

static ssize_t get_digital_filter(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
    if (digital_filter_flag == DIGITAL_FILTER_SHARP_ROLL_OFF) {
        return sprintf(buf, "sharp_roll_off");
    } else if (digital_filter_flag == DIGITAL_FILTER_SLOW_ROLL_OFF) {
        return sprintf(buf, "slow_roll_off");
    } else if (digital_filter_flag == DIGITAL_FILTER_SHORT_DELAY_ROLL_OFF) {
        return sprintf(buf, "short_delay_roll_off");
    } else if (digital_filter_flag == DIGITAL_FILTER_SHORT_DELAY_SLOW_OFF) {
        return sprintf(buf, "short_delay_slow_off");
    } else if (digital_filter_flag == DIGITAL_FILTER_SUPER_ROLL_OFF) {
        return sprintf(buf, "super_roll_off");
    }
    return sprintf(buf, "unknown");
}

static ssize_t set_digital_filter(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t len)
{
    if (!strncmp(buf, "sharp_roll_off", 14)) {
        digital_filter_flag = GAIN_SELECT_LOW;
    } else if (!strncmp(buf, "slow_roll_off", 13)) {
        digital_filter_flag = GAIN_SELECT_MIDDLE;
    } else if (!strncmp(buf, "short_delay_roll_off", 20)) {
        digital_filter_flag = GAIN_SELECT_HIGH;
    } else if (!strncmp(buf, "short_delay_slow_off", 20)) {
        digital_filter_flag = GAIN_SELECT_HIGH;
    } else if (!strncmp(buf, "super_roll_off", 14)) {
        digital_filter_flag = GAIN_SELECT_HIGH;
    }
	sa_sound_set_digital_filter(buf);

	return len;
}

/* device attributes */
static DEVICE_ATTR(lineout_select, S_IRUGO | S_IWUSR,
		   get_lineout_select, set_lineout_select);
static DEVICE_ATTR(gain_select, S_IRUGO | S_IWUSR,
		   get_gain_select, set_gain_select);
static DEVICE_ATTR(lr_balance, S_IRUGO | S_IWUSR,
		   get_lr_balance, set_lr_balance);
static DEVICE_ATTR(dsd_gain, S_IRUGO | S_IWUSR,
		   get_dsd_gain, set_dsd_gain);
static DEVICE_ATTR(digital_filter, S_IRUGO | S_IWUSR,
		   get_digital_filter, set_digital_filter);

static struct attribute *sa_sound_setting_attributes[] = {
	&dev_attr_lineout_select.attr,
	&dev_attr_gain_select.attr,
	&dev_attr_lr_balance.attr,
	&dev_attr_dsd_gain.attr,
	&dev_attr_digital_filter.attr,
	NULL
};

static const struct attribute_group sa_sound_setting_group = {
	.attrs = sa_sound_setting_attributes,
};

static int sa_sound_setting_register_sysfs(struct platform_device* dev)
{
	int ret;

	ret = sysfs_create_group(&(dev->dev.kobj), &sa_sound_setting_group);
	if (ret < 0)
		return ret;

	return 0;
}

static int sa_sound_setting_probe(struct platform_device* dev)
{
    int ret;
    ret = sa_sound_setting_register_sysfs(dev);
	if (ret < 0)
		return ret;
    return 0;
}

static int sa_sound_setting_remove(struct platform_device* dev)
{
    return 0;
}

static struct platform_device sa_sound_setting_device = {
	.name	= "sa_sound_setting",
	.id 	= -1,
};

static struct platform_driver sa_sound_setting_driver = {
	.probe		= sa_sound_setting_probe,
	.remove		= sa_sound_setting_remove,
	.driver		= {
		.name	= "sa_sound_setting",
		.owner	= THIS_MODULE,
	},
};


static int __init sa_sound_setting_init(void)
{
    int ret = 0;
    printk_debug("sa_sound_setting_init\n");
    ret = platform_device_register( &sa_sound_setting_device );
    if (ret) {
        goto err;
    }
    
    ret = platform_driver_register( &sa_sound_setting_driver );
    if (ret) {
        goto err;
    }
    
    printk_debug("sa_sound_setting_init finish\n");
    return 0;
err:
    return ret;
}

static void __exit sa_sound_setting_exit(void)
{
}

module_init(sa_sound_setting_init);
module_exit(sa_sound_setting_exit);

/* Module information */
MODULE_AUTHOR("ringsd");
MODULE_DESCRIPTION("SmartAction Sound Setting");
MODULE_LICENSE("GPL");

/*******************************************************************************
	END OF FILE
*******************************************************************************/
