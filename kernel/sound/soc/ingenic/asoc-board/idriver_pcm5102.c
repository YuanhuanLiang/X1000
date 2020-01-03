
/*
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 *	http://www.ingenic.com
 * Author: cli <chen.li@ingenic.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <board.h>
#include "../icodec/icdc_d3.h"
#include "sa_sound_switch.h"
#include "sa_sound_dapm.h"

#define GPIO_SET(id, name, level) \
    ret = gpio_request(id, name);\
if (0 == ret)\
{\
    gpio_direction_output(id, level);\
}\
else\
{\
    dprintf("request gpio ["name"] failed.\n");\
}
#if 0
#define dprintf    dprintf
#else
#define dprintf(...)
#endif

static int headphone_jack_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    dprintf("%s widget:%s event:%s\n", __FUNCTION__, widget->name, event_to_ename(event));
    return 0;
}


static int headphone_amp_power_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    dprintf("%s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

static int dac_out_power_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    dprintf("%s, %d\n", __FUNCTION__, __LINE__);
    return 0;
}

DEFINE_POWER_CTRL(dac) 

static int headphone_amp_power_flag = 0;                                               

static int headphone_amp_power_get(struct snd_kcontrol *kcontrol,                      
        struct snd_ctl_elem_value *ucontrol)                                    
{                                                                               
    ucontrol->value.integer.value[0] = headphone_amp_power_flag;                       

    return 0;                                                                   
}                                                                               

static int headphone_amp_power_put(struct snd_kcontrol *kcontrol,                      
        struct snd_ctl_elem_value *ucontrol)                                    
{                                                                               
    int val = ucontrol->value.integer.value[0];
    dprintf("%s, %d val:%d\n", __FUNCTION__, __LINE__, val);
    switch(val)
    {
        case 0:
            gpio_direction_output(GPIO_DAC_N_MUTE, 0);
            mdelay(100);
            gpio_direction_output(GPIO_HP_MUTE, 0);
            mdelay(10);
            gpio_direction_output(GPIO_PO_PWR_EN, 0);
            break;
        case 1:
            gpio_direction_output(GPIO_DAC_N_MUTE, 0);
            gpio_direction_output(GPIO_HP_MUTE, 0);
            mdelay(10);
            gpio_direction_output(GPIO_PO_PWR_EN, 1);
            mdelay(1300);
            gpio_direction_output(GPIO_HP_MUTE, 1);
            mdelay(100);
            gpio_direction_output(GPIO_DAC_N_MUTE, 1);
            break;
    }
    headphone_amp_power_flag = val;                       
    return 0;                                                                   
}                                                                               

static int headphone_amp_power_connected(struct snd_soc_dapm_widget *source,           
        struct snd_soc_dapm_widget *sink)                                  
{                                                                               
    return headphone_amp_power_flag;                                                   
}                                                                               

const char* event_to_ename(int event)
{
    switch(event)
    {
        case SND_SOC_DAPM_PRE_PMU:
            return "SND_SOC_DAPM_PRE_PMU";
        case SND_SOC_DAPM_POST_PMU:
            return "SND_SOC_DAPM_POST_PMU";
        case SND_SOC_DAPM_PRE_PMD:
            return "SND_SOC_DAPM_PRE_PMD";
        case SND_SOC_DAPM_POST_PMD:
            return "SND_SOC_DAPM_POST_PMD";
        case SND_SOC_DAPM_PRE_POST_PMD:
            return "SND_SOC_DAPM_PRE_POST_PMD";
        default:
            dprintf("unknown event:%d\n", event);
            return "SND_SOC_DAPM_UNKNOWN";
    }
    return NULL;
}

static const struct snd_soc_dapm_widget sa_sound_platform_widgets[] = {
    //in
    SND_SOC_DAPM_INPUT("I2S In"),

    SND_SOC_DAPM_INPUT("DAC In"),

    SND_SOC_DAPM_SUPPLY("DAC Power", SND_SOC_NOPM, 0, 0, dac_out_power_event, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD ),

    SND_SOC_DAPM_SUPPLY("Headphone AMP Power", SND_SOC_NOPM, 0, 0, headphone_amp_power_event, SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD ),
    SND_SOC_DAPM_INPUT("Headphone AMP In"),

    //out
    SND_SOC_DAPM_OUTPUT("DAC Out"),
    SND_SOC_DAPM_HP("Headphone Jack", headphone_jack_event),
};

/*******************************************************************************
  kcontrol path:
SPDIF:
{Spdif In} -> {HBC2500 Spdif Out} -> {Spdif Jack In} -> {Spdif Jack Out}
LO:
{DAC Power                  } -> +
{HBC2500 I2S_DSD Out} -> {DAC In} -> {DAC Out} -> {Lineout AMP In} -> {Line Out}
PO: 
{DAC Power               } -> +                    {Headphone AMP Power} -> +
{HBC2500 I2S_DSD Out} -> {DAC In} -> {DAC Out                      } -> {Headphone AMP In} -> {Headphone Jack}
 ********************************************************************************/
static const struct snd_soc_dapm_route sa_sound_platform_audio_map[]={
    /* Dac I2S_DSD In --> I2S In */
    {"HBC2500 I2S_DSD In", NULL, "I2S In"},

    /* HBC2500 I2S_DSD Out --> Dac In */

    {"DAC In", NULL, "HBC2500 I2S_DSD Out"},
    {"DAC In", NULL, "DAC Power", dac_power_connected},

    {"DAC Out", NULL, "DAC In"},

    /* DAC_OUT --> Headphone AMP In */
    {"Headphone AMP In", NULL, "DAC Out"},
    {"Headphone AMP In", NULL, "Headphone AMP Power", headphone_amp_power_connected},
    {"Headphone Jack", NULL, "Headphone AMP In"},
} ;

