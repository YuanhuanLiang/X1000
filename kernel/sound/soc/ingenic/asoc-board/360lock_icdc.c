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
#include <linux/switch.h>
#include <linux/delay.h>
#include <mach/jzsnd.h>
#include <board.h>
#include <linux/mutex.h>
#include "../icodec/icdc_d3.h"

static struct class *lock_360_snd_class;
static struct device *lock_360_snd_device;

static unsigned int speaker_control_flag = 3;
static unsigned int speaker_power_flag = 0;
static DEFINE_MUTEX(speaker_control_mutex);
static struct snd_codec_data *codec_platform_data = NULL;

static int lock_360_spk_power(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
    mutex_lock(&speaker_control_mutex);
    if (SND_SOC_DAPM_EVENT_ON(event)) {
        switch(speaker_control_flag) {
            case 0:
                break;
            case 1:
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
                break;
            case 2:
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
                break;
            case 3:
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
                break;
        }
        speaker_power_flag = 1;
    } else {
            gpio_direction_output(codec_platform_data->gpio_spk_en.gpio,
                    !(codec_platform_data->gpio_spk_en.active_level));
            gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio,
                    !(codec_platform_data->gpio_amp_pwr.active_level));
            speaker_power_flag = 0;
    }
    mutex_unlock(&speaker_control_mutex);
    return 0;
}

void lock_360_spk_sdown(struct snd_pcm_substream *sps) {

    if (sps->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        mutex_lock(&speaker_control_mutex);
        gpio_direction_output(codec_platform_data->gpio_spk_en.gpio,
                !(codec_platform_data->gpio_spk_en.active_level));
        gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio,
                !(codec_platform_data->gpio_amp_pwr.active_level));
        speaker_power_flag = 0;
        mutex_unlock(&speaker_control_mutex);
    }

    return;
}

int lock_360_spk_sup(struct snd_pcm_substream *sps) {

    if (sps->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        mutex_lock(&speaker_control_mutex);
        switch(speaker_control_flag) {
            case 0:
                break;
            case 1:
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
                break;
            case 2:
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
                break;
            case 3:
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
                break;
        }
        speaker_power_flag = 1;
        mutex_unlock(&speaker_control_mutex);
    }

    return 0;
}

int lock_360_i2s_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params) {
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    int ret;

    /*FIXME snd_soc_dai_set_pll*/
    ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_CBM_CFM);
    if (ret)
        return ret;
    ret = snd_soc_dai_set_sysclk(cpu_dai, JZ_I2S_INNER_CODEC, 24000000, SND_SOC_CLOCK_OUT);
    if (ret)
        return ret;
    return 0;
}
;

int lock_360_i2s_hw_free(struct snd_pcm_substream *substream)
{
    /*notify release pll*/
    return 0;
}

extern void board_icodec_mute(int mute);
void board_icodec_mute(int mute)
{
    if (codec_platform_data && (codec_platform_data->gpio_amp_mute.gpio) != -1) {

        if (mute) {
            gpio_direction_output(codec_platform_data->gpio_amp_mute.gpio, codec_platform_data->gpio_amp_mute.active_level);
        } else {
            gpio_direction_output(codec_platform_data->gpio_amp_mute.gpio, !(codec_platform_data->gpio_amp_mute.active_level));
        }

    }

}

static struct snd_soc_ops lock_360_i2s_ops = {
    .startup = lock_360_spk_sup,
    .shutdown = lock_360_spk_sdown,
    .hw_params = lock_360_i2s_hw_params,
    .hw_free = lock_360_i2s_hw_free,
};
static const struct snd_soc_dapm_widget lock_360_dapm_widgets[] = {
SND_SOC_DAPM_HP("Headphone Jack", NULL),
SND_SOC_DAPM_SPK("Speaker", lock_360_spk_power),
SND_SOC_DAPM_MIC("Mic Buildin", NULL),
SND_SOC_DAPM_MIC("DMic", NULL),
};

static struct snd_soc_jack lock_360_icdc_d3_hp_jack;
static struct snd_soc_jack_pin lock_360_icdc_d3_hp_jack_pins[] = {
    {
        .pin = "Headphone Jack",
        .mask = SND_JACK_HEADPHONE,
    },
};

#ifdef HAVE_HEADPHONE
static struct snd_soc_jack_gpio lock_360_icdc_d3_jack_gpio[] = {
    {
        .name = "Headphone detection",
        .report = SND_JACK_HEADPHONE,
        .debounce_time = 150,
    }
};
#endif

