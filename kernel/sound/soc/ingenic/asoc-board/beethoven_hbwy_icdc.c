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
#include "../icodec/icdc_d3.h"

static struct snd_switch_data {
    struct switch_dev sdev;
    int irq;
    struct work_struct work;
    int gpio;
    int valid_level;
    int state;
} linein_detect;

#ifdef GPIO_LINEIN_AUX_SEL
static struct class *linein_switch_class;
static struct device *linein_switch_device;
static unsigned int linein_switch_flag;
#endif

static struct snd_codec_data *codec_platform_data = NULL;

static int beethoven_hbwy_spk_power(struct snd_soc_dapm_widget *w,
        struct snd_kcontrol *kcontrol, int event)
{
    if (SND_SOC_DAPM_EVENT_ON(event)) {
        if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
            gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
            printk("gpio speaker enable %d\n", gpio_get_value(codec_platform_data->gpio_spk_en.gpio));
        } else
            printk("set speaker enable failed. please check codec_platform_data\n");
    } else {
        if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
            gpio_direction_output(codec_platform_data->gpio_spk_en.gpio,
                    !(codec_platform_data->gpio_spk_en.active_level));
            printk("gpio speaker disable %d\n", gpio_get_value(codec_platform_data->gpio_spk_en.gpio));
        } else
            printk("set speaker disable failed. please check codec_platform_data\n");
    }
    return 0;
}

void beethoven_hbwy_spk_sdown(struct snd_pcm_substream *sps) {

    if (sps->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
            gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, !(codec_platform_data->gpio_spk_en.active_level));
        }
    }

    return;
}

int beethoven_hbwy_spk_sup(struct snd_pcm_substream *sps) {

    if (sps->stream == SNDRV_PCM_STREAM_PLAYBACK) {
        if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
            gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
        }
    }
    return 0;
}