static const struct snd_kcontrol_new sa_sound_platform_controls[] = {
    SOC_SINGLE_EXT("Headphone AMP Power Switch", 0, 0, 1, 0, headphone_amp_power_get, headphone_amp_power_put),
    SOC_SINGLE_EXT("DAC Power Switch", 0, 0, 1, 0, dac_power_get, dac_power_put),
    SOC_DAPM_PIN_SWITCH("Headphone Jack"),
};

void idriver_i2s_shutdown(struct snd_pcm_substream *sps)
{
    dprintf( "%s %d\n", __FUNCTION__, __LINE__ );
    gpio_direction_output(GPIO_DAC_N_MUTE, 0);
    gpio_direction_output(GPIO_HP_MUTE, 0);
    return;
}

int idriver_i2s_startup(struct snd_pcm_substream *sps)
{
    dprintf( "%s %d\n", __FUNCTION__, __LINE__ );
    return 0;
}

int idriver_i2s_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params) {
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    int ret;

    unsigned int sample_rate = params_rate(params);

    dprintf( "%s %d\n", __FUNCTION__, __LINE__ );

    /*FIXME snd_soc_dai_set_pll*/
    ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_CBS_CFM);

    if (ret)
        return ret;

    dprintf( "%s %d\n", __FUNCTION__, __LINE__ );
    ret = snd_soc_dai_set_sysclk(cpu_dai, JZ_I2S_EX_CODEC, sample_rate, SND_SOC_CLOCK_OUT);
    if (ret)
        return ret;

    dprintf( "%s %d\n", __FUNCTION__, __LINE__ );

    return 0;
};

int idriver_i2s_hw_free(struct snd_pcm_substream *substream)
{
    /*notify release pll*/
    return 0;
};

int idriver_prepare(struct snd_pcm_substream *substream)
{
    /*notify release pll*/
    dprintf("%s\n", __FUNCTION__);
    //gpio_direction_output(GPIO_DAC_N_MUTE, 0);
    //gpio_direction_output(GPIO_HP_MUTE, 0);
    return 0;
};
int idriver_trigger(struct snd_pcm_substream *substream, int cmd)
{
    /*notify release pll*/
    dprintf("%s,cmd:%d\n", __FUNCTION__,cmd);
    if(cmd){
        gpio_direction_output(GPIO_HP_MUTE, 1);
        gpio_direction_output(GPIO_DAC_N_MUTE, 1);
    }
    else{
        gpio_direction_output(GPIO_DAC_N_MUTE, 0);
        mdelay(100);
        gpio_direction_output(GPIO_HP_MUTE, 0);
        mdelay(100);
    }
    return 0;
};
static struct snd_soc_ops idriver_i2s_ops = {
    .startup = idriver_i2s_startup,
    .shutdown = idriver_i2s_shutdown,
    .hw_params = idriver_i2s_hw_params,
    .hw_free = idriver_i2s_hw_free,
    .trigger = idriver_trigger,
    .prepare = idriver_prepare,
};

#ifdef HAVE_HEADPHONE
static struct snd_soc_jack_gpio idriver_icdc_d3_jack_gpio[] = {
    {
        .name = "Headphone detection",
        .report = SND_JACK_HEADPHONE,
        .debounce_time = 150,
    }
};
#endif


static int idriver_dlv_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
    return 0;
}

static struct snd_soc_dai_link idriver_dais[] = {
    [0] = {
        .name = "idriver-pcm5102",
        .stream_name = "idriver-pcm5102",
        .platform_name = "jz-asoc-aic-dma",
        .cpu_dai_name = "jz-asoc-aic-i2s",
        .codec_dai_name = "pcm5102-hifi",
        .codec_name = "pcm5102",
        .init = idriver_dlv_dai_link_init,
        .ops = &idriver_i2s_ops,
    },
};

static struct snd_soc_card idriver = {
    .name = "idriver",
    .owner = THIS_MODULE,
    .dai_link = idriver_dais,
    .num_links = ARRAY_SIZE(idriver_dais),
    .controls = sa_sound_platform_controls,
    .num_controls = ARRAY_SIZE(sa_sound_platform_controls),
    .dapm_widgets = sa_sound_platform_widgets,
    .num_dapm_widgets = ARRAY_SIZE(sa_sound_platform_widgets),
    .dapm_routes = sa_sound_platform_audio_map,
    .num_dapm_routes = ARRAY_SIZE(sa_sound_platform_audio_map),
};

static int snd_idriver_probe(struct platform_device *pdev)
{
    int ret = 0;
    idriver.dev = &pdev->dev;
    ret = snd_soc_register_card(&idriver);
    if (ret)
        dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
    GPIO_SET(GPIO_HP_MUTE, "HP Mute", 0);
    GPIO_SET(GPIO_LINE_MUTE, "Line Mute", 0);
    GPIO_SET(GPIO_DAC_N_MUTE,  "DAC Mute", 0);
    GPIO_SET(GPIO_PO_PWR_EN,  "Analog power", 0); 
    return ret;
}

static int snd_idriver_remove(struct platform_device *pdev)
{
    snd_soc_unregister_card(&idriver);
    platform_set_drvdata(pdev, NULL);
    return 0;
}

static struct platform_driver snd_idriver_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "ingenic-idriver",
        .pm = &snd_soc_pm_ops,
    },
    .probe = snd_idriver_probe,
    .remove = snd_idriver_remove,
};
module_platform_driver(snd_idriver_driver);

MODULE_AUTHOR("sccheng<shicheng.cheng@ingenic.com>");
MODULE_DESCRIPTION("ALSA SoC idriver Snd Card");
MODULE_LICENSE("GPL");