/* lock_360 machine audio_map */
static const struct snd_soc_dapm_route audio_map[] = {
    /* ext speaker connected to DO_LO_PWM  */
    { "Speaker", NULL, "DO_LO_PWM" },

    /* mic is connected to AIP/N1 */
    { "MICBIAS", NULL, "Mic Buildin" },
    { "DMIC", NULL, "DMic" },

};

static int lock_360_dlv_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
    struct snd_soc_codec *codec = rtd->codec;
    struct snd_soc_dapm_context *dapm = &codec->dapm;
    int err;

    err = snd_soc_dapm_new_controls(dapm, lock_360_dapm_widgets,
            ARRAY_SIZE(lock_360_dapm_widgets));
    if (err) {
        printk("lock_360 dai add controls err!!\n");
        return err;
    }
    /* Set up rx1950 specific audio path audio_mapnects */
    err = snd_soc_dapm_add_routes(dapm, audio_map,
            ARRAY_SIZE(audio_map));
    if (err) {
        printk("add lock_360 dai routes err!!\n");
        return err;
    }
    snd_soc_jack_new(codec, "Headset Jack", SND_JACK_HEADSET, &lock_360_icdc_d3_hp_jack);
    snd_soc_jack_add_pins(&lock_360_icdc_d3_hp_jack, ARRAY_SIZE(lock_360_icdc_d3_hp_jack_pins),
            lock_360_icdc_d3_hp_jack_pins);
#ifdef HAVE_HEADPHONE
    if (gpio_is_valid(DORADO_HP_DET)) {
        lock_360_icdc_d3_jack_gpio[jack].gpio = lock_360_HP_DET;
        lock_360_icdc_d3_jack_gpio[jack].invert = !lock_360_HP_DET_LEVEL;
        snd_soc_jack_add_gpios(&lock_360_icdc_d3_hp_jack, 1, lock_360_icdc_d3_jack_gpio);
    }
#else
    snd_soc_dapm_disable_pin(dapm, "Headphone Jack");
#endif

    snd_soc_dapm_enable_pin(dapm, "Speaker");
    snd_soc_dapm_enable_pin(dapm, "Mic Buildin");
    snd_soc_dapm_enable_pin(dapm, "DMic");

    snd_soc_dapm_sync(dapm);
    return 0;
}

static struct snd_soc_dai_link lock_360_dais[] = {
    {
        .name = "lock_360 ICDC",
        .stream_name = "lock_360 ICDC",
        .platform_name = "jz-asoc-aic-dma",
        .cpu_dai_name = "jz-asoc-aic-i2s",
        .init = lock_360_dlv_dai_link_init,
        .codec_dai_name = "icdc-d3-hifi",
        .codec_name = "icdc-d3",
        .ops = &lock_360_i2s_ops,
    },
#ifdef CONFIG_SND_ASOC_INGENIC_PCM
        {
            .name = "lock_360 PCMBT",
            .stream_name = "lock_360 PCMBT",
            .platform_name = "jz-asoc-pcm-dma",
            .cpu_dai_name = "jz-asoc-pcm",
            .codec_dai_name = "pcm dump dai",
            .codec_name = "pcm dump",
        },
#endif

#ifdef CONFIG_SND_ASOC_INGENIC_DMIC

#ifdef CONFIG_SND_ASOC_JZ_DMIC_MODULE
        {
            .name = "lock_360 DMIC",
            .stream_name = "lock_360 DMIC",
            .platform_name = "jz-asoc-dmic-module-dma",
            .cpu_dai_name = "jz-asoc-dmic-module",
            .codec_dai_name = "dmic dump dai",
            .codec_name = "dmic dump",
        },
#else
        {
            .name = "lock_360 DMIC",
            .stream_name = "lock_360 DMIC",
            .platform_name = "jz-asoc-dmic-dma",
            .cpu_dai_name = "jz-asoc-dmic",
            .codec_dai_name = "dmic dump dai",
            .codec_name = "dmic dump",
        },

#endif

#endif
    };

static struct snd_soc_card lock_360 = {
    .name = "lock_360",
    .owner = THIS_MODULE,
    .dai_link = lock_360_dais,
    .num_links = ARRAY_SIZE(lock_360_dais),
};