int beethoven_hbwy_i2s_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params) {
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

int beethoven_hbwy_i2s_hw_free(struct snd_pcm_substream *substream)
{
    /*notify release pll*/
    return 0;
}
;

static struct snd_soc_ops beethoven_hbwy_i2s_ops = {
    .startup = beethoven_hbwy_spk_sup,
    .shutdown = beethoven_hbwy_spk_sdown,
    .hw_params = beethoven_hbwy_i2s_hw_params,
    .hw_free = beethoven_hbwy_i2s_hw_free,
};
static const struct snd_soc_dapm_widget beethoven_hbwy_dapm_widgets[] = {
SND_SOC_DAPM_HP("Headphone Jack", NULL),
SND_SOC_DAPM_SPK("Speaker", beethoven_hbwy_spk_power),
SND_SOC_DAPM_MIC("Mic Buildin", NULL),
SND_SOC_DAPM_MIC("DMic", NULL),
};

static struct snd_soc_jack beethoven_hbwy_icdc_d3_hp_jack;
static struct snd_soc_jack_pin beethoven_hbwy_icdc_d3_hp_jack_pins[] = {
    {
        .pin = "Headphone Jack",
        .mask = SND_JACK_HEADPHONE,
    },
};

#ifdef HAVE_HEADPHONE
static struct snd_soc_jack_gpio beethoven_hbwy_icdc_d3_jack_gpio[] = {
    {
        .name = "Headphone detection",
        .report = SND_JACK_HEADPHONE,
        .debounce_time = 150,
    }
};
#endif

/* beethoven_hbwy machine audio_map */
static const struct snd_soc_dapm_route audio_map[] = {
    /* ext speaker connected to DO_LO_PWM  */
    { "Speaker", NULL, "DO_LO_PWM" },

    /* mic is connected to AIP/N1 */
    { "MICBIAS", NULL, "Mic Buildin" },
    { "DMIC", NULL, "DMic" },

};

static int beethoven_hbwy_dlv_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
    struct snd_soc_codec *codec = rtd->codec;
    struct snd_soc_dapm_context *dapm = &codec->dapm;
    struct snd_soc_card *card = rtd->card;
    int err;
    if ((codec_platform_data) && ((codec_platform_data->gpio_spk_en.gpio) != -1)) {
        err = devm_gpio_request(card->dev, codec_platform_data->gpio_spk_en.gpio, "Speaker_en");
        if (err)
            return err;
    } else {
        pr_err("codec_platform_data gpio_spk_en is NULL\n");
        return err;
    }
    err = snd_soc_dapm_new_controls(dapm, beethoven_hbwy_dapm_widgets,
            ARRAY_SIZE(beethoven_hbwy_dapm_widgets));
    if (err) {
        printk("beethoven_hbwy dai add controls err!!\n");
        return err;
    }
    /* Set up rx1950 specific audio path audio_mapnects */
    err = snd_soc_dapm_add_routes(dapm, audio_map,
            ARRAY_SIZE(audio_map));
    if (err) {
        printk("add beethoven_hbwy dai routes err!!\n");
        return err;
    }
    snd_soc_jack_new(codec, "Headset Jack", SND_JACK_HEADSET, &beethoven_hbwy_icdc_d3_hp_jack);
    snd_soc_jack_add_pins(&beethoven_hbwy_icdc_d3_hp_jack, ARRAY_SIZE(beethoven_hbwy_icdc_d3_hp_jack_pins),
            beethoven_hbwy_icdc_d3_hp_jack_pins);
#ifdef HAVE_HEADPHONE
    if (gpio_is_valid(DORADO_HP_DET)) {
        beethoven_hbwy_icdc_d3_jack_gpio[jack].gpio = beethoven_hbwy_HP_DET;
        beethoven_hbwy_icdc_d3_jack_gpio[jack].invert = !beethoven_hbwy_HP_DET_LEVEL;
        snd_soc_jack_add_gpios(&beethoven_hbwy_icdc_d3_hp_jack, 1, beethoven_hbwy_icdc_d3_jack_gpio);
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

static struct snd_soc_dai_link beethoven_hbwy_dais[] = {
    {
        .name = "beethoven_hbwy ICDC",
        .stream_name = "beethoven_hbwy ICDC",
        .platform_name = "jz-asoc-aic-dma",
        .cpu_dai_name = "jz-asoc-aic-i2s",
        .init = beethoven_hbwy_dlv_dai_link_init,
        .codec_dai_name = "icdc-d3-hifi",
        .codec_name = "icdc-d3",
        .ops = &beethoven_hbwy_i2s_ops,
    },
#ifdef CONFIG_SND_ASOC_INGENIC_PCM
        {
            .name = "beethoven_hbwy PCMBT",
            .stream_name = "beethoven_hbwy PCMBT",
            .platform_name = "jz-asoc-pcm-dma",
            .cpu_dai_name = "jz-asoc-pcm",
            .codec_dai_name = "pcm dump dai",
            .codec_name = "pcm dump",
        },
#endif

#ifdef CONFIG_SND_ASOC_INGENIC_DMIC

#ifdef CONFIG_SND_ASOC_JZ_DMIC_MODULE
        {
            .name = "beethoven_hbwy DMIC",
            .stream_name = "beethoven_hbwy DMIC",
            .platform_name = "jz-asoc-dmic-module-dma",
            .cpu_dai_name = "jz-asoc-dmic-module",
            .codec_dai_name = "dmic dump dai",
            .codec_name = "dmic dump",
        },
#else
        {
            .name = "beethoven_hbwy DMIC",
            .stream_name = "beethoven_hbwy DMIC",
            .platform_name = "jz-asoc-dmic-dma",
            .cpu_dai_name = "jz-asoc-dmic",
            .codec_dai_name = "dmic dump dai",
            .codec_name = "dmic dump",
        },

#endif

#endif
    };

static struct snd_soc_card beethoven_hbwy = {
    .name = "beethoven_hbwy",
    .owner = THIS_MODULE,
    .dai_link = beethoven_hbwy_dais,
    .num_links = ARRAY_SIZE(beethoven_hbwy_dais),
};

static void set_switch_state(struct snd_switch_data *switch_data, int state)
{
        if (switch_data->state != state) {
                switch_set_state(&switch_data->sdev, state);
                switch_data->state = state;
        }
}

static void linein_switch_work(struct work_struct *linein_work)
{
    int i;
    int state = 0;
    int tmp_state = 0;
    int delay = 0;

    struct snd_switch_data *switch_data = container_of(linein_work, struct snd_switch_data, work);

    if (switch_data->state == 1) {
        /*
         * The event of linein plugout should check more time to avoid frequently plug action.
         * You can change the delay time(ms) according to your needs.
         */
        delay = 50;
    } else {
        /*
         * The event of linein plugin should report immediately.
         */
        delay = 50;
    }

    state = gpio_get_value(switch_data->gpio);
    for (i = 0; i < 5; i++) {
        msleep(delay);
        tmp_state = gpio_get_value(switch_data->gpio);
        if (tmp_state != state) {
            i = -1;
            state = gpio_get_value(switch_data->gpio);
            continue;
        }
    }

    if (state == 1)
        irq_set_irq_type(switch_data->irq, IRQF_TRIGGER_LOW);
    else if (state == 0)
        irq_set_irq_type(switch_data->irq, IRQF_TRIGGER_HIGH);

    enable_irq(switch_data->irq);

    if (state == (int) switch_data->valid_level) {
        state = 1;
#if 0
        gpio_direction_output(GPIO_LINEIN_AUX_SEL, GPIO_LINEIN_AUX_SEL_LEVEL);
#endif
    } else {
        state = 0;
#if 0
        gpio_direction_output(GPIO_LINEIN_AUX_SEL, !GPIO_LINEIN_AUX_SEL_LEVEL);
#endif
    }

    set_switch_state(switch_data, state);
    return;
}

static irqreturn_t linein_irq_handler(int irq, void *dev_id)
{
    struct snd_switch_data *switch_data =
            (struct snd_switch_data *) dev_id;

    disable_irq_nosync(switch_data->irq);

    schedule_work(&switch_data->work);

    return IRQ_HANDLED;
}

static ssize_t switch_linein_print_name(struct switch_dev *sdev, char *buf)
{
    return sprintf(buf, "%s.\n", sdev->name);
}

static ssize_t switch_linein_print_state(struct switch_dev *sdev, char *buf)
{
    char *state[2] = { "0", "1" };
    unsigned int state_val = switch_get_state(sdev);

    if (state_val == 1)
        return sprintf(buf, "%s\n", state[1]);
    else
        return sprintf(buf, "%s\n", state[0]);
}

#ifdef GPIO_LINEIN_AUX_SEL
static ssize_t linein_switch_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", linein_switch_flag);
}
static ssize_t linein_switch_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret)
        return ret;

    linein_switch_flag = !!state;
    gpio_direction_output(GPIO_LINEIN_AUX_SEL, linein_switch_flag ? GPIO_LINEIN_AUX_SEL_LEVEL : !GPIO_LINEIN_AUX_SEL_LEVEL);
    return size;
}

