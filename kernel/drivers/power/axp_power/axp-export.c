#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#include "axp-cfg.h"
#include "axp-mfd.h"
#include "axp-regu.h"

#define i2c_open(); \
    if(!axp_dev_p){\
        printk("axp_dev_p not inited\n");\
        return -1;\
    }

#define i2c_setclk(a);
#define i2c_close();
#define AXP192_IIC_SPEED 0
#define AXP192_IIC_ADDR 0
static struct device *axp_dev_p;

static inline int i2c_read(unsigned char device, unsigned char *buf,
        unsigned char address, int count)
{
    return axp_reads(axp_dev_p, address, count, buf);
}

static inline int i2c_write(unsigned char device, unsigned char *buf,
        unsigned char address, int count)
{
    return axp_writes(axp_dev_p, address, count, buf);
}


static const int sc_adc_reg_addr[] = {0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x62, 0x64, 0x66, 0x68, 0x6a,
    0x70, 0x78, 0x7a, 0x7c, 0x7e};
static const int sc_adc_bits[]     = {12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
    24,   12,   13,   13,   12};

// adc get val, model:
// 0:  ACIN 电压  ；  0x56h  0x57h  ;  step = 1.7mv ;  0x00(0v) -- 0xfff(6.9615v)
// 1:  ACIN 电流  :   0x58h  0x59h  ;  step = 0.625ma ; 0x00(0mA) -- 0xfff(2.5594A)
// 2:  VBUS 电压  :   0x5ah  0x5bh  ;  step = 1.7mv ;  0x00(0mv) -- 0xfff(6.9615V)
// 3:  VBUS 电流  :   0x5ch  0x5dh  ;  step = 0.375ma; 0x00(0mA) -- 0xfff(1.5356A)
// 4:  AXP192 内部温度监控 ： 0x5eh  0x5fh  ; step 0.1度;  0x00 (-144.7度)  -- 0xfff (264.8度)
// 5:  TS 输入ADC 数据， 默认检测电池温度  :  0x62h  0x63h ; step = 0.8mv ; 0x00(0mv) -- 0xfff(3.276V)
// 6:  GPIO0 电压 ADC 数据.  :  0x64h  0x65h ; step = 0.5mv ;   0x00(0/0.7v) -- 0xfff (2.0475/2.7475v)
// 7:  GPIO1 电压 ADC 数据.  :  0x66h  0x67h ; step = 0.5mv ;   0x00(0/0.7v) -- 0xfff (2.0475/2.7475v)
// 8:  GPIO2 电压 ADC 数据.  :  0x68h  0x69h ; step = 0.5mv ;   0x00(0/0.7v) -- 0xfff (2.0475/2.7475v)
// 9:  GPIO3 电压 ADC 数据.  :  0x6ah  0x6bh ; step = 0.5mv ;   0x00(0/0.7v) -- 0xfff (2.0475/2.7475v)
// 10:  电池瞬时功率  :  0x70h  0x71h  0x72h
// 11:  电池电压检测  :  0x78h  0x79h   ;  step = 1.1mv ; 0x00(0mv) -- 0xfff(4.5045V);
// 12:  电池充电电流  ： 0x7ah  0x7bh   ;  step = 0.5ma ; 0x00(0mA) -- 0xfff (4.095A)
// 13:  电池放电电流  ： 0x7ch  0x7dh   ; step = 0.5ma; 0x00(0mA) -- 0xfff(4.095A)
// 14:  APS 电压  :  0x7eh  0x7fh ; step = 1.4mv ; 0x00(0mV) -- 0xfff (5.733V)
static int pmu_get_adc_value(int module, int *adc_value)
{
    int reg_num, r;
    unsigned char reg[4];

    i2c_open();
    i2c_setclk(AXP192_IIC_SPEED);

    reg_num = (sc_adc_bits[module] + 7) / 8;
    r = i2c_read(AXP192_IIC_ADDR, &reg[0], sc_adc_reg_addr[module], reg_num);
    if (0 == r)
    {
        if (12 == sc_adc_bits[module])
        {
            *adc_value = (reg[0] << 4) | (reg[1] & 0x0f);
        }
        else if (13 == sc_adc_bits[module])
        {
            *adc_value = (reg[0] << 5) | (reg[1] & 0x1f);
        }
        else
        {
            *adc_value = (reg[0] << 16) | (reg[1] << 8) | reg[2];
        }
    }

    //printf("pmu_get_adc module = %d adc_value=%d  \n", module, *adc_value  );
    i2c_close();

    return r;
}

//  pmu_set_adc_control
//  module :
//  0 :  电池电压 ADC 使能
//  1 ： 电池电流 ADC 使能
//  2 :  ACIN 电压 ADC 使能
//  3 ： ACIN 电流 ADC 使能
//  4 :  VBUS 电压 ADC 使能
//  5 ： VBUS 电流 ADC 使能
//  6 :  APS 电压 ADC 使能
//  7 ： TS 管脚 ADC 使能
//  8 :  AXP192 内部温度检测 ADC 使能
//  9 ： GPIO 0 ADC 使能
//  10 :  GPIO 1 ADC 使能
//  11 ： GPIO 2 ADC 使能
//  12 :  GPIO 3 ADC 使能