static ssize_t speaker_control_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", speaker_control_flag);
}
static ssize_t speaker_control_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret) {
        printk("speaker_control_store: invalid parameter\n");
        return ret;
    }

    if(state > 3) {
        printk("speaker_control_store: invalid parameter\n");
        return -EINVAL;
    }

    mutex_lock(&speaker_control_mutex);
    speaker_control_flag = state;
    if(speaker_power_flag) {
        switch(speaker_control_flag) {
            case 0:
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, !(codec_platform_data->gpio_amp_pwr.active_level));
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, !(codec_platform_data->gpio_spk_en.active_level));
                break;
            case 1:
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, !(codec_platform_data->gpio_amp_pwr.active_level));
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
                break;
            case 2:
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, !(codec_platform_data->gpio_spk_en.active_level));
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
                break;
            case 3:
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
                gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
                break;
        }
    }
    mutex_unlock(&speaker_control_mutex);
    return size;
}

static ssize_t help_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "speaker_control :\n 0: No output\n 1: Left channel\n 2: Right channel\n 3: Stereo\n");
}

static DEVICE_ATTR_RW(speaker_control);
static DEVICE_ATTR_RO(help);

static int snd_lock_360_probe(struct platform_device *pdev)
{
    int ret = 0;
    lock_360.dev = &pdev->dev;
    codec_platform_data = (struct snd_codec_data *) lock_360.dev->platform_data;

    ret = gpio_request(codec_platform_data->gpio_spk_en.gpio, "speaker left channel");
    if (ret) {
        pr_err("codec_platform_data gpio_spk_en is NULL\n");
        return ret;
    }
    gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, !(codec_platform_data->gpio_spk_en.active_level));

    ret = gpio_request(codec_platform_data->gpio_amp_pwr.gpio, "speaker right channel");
    if (ret) {
        pr_err("codec_platform_data gpio_amp_pwr is NULL\n");
        return ret;
    }
    gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, !(codec_platform_data->gpio_amp_pwr.active_level));

    if (codec_platform_data && codec_platform_data->gpio_amp_mute.gpio != -1) {
        ret = gpio_request(codec_platform_data->gpio_amp_mute.gpio, "Amplifier_mute");
        if (ret) {
            pr_err("codec_platform_data gpio_amp_mute is NULL\n");
            return ret;
        }
        gpio_direction_output(codec_platform_data->gpio_amp_mute.gpio, !(codec_platform_data->gpio_amp_mute.active_level));
    }


    lock_360_snd_class = class_create(THIS_MODULE, "sound_card");
    if(IS_ERR(lock_360_snd_class)) {
        printk("unable to create sound_card class!\n");
        ret = PTR_ERR(lock_360_snd_class);
        return ret;
    }

    lock_360_snd_device = device_create(lock_360_snd_class, NULL, 0, NULL, "%s", "speaker_control");
    if(IS_ERR(lock_360_snd_device)) {
        printk("unable to create speaker_control device!\n");
        ret = PTR_ERR(lock_360_snd_device);
        class_destroy(lock_360_snd_class);
        return ret;
    }

    device_create_file(lock_360_snd_device, &dev_attr_speaker_control);
    device_create_file(lock_360_snd_device, &dev_attr_help);

    ret = snd_soc_register_card(&lock_360);
    if (ret) {
        dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
        return ret;
    }


    return 0;
}

static int snd_lock_360_remove(struct platform_device *pdev)
{
    snd_soc_unregister_card(&lock_360);
    device_unregister(lock_360_snd_device);
    class_destroy(lock_360_snd_class);
    if (codec_platform_data->gpio_amp_mute.gpio != -1) {
        gpio_free(codec_platform_data->gpio_amp_mute.gpio);
    }

    if (codec_platform_data->gpio_amp_pwr.gpio != -1) {
        gpio_free(codec_platform_data->gpio_amp_pwr.gpio);
    }

    if (codec_platform_data->gpio_spk_en.gpio != -1) {
        gpio_free(codec_platform_data->gpio_spk_en.gpio);
    }

    return 0;
}

static struct platform_driver snd_lock_360_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "ingenic-alsa",
        .pm = &snd_soc_pm_ops,
    },
    .probe = snd_lock_360_probe,
    .remove = snd_lock_360_remove,
};
module_platform_driver(snd_lock_360_driver);

MODULE_AUTHOR("shuan.xin@ingenic.com");
MODULE_DESCRIPTION("ALSA SoC lock_360 Snd Card");
MODULE_LICENSE("GPL");