static ssize_t help_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "linein_switch : \n 0 : amic \n 1: linein\n");
}

static DEVICE_ATTR_RW(linein_switch);
static DEVICE_ATTR_RO(help);
#endif

static int snd_beethoven_hbwy_probe(struct platform_device *pdev)
{
    int ret = 0;
    beethoven_hbwy.dev = &pdev->dev;
    codec_platform_data = (struct snd_codec_data *) beethoven_hbwy.dev->platform_data;

    /* linein detect register */
    if (codec_platform_data && codec_platform_data->gpio_linein_detect.gpio != -1) {
        linein_detect.gpio = codec_platform_data->gpio_linein_detect.gpio;
        linein_detect.valid_level = codec_platform_data->gpio_linein_detect.active_level;
        if (!gpio_is_valid(linein_detect.gpio)) {
            printk("linein detect gpio invalid !!!.\n");
            ret = -EINVAL;
            goto platform_data_error;
        }

        ret = gpio_request(linein_detect.gpio, "linein_detect");
        if (ret < 0) {
            printk("gpio linein_detect request err\n");
            goto platform_data_error;
        }

        linein_detect.sdev.name = "linein";
        linein_detect.sdev.print_state = switch_linein_print_state;
        linein_detect.sdev.print_name = switch_linein_print_name;
        linein_detect.state = -1;

        ret = switch_dev_register(&linein_detect.sdev);
        if (ret < 0) {
            printk("linein switch_dev register fail.\n");
            goto switch_dev_register_error;
        }

        linein_detect.irq = gpio_to_irq(linein_detect.gpio);
        if (linein_detect.irq < 0) {
            printk("get linein_irq error.\n");
            ret = linein_detect.irq;
            goto linein_detect_request_irq_error;
        }

        INIT_WORK(&linein_detect.work, linein_switch_work);

        ret = request_irq(linein_detect.irq, linein_irq_handler,
        IRQF_TRIGGER_FALLING, "linein_detect", &linein_detect);
        if (ret < 0) {
            printk("request linein detect irq fail.\n");
            goto linein_detect_request_irq_error;
        }
        disable_irq(linein_detect.irq);

        linein_switch_work(&linein_detect.work);

    } else {
        linein_detect.gpio = -1;
        linein_detect.irq = -1;
    }

#ifdef GPIO_LINEIN_AUX_SEL
    if (GPIO_LINEIN_AUX_SEL != -1) {
        ret = gpio_request(GPIO_LINEIN_AUX_SEL, "linein_aux_sel");
        if (ret) {
            printk("get linein_aux_sel error.\n");
            goto sound_card_register_error;
        }

        gpio_direction_output(GPIO_LINEIN_AUX_SEL, !GPIO_LINEIN_AUX_SEL_LEVEL);

        linein_switch_class = class_create(THIS_MODULE, "sound_card");
        if(IS_ERR(linein_switch_class)) {
            printk("unable to create sound_card class!\n");
            ret = PTR_ERR(linein_switch_class);
            gpio_free(GPIO_LINEIN_AUX_SEL);
            goto sound_card_register_error;
        }

        linein_switch_device = device_create(linein_switch_class, NULL, 0, NULL, "%s", "linein");
        if(IS_ERR(linein_switch_device)) {
            printk("unable to create linin device!\n");
            ret = PTR_ERR(linein_switch_device);
            class_destroy(linein_switch_class);
            gpio_free(GPIO_LINEIN_AUX_SEL);
            goto sound_card_register_error;
        }

        device_create_file(linein_switch_device, &dev_attr_linein_switch);
        device_create_file(linein_switch_device, &dev_attr_help);
    }
#endif

    ret = snd_soc_register_card(&beethoven_hbwy);
    if (ret) {
        dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
        goto sound_card_register_error;
    }


    return 0;

sound_card_register_error:
    if(linein_detect.irq != -1)
        free_irq(linein_detect.irq, &linein_detect);
linein_detect_request_irq_error:
    if(linein_detect.gpio != -1)
        switch_dev_unregister(&linein_detect.sdev);
switch_dev_register_error:
    if(linein_detect.gpio != -1)
        gpio_free(linein_detect.gpio);
platform_data_error:
    return ret;
}

static int snd_beethoven_hbwy_remove(struct platform_device *pdev)
{
    snd_soc_unregister_card(&beethoven_hbwy);
#ifdef GPIO_LINEIN_AUX_SEL
    if (GPIO_LINEIN_AUX_SEL != -1) {
        device_unregister(linein_switch_device);
        class_destroy(linein_switch_class);
        gpio_free(GPIO_LINEIN_AUX_SEL);
    }
#endif
    if(linein_detect.irq != -1) {
        free_irq(linein_detect.irq, &linein_detect);
        switch_dev_unregister(&linein_detect.sdev);
        gpio_free(linein_detect.gpio);
    }
    return 0;
}

static struct platform_driver snd_beethoven_hbwy_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "ingenic-alsa",
        .pm = &snd_soc_pm_ops,
    },
    .probe = snd_beethoven_hbwy_probe,
    .remove = snd_beethoven_hbwy_remove,
};
module_platform_driver(snd_beethoven_hbwy_driver);

MODULE_AUTHOR("shuan.xin@ingenic.com");
MODULE_DESCRIPTION("ALSA SoC beethoven_hbwy Snd Card");
MODULE_LICENSE("GPL");