static int pmu_set_adc_control(int module, int on_off) // on_off : 1 -- on,  0 -- off;
{
    int r;
    unsigned char reg[4];


    r = i2c_read(AXP192_IIC_ADDR, &reg[0], 0x82, 2);
    if (0 != r)
    {
        i2c_close();

        return r;
    }

    if (module < 8)
    {
        if (on_off)
        {
            reg[0] |= (1 << (7 - module));
        }
        else
        {
            reg[0] &= ~(1 << (7 - module));
        }
    }
    else if (8 == module)
    {
        if (on_off)
        {
            reg[1] |= (1 << 7);
        }
        else
        {
            reg[1] &= ~(1 << 7);
        }
    }
    else
    {
        if (on_off)
        {
            reg[1] |= (1 << (12 - module));
        }
        else
        {
            reg[1] &= ~(1 << (12 - module));
        }
    }

    r = i2c_write(AXP192_IIC_ADDR, &reg[0], 0x82, 1);
    if (0 == r)
    {
        r = i2c_write(AXP192_IIC_ADDR, &reg[1], 0x83, 1);
    }
    i2c_close();

    return r;
}

static int pmu_set_adc_sample_rate(int sample_rate)
{
    int r;
    unsigned char reg[4];

    i2c_open();
    i2c_setclk(AXP192_IIC_SPEED);

    r = i2c_read(AXP192_IIC_ADDR, &reg[0], 0x84, 1);
    if (0 != r)
    {
        i2c_close();

        return r;
    }

    if (sample_rate > 150)
    {
        sample_rate = 150;
    }
    reg[0] = (reg[0] & 0x3f) | ((sample_rate / 50) << 6);

    r = i2c_write(AXP192_IIC_ADDR, &reg[0], 0x84, 1);
    i2c_close();

    return r;
}

static int pmu_set_gpio_adc_voltage_scope(int gpio, int voltage_scope)
{
    int r;
    unsigned char reg[4];

    i2c_open();
    i2c_setclk(AXP192_IIC_SPEED);

    r = i2c_read(AXP192_IIC_ADDR, &reg[0], 0x85, 1);
    if (0 != r)
    {
        i2c_close();

        return r;
    }

    if (0 == voltage_scope)
    {
        /* 0 - 2.0475 */
        reg[0] &= ~(1 << gpio);
    }
    else
    {
        /* 0.7 - 2.7475 */
        reg[0] |= (1 << gpio);
    }

    r = i2c_write(AXP192_IIC_ADDR, &reg[0], 0x85, 1);
    i2c_close();

    return r;
}

static int sc_reg_addr[] = {0x90, 0x92, 0x93, 0x95, 0x95, 0x9e};
static int sc_reg_num[]  = {2,    1,    1,    1,    1,    1};

static int pmu_set_gpio_function(int gpio, int axp_function, int cur, int output_voltage, int pull_down_on_off)
{
    int r;
    unsigned char reg[4];

    i2c_open();
    i2c_setclk(AXP192_IIC_SPEED);

    r = i2c_read(AXP192_IIC_ADDR, &reg[0], sc_reg_addr[gpio], sc_reg_num[gpio]);
    if (0 != r)
    {
        i2c_close();

        return r;
    }

    switch (gpio)
    {
        case 0:
            reg[1] = (reg[1] & 0x0f) | (((output_voltage - 1800) / 100) << 4);
        case 1:
        case 2:
            reg[0] = (reg[0] & 0x07) | (cur << 3);
            reg[0] = (reg[0] & 0xf8) | axp_function;
            r = i2c_read(AXP192_IIC_ADDR, &reg[2], 0x97, 1);
            if (0 != r)
            {
                i2c_close();

                return r;
            }

            if (pull_down_on_off)
            {
                reg[2] |= (1 << gpio);
            }
            else
            {
                reg[2] &= ~(1 << gpio);
            }
            break;
        case 3:
            reg[0] = (reg[0] & 0xfc) | axp_function;
            reg[0] |= 0x80; // enable
            break;
        case 4:
            reg[0] = (reg[0] & 0xf3) | (axp_function << 2);
            reg[0] |= 0x80; // enable
            break;
        case 5:
            reg[0] = (reg[0] & 0x3f) | (axp_function << 6);
            break;
        default:
            break;
    }

    r = i2c_write(AXP192_IIC_ADDR, &reg[0], sc_reg_addr[gpio], 1);
    if ((0 == r) && (sc_reg_num[gpio] > 1))
    {
        r = i2c_write(AXP192_IIC_ADDR, &reg[1], sc_reg_addr[gpio] + 1, 1);
    }
    if ((0 == r) && (gpio < 3))
    {
        r = i2c_write(AXP192_IIC_ADDR, &reg[2], 0x97, 1);
    }
    i2c_close();
    return r;
}

int axp_init_gpio_adc(int gpio_index)
{ 
    int ret = 0;
    ret += pmu_set_adc_sample_rate(100);
    ret += pmu_set_gpio_adc_voltage_scope( gpio_index, 1); // gpio 0 ;  adc : 0.7v -- 2.7475v  ;
    ret += pmu_set_gpio_function(gpio_index, 4, 0, 0, 0); //pmu gpio 0; function 4, adc function
    ret += pmu_set_adc_control( 9 + gpio_index, 1); // gpio0 , adc on
    return ret;
}


int axp_get_adc_value(int gpio_index, int* adc_value)
{
    return pmu_get_adc_value(6 + gpio_index, adc_value);
}

void store_axp_dev(struct device * p)
{
    axp_dev_p = p;
}

void clear_axp_dev(void)
{
    axp_dev_p = 0;
}

EXPORT_SYMBOL(store_axp_dev);
EXPORT_SYMBOL(clear_axp_dev);
EXPORT_SYMBOL(axp_get_adc_value);
EXPORT_SYMBOL(axp_init_gpio_adc);

