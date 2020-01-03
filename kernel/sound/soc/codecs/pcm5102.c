/*******************************************************************************
	Copyright SmartAction Tech. 2016.
	All Rights Reserved.

	File: pcm5102.c

	Description:

	TIME LIST:
	CREATE By Ringsd   2016/01/12 12:21:38

*******************************************************************************/

#define TAG "pcm5102"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <sound/initval.h>

#include <gpio.h>

/* pcm5102 dapm widgets */
static const struct snd_soc_dapm_widget pcm5102_dapm_widgets[] = {
    SND_SOC_DAPM_INPUT( "pcm5102 I2S In" ),

	SND_SOC_DAPM_OUTPUT("pcm5102 LOUT"),
	SND_SOC_DAPM_OUTPUT("pcm5102 ROUT"),
};

static const struct snd_soc_dapm_route pcm5102_audio_map[] = {
	/*stereo mixer */
	{"pcm5102 LOUT", NULL, "pcm5102 I2S In"},
	{"pcm5102 ROUT", NULL, "pcm5102 I2S In"},
};

static int pcm5102_set_dai_sysclk(struct snd_soc_dai *codec_dai,
	int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int pcm5102_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	return 0;
}

static int pcm5102_set_dai_fmt(struct snd_soc_dai *dai,
		unsigned int fmt)
{
	return 0;
}

static int pcm5102_digital_mute(struct snd_soc_dai *dai, int mute)
{
	return 0;
}

static int pcm5102_set_bias_level(struct snd_soc_codec *codec,
	enum snd_soc_bias_level level)
{
	codec->dapm.bias_level = level;
	return 0;
}

#define pcm5102_RATES (SNDRV_PCM_RATE_8000_192000)

static const struct snd_soc_dai_ops pcm5102_dai_ops = {
	.hw_params	= pcm5102_hw_params,
	.set_fmt	= pcm5102_set_dai_fmt,
	.digital_mute = pcm5102_digital_mute,
	.set_sysclk	= pcm5102_set_dai_sysclk,
};

static struct snd_soc_dai_driver pcm5102_dai = {
	.name = "pcm5102-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = pcm5102_RATES,
		.formats = (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE),
    },
	.ops = &pcm5102_dai_ops,
};

static int pcm5102_suspend(struct snd_soc_codec *codec)
{
	return 0;
}

static int pcm5102_resume(struct snd_soc_codec *codec)
{
	return 0;
}

static int pcm5102_probe(struct snd_soc_codec *codec)
{			
	return 0;
}

/* power down chip */
static int pcm5102_remove(struct snd_soc_codec *codec)
{
	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_pcm5102 = {
	.probe =	pcm5102_probe,
	.remove =	pcm5102_remove,
	.suspend =	pcm5102_suspend,
	.resume =	pcm5102_resume,
	.set_bias_level = pcm5102_set_bias_level,
	.dapm_widgets = pcm5102_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(pcm5102_dapm_widgets),
	.dapm_routes = pcm5102_audio_map,
	.num_dapm_routes = ARRAY_SIZE(pcm5102_audio_map),
};

static int pcm5102_codec_probe(struct platform_device *pdev)
{
	int ret;
	ret = snd_soc_register_codec(&pdev->dev,
			&soc_codec_dev_pcm5102, &pcm5102_dai, 1);
    if( ret )
    {
        printk( "snd_soc_register_codec pcm5102 fail.\n" );
    }
    else
    {
        printk( "snd_soc_register_codec pcm5102 success.\n" );
    }
    
	return ret;
}

static int pcm5102_codec_remove(struct platform_device *pdev)
{
	snd_soc_unregister_codec(&pdev->dev);
	return 0;
}

static struct platform_driver pcm5102_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "pcm5102",
	},
	.probe = pcm5102_codec_probe,
	.remove = pcm5102_codec_remove,
};
module_platform_driver(pcm5102_driver);

MODULE_DESCRIPTION("Soc pcm5102 driver");
MODULE_AUTHOR("ringsd ");
MODULE_LICENSE("GPL");

/*******************************************************************************
    END OF FILE
*******************************************************************************/
