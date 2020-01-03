/*
 *
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <plat/mfp.h>

#include "plat/hbc2500.h"
#include "sound/sa_sound_switch.h"
#include "sa_sound_dapm.h"


/********************************************************************************
 *    analog power manager
 *******************************************************************************/

//#include "sa_sound_dapm_common.c" 

#define printk_debug    printk
//#define printk_debug(...)

#define COMIP_POWER_AMP_EN          BIT(1)
#define COMIP_POWER_AMP_B_EN        BIT(2)
static volatile int comip_audio_power = 0;

int comip_audio_set_mute( const char* name, int mute )
{
    printk_debug( "%s %s\n", name, (mute ? "mute" : "unmute") );

	return 0;
}

static int sa_sound_dapm_mute( const char* name, int mute )
{
    comip_audio_set_mute(name, mute);
    
    return 0;
}

extern void ak4490_init( void );

static int comip_audio_power_setup(int setup_flag)
{
    struct regulator* apower;
    struct regulator* ampb_en;
    
    apower = regulator_get(NULL, "apower-en");
    ampb_en = regulator_get(NULL, "ampb-en");
    if (!apower || !ampb_en) {
        printk(KERN_ERR "%s\n", __func__ );
        return -1;
    }

    if (setup_flag & COMIP_POWER_AMP_EN) {
        regulator_enable(apower);
        printk_debug("---> setup amp_en\n");
		mdelay(1000);
		printk_debug("---> delay 1000ms\n");
        comip_audio_power |= COMIP_POWER_AMP_EN;
    }

    if (setup_flag & COMIP_POWER_AMP_B_EN) {
        regulator_enable(ampb_en);
        printk_debug("---> setup amp_b_en\n");
		mdelay(100);
		printk_debug("---> delay 100ms\n");
        comip_audio_power |= COMIP_POWER_AMP_B_EN;
    }

	//ak4490_init();
	printk_debug("---> setup ak4490_init\n");
	mdelay(500);
	printk_debug("---> delay 500ms\n");
    
    return 0;
}

static void comip_audio_power_shutdown(void)
{
    struct regulator* apower;
    struct regulator* ampb_en;
    
    apower = regulator_get(NULL, "apower-en");
    ampb_en = regulator_get(NULL, "ampb-en");
    if (!apower || !ampb_en) {
        printk(KERN_ERR "%s \n", __func__);
        return ;
    }

    if (comip_audio_power & COMIP_POWER_AMP_B_EN) {
		mdelay(1000);
		printk_debug("---> delay 1000ms\n");
        regulator_disable(ampb_en);
        printk_debug("---> shutdown ampb_en\n");
        comip_audio_power &= ~COMIP_POWER_AMP_B_EN;
    }

    if (comip_audio_power & COMIP_POWER_AMP_EN) {
		mdelay(1000);
        printk_debug("---> delay 1000ms\n");
        regulator_disable(apower);
        printk_debug("---> shutdown amp_en\n");
        comip_audio_power &= ~COMIP_POWER_AMP_EN;
    }
}

static int headphone_jack_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    printk_debug("%s widget:%s event:%s\n", __FUNCTION__, widget->name, event_to_ename(event));
    
    switch(event)
    {
        case SND_SOC_DAPM_PRE_PMD:
            comip_audio_power_shutdown();
            break;
        case SND_SOC_DAPM_POST_PMU:
            comip_audio_power_setup(COMIP_POWER_AMP_EN | COMIP_POWER_AMP_B_EN);
            break;
    }

    return 0;
}

static int line_out_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    printk_debug("%s widget:%s event:%s\n", __FUNCTION__, widget->name, event_to_ename(event));

    switch(event)
    {
        case SND_SOC_DAPM_PRE_PMD:
            comip_audio_power_shutdown();
            break;
        case SND_SOC_DAPM_POST_PMU:
            comip_audio_power_setup(COMIP_POWER_AMP_EN);
            break;
    }
    
    return 0;
}

static int lineout_amp_power_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    return 0;
}

static int headphone_amp_power_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    return 0;
}

static int dac_out_power_event(struct snd_soc_dapm_widget *widget, struct snd_kcontrol *kcontrol, int event)
{
    return 0;
}

/*******************************************************************************/

static struct platform_device *comip_snd_soc_device;

static int comip_snd_soc_hw_startup(struct snd_pcm_substream *substream)
{
	return 0;
}

static void comip_snd_soc_hw_shutdown(struct snd_pcm_substream *substream)
{
	return;
}

static int comip_snd_soc_hw_params(struct snd_pcm_substream *substream,
                                struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int ret = 0;

    /*
    hbc2500_set_spdif_mode(0);
    if(params_format(params) == SNDRV_PCM_FORMAT_DSD_32_1B)
    {
        hbc2500_set_samplerate(params_rate(params)*32);
    }
    else
    {
        hbc2500_set_samplerate(params_rate(params));
    }
    */

#if 0    
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_DSP_A |
								SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0)
		return ret;
        
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_DSP_A |
								SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;
#endif
        
	return ret;
}

static int comip_snd_soc_trigger(struct snd_pcm_substream *substream, int cmd)
{
    return 0;
}

extern int pga2311_add_controls(struct snd_soc_codec *codec);
extern int hbc2500_add_controls(struct snd_soc_codec *codec);

static int comip_snd_soc_dai_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
    
    //hbc2500_add_controls(codec);
    //pga2311_add_controls(codec);

	snd_soc_dapm_nc_pin(dapm, "Headphone Jack");
	snd_soc_dapm_nc_pin(dapm, "Line Out");

	snd_soc_dapm_sync(dapm);
    
	return 0;
}

static struct snd_soc_ops comip_snd_soc_ops = {
	.startup = comip_snd_soc_hw_startup,
	.shutdown = comip_snd_soc_hw_shutdown,
	.hw_params = comip_snd_soc_hw_params,
    .trigger = comip_snd_soc_trigger,
};

static struct snd_soc_dai_link comip_snd_soc_dai_link[] = {
	{
		.name = "comip",
		.stream_name = "comip",
		.codec_name = "ak4490",
		.codec_dai_name = "ak4490-hifi",
		.cpu_dai_name = "comip-i2s.0",
		.platform_name	= "comip-i2s.0",
		.ops = &comip_snd_soc_ops,
        .init = comip_snd_soc_dai_init,
	}
};

static struct snd_soc_card comip_snd_soc = {
	.name = "comip_snd_soc",
	.dai_link = comip_snd_soc_dai_link,
	.num_links = ARRAY_SIZE(comip_snd_soc_dai_link),
	.controls = sa_sound_platform_controls,
	.num_controls = ARRAY_SIZE(sa_sound_platform_controls),
    .dapm_widgets = sa_sound_platform_widgets,
    .num_dapm_widgets = ARRAY_SIZE(sa_sound_platform_widgets),
    .dapm_routes = sa_sound_platform_audio_map,
    .num_dapm_routes = ARRAY_SIZE(sa_sound_platform_audio_map),
};

static int comip_snd_soc_probe(struct platform_device *pdev)
{
	int ret;

	dev_info(&pdev->dev, "comip_snd_soc_probe\n");
    
	comip_snd_soc.dev = &pdev->dev;
    ret = snd_soc_register_card(&comip_snd_soc);
	if (ret) {
		dev_err(&pdev->dev, "add platform device failed\n");
	}

	return ret;
}

static int comip_snd_soc_remove(struct platform_device *pdev)
{
    snd_soc_unregister_card(&comip_snd_soc);
	return 0;
}

static struct platform_driver comip_snd_soc_driver = {
	.probe  = comip_snd_soc_probe,
	.remove = comip_snd_soc_remove,
	.driver = {
		.name = "comip_snd_soc",
		.owner = THIS_MODULE,
	},
};

static int __init comip_snd_soc_init(void)
{
	return platform_driver_register(&comip_snd_soc_driver);
}

static void __exit comip_snd_soc_exit(void)
{
	platform_driver_unregister(&comip_snd_soc_driver);
}

module_init(comip_snd_soc_init);
module_exit(comip_snd_soc_exit);

MODULE_AUTHOR("Peter Tang <tangyong@leadcoretech.com>");
MODULE_DESCRIPTION("comip sound soc driver");
MODULE_LICENSE("GPL");

