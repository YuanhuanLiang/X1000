 /*
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 *  http://www.ingenic.com
 * Author: tjin <tao.jin@ingenic.com>
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
#include "../asoc-v13/asoc-aic-v13.h"

static struct snd_switch_data {
        struct switch_dev sdev;
        int irq;
        struct work_struct work;
        int gpio;
        int valid_level;
        int state;
}linein_switch;

extern unsigned char akm4951_linein_init_switch_data;
extern unsigned char akm4951_output_init_switch_data;
extern unsigned char akm4951_init_flag;
extern int akm4951_i2c_write_regs(unsigned char reg, unsigned char* data, unsigned int len);
extern int akm4951_i2c_read_reg(unsigned char reg, unsigned char* data, unsigned int len);

static struct class *cmd_snd_class;
static struct device *cmd_snd_device;
static unsigned int speaker_mute_flag = 1;
static unsigned int speaker_power_flag = 0;
static DEFINE_MUTEX(speaker_control_mutex);
static struct snd_codec_data *codec_platform_data = NULL;

unsigned long codec_sysclk = 24000000;
static int spk_en_power(struct snd_soc_dapm_widget *w,
                struct snd_kcontrol *kcontrol, int event)
{
    if (SND_SOC_DAPM_EVENT_ON(event)) {
        if (codec_platform_data && (codec_platform_data->gpio_amp_pwr.gpio) != -1) {
            gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, codec_platform_data->gpio_amp_pwr.active_level);
            printk("gpio speaker enable %d\n", gpio_get_value(codec_platform_data->gpio_amp_pwr.gpio));
        } else
            printk("set speaker enable failed. please check codec_platform_data\n");
    } else {
        if (codec_platform_data && (codec_platform_data->gpio_amp_pwr.gpio) != -1) {
            gpio_direction_output(codec_platform_data->gpio_amp_pwr.gpio, !(codec_platform_data->gpio_amp_pwr.active_level));
            printk("gpio speaker disable %d\n", gpio_get_value(codec_platform_data->gpio_amp_pwr.gpio));
        } else
            printk("set speaker disable failed. please check codec_platform_data\n");
    }
    return 0;
}

void spk_sdown(struct snd_pcm_substream *sps)
{
    mutex_lock(&speaker_control_mutex);
    speaker_power_flag = 0;
    speaker_mute_flag = 1;
    if (codec_platform_data && (codec_platform_data->gpio_spk_mute.gpio) != -1) {
        gpio_direction_output(codec_platform_data->gpio_spk_mute.gpio, codec_platform_data->gpio_spk_mute.active_level);
        mdelay(100);
    }

    if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
        gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, !(codec_platform_data->gpio_spk_en.active_level));
    }
    mutex_unlock(&speaker_control_mutex);
}

int spk_sup(struct snd_pcm_substream *sps)
{
    mutex_lock(&speaker_control_mutex);
    speaker_power_flag = 1;
    speaker_mute_flag = 0;
    if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
        gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
    }

    if (codec_platform_data && (codec_platform_data->gpio_spk_mute.gpio) != -1) {
        mdelay(100);
        gpio_direction_output(codec_platform_data->gpio_spk_mute.gpio, !(codec_platform_data->gpio_spk_mute.active_level));
    }
    mutex_unlock(&speaker_control_mutex);
    return 0;
}

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

        struct snd_switch_data *switch_data =
                container_of(linein_work, struct snd_switch_data, work);

        if(switch_data->state == 1){
        /*
         * The event of linein plugout should check more time to avoid frequently plug action.
         * You can change the delay time(ms) according to your needs.
         */
                delay = 50;
    }else{
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

    if (state == (int)switch_data->valid_level){
            state = 1;
        }else{
            state = 0;
        }

        set_switch_state(switch_data, state);
    return;
}

static irqreturn_t linein_irq_handler(int irq, void *dev_id)
{
        struct snd_switch_data *switch_data =
            (struct snd_switch_data *)dev_id;

        disable_irq_nosync(switch_data->irq);

        schedule_work(&switch_data->work);

        return IRQ_HANDLED;
}

static ssize_t switch_linein_print_name(struct switch_dev *sdev, char *buf)
{
        return sprintf(buf,"%s.\n",sdev->name);
}

static ssize_t switch_linein_print_state(struct switch_dev *sdev, char *buf)
{
        char *state[2] = {"0", "1"};
        unsigned int state_val = switch_get_state(sdev);

        if (state_val == 1)
                return sprintf(buf, "%s\n", state[1]);
        else
                return sprintf(buf, "%s\n", state[0]);
}

int i2s_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params) {
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
    int ret;

    /*FIXME snd_soc_dai_set_pll*/
    ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S|SND_SOC_DAIFMT_CBM_CFM);
    if (ret)
        return ret;
    ret = snd_soc_dai_set_sysclk(cpu_dai, JZ_I2S_EX_CODEC, 24000000, SND_SOC_CLOCK_OUT);
    if (ret)
        return ret;
    return 0;
};

int i2s_hw_free(struct snd_pcm_substream *substream)
{
    /*notify release pll*/
    return 0;
};


static struct snd_soc_ops beethoven_i2s_ops = {
    .startup = spk_sup,
    .shutdown = spk_sdown,
    .hw_params = i2s_hw_params,
    .hw_free = i2s_hw_free,
};

static const struct snd_soc_dapm_widget i2s_dapm_widgets[] = {
    SND_SOC_DAPM_HP("Headphone Jack", NULL),
    SND_SOC_DAPM_SPK("Speaker", spk_en_power),
    SND_SOC_DAPM_MIC("Mic", NULL),
};

static struct snd_soc_jack icdc_d3_hp_jack;
static struct snd_soc_jack_pin icdc_d3_hp_jack_pins[] = {
    {
        .pin = "Headphone Jack",
        .mask = SND_JACK_HEADPHONE,
    },
};

static struct snd_soc_jack_gpio icdc_d3_jack_gpio[] = {
    {
        .name = "Headphone detection",
        .report = SND_JACK_HEADPHONE,
        .debounce_time = 150,
    }
};

/* machine audio_map */
static const struct snd_soc_dapm_route i2s_audio_map[] = {
    /* ext speaker connected to DAC out */
    {"Speaker", NULL, "DAC OUT"},

    /* mic is connected to ADC in */
    {"Mic", NULL, "ADC IN"},

};

static int i2s_dai_link_init(struct snd_soc_pcm_runtime *rtd)
{
    struct snd_soc_codec *codec = rtd->codec;
    struct snd_soc_dapm_context *dapm = &codec->dapm;
    int err;

    err = snd_soc_dapm_new_controls(dapm, i2s_dapm_widgets,
            ARRAY_SIZE(i2s_dapm_widgets));
    if (err){
        printk("add dapm controls err!!\n");
        return err;
    }

    /* Set up specific audio path audio_mapnects */
    err = snd_soc_dapm_add_routes(dapm, i2s_audio_map,
            ARRAY_SIZE(i2s_audio_map));
    if (err){
        printk("add dapm routes err!!\n");
        return err;
    }

    snd_soc_jack_new(codec, "Headset Jack", SND_JACK_HEADSET, &icdc_d3_hp_jack);
    snd_soc_jack_add_pins(&icdc_d3_hp_jack,ARRAY_SIZE(icdc_d3_hp_jack_pins), icdc_d3_hp_jack_pins);
#ifdef HAVE_HEADPHONE
    if (gpio_is_valid(DORADO_HP_DET)) {
        icdc_d3_jack_gpio[jack].gpio = PHOENIX_HP_DET;
        icdc_d3_jack_gpio[jack].invert = !PHOENIX_HP_DET_LEVEL;
        snd_soc_jack_add_gpios(&icdc_d3_hp_jack, 1, icdc_d3_jack_gpio);
    }
#else
    snd_soc_dapm_disable_pin(dapm, "Headphone Jack");
#endif

    snd_soc_dapm_force_enable_pin(dapm, "Speaker");
    snd_soc_dapm_force_enable_pin(dapm, "Mic");

    snd_soc_dapm_sync(dapm);
    return 0;
}

static struct snd_soc_dai_link soc_dais[] = {
    [0] = {
        .name = "beethoven akm4951",
        .stream_name = "beethoven akm4951",
        .platform_name = "jz-asoc-aic-dma",
        .cpu_dai_name = "jz-asoc-aic-i2s",
        .init = i2s_dai_link_init,
        .codec_dai_name = "akm4951-dai",
        .codec_name = "akm4951.0-0012",
        .ops = &beethoven_i2s_ops,
    },
    [1] = {
        .name = "beethoven PCMBT",
        .stream_name = "beethoven PCMBT",
        .platform_name = "jz-asoc-pcm-dma",
        .cpu_dai_name = "jz-asoc-pcm",
        .codec_dai_name = "pcm dump dai",
        .codec_name = "pcm dump",
    },
    [2] = {
        .name = "beethoven DMIC",
        .stream_name = "beethoven DMIC",
        .platform_name = "jz-asoc-dmic-dma",
        .cpu_dai_name = "jz-asoc-dmic",
        .codec_dai_name = "dmic dump dai",
        .codec_name = "dmic dump",
    },

};

static struct snd_soc_card beethoven_card = {
    .name = "beethoven",
    .owner = THIS_MODULE,
    .dai_link = soc_dais,
    .num_links = ARRAY_SIZE(soc_dais),
};

static ssize_t mute_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%u\n", speaker_mute_flag);
}
static ssize_t mute_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t size)
{
    unsigned long state;
    ssize_t ret = -EINVAL;

    ret = kstrtoul(buf, 10, &state);
    if(ret) {
        printk("mute_store: invalid parameter\n");
        return ret;
    }

    mutex_lock(&speaker_control_mutex);
    if(speaker_power_flag) {
        if(state) {
            speaker_mute_flag = 1;

            if (codec_platform_data && (codec_platform_data->gpio_spk_mute.gpio) != -1) {
                gpio_direction_output(codec_platform_data->gpio_spk_mute.gpio, codec_platform_data->gpio_spk_mute.active_level);
                mdelay(100);
            }

            if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, !(codec_platform_data->gpio_spk_en.active_level));
            }
        } else {
            speaker_mute_flag = 0;

            if (codec_platform_data && (codec_platform_data->gpio_spk_en.gpio) != -1) {
                gpio_direction_output(codec_platform_data->gpio_spk_en.gpio, codec_platform_data->gpio_spk_en.active_level);
            }

            if (codec_platform_data && (codec_platform_data->gpio_spk_mute.gpio) != -1) {
                mdelay(100);
                gpio_direction_output(codec_platform_data->gpio_spk_mute.gpio, !(codec_platform_data->gpio_spk_mute.active_level));
            }
        }
    }

    mutex_unlock(&speaker_control_mutex);
    return size;
}

static ssize_t help_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "mute :\n 0: normal\n 1: mute\n");
}

static DEVICE_ATTR_RW(mute);
static DEVICE_ATTR_RO(help);

static int snd_beethoven_probe(struct platform_device *pdev) {
    int ret = 0;
    unsigned char data = 0x0;

    beethoven_card.dev = &pdev->dev;
    codec_platform_data = (struct snd_codec_data *)beethoven_card.dev->platform_data;

    linein_switch.sdev.name = "linein";
    linein_switch.sdev.print_state = switch_linein_print_state;
    linein_switch.sdev.print_name  = switch_linein_print_name;
    linein_switch.state = -1;

    ret = switch_dev_register(&linein_switch.sdev);
    if (ret < 0) {
        printk("linein switch dev register fail.\n");
        return ret;
    }

    /*
     *  Output channel selection
     * 2 : headphone
     *  Input channel selection
     * 3 : linein
     */
    akm4951_output_init_switch_data = 0x02;
    akm4951_linein_init_switch_data = 0x03;

    /* linein detect register */
    if (codec_platform_data && codec_platform_data->gpio_linein_detect.gpio != -1) {
        linein_switch.gpio = codec_platform_data->gpio_linein_detect.gpio;
        linein_switch.valid_level = codec_platform_data->gpio_linein_detect.active_level;
        if (!gpio_is_valid(linein_switch.gpio)){
            printk("linein detect gpio error.\n");
            return -1;
        }

        ret = gpio_request(linein_switch.gpio, "linein_detect");
        if (ret < 0) {
            printk("gpio linein_detect request err\n");
            return ret;
        }

        linein_switch.irq = gpio_to_irq(linein_switch.gpio);
        if (linein_switch.irq < 0) {
            printk("get linein_irq error.\n");
            ret = linein_switch.irq;
            return ret;
        }

        INIT_WORK(&linein_switch.work, linein_switch_work);

        ret = request_irq(linein_switch.irq, linein_irq_handler,
                      IRQF_TRIGGER_FALLING, "linein_detect", &linein_switch);
        if (ret < 0) {
            printk("request linein detect irq fail.\n");
            return ret;
        }
        disable_irq(linein_switch.irq);

        linein_switch_work(&linein_switch.work);

    } else {
        linein_switch.irq = -1;
    }

    if (codec_platform_data && codec_platform_data->gpio_amp_pwr.gpio != -1) {
        ret = gpio_request(codec_platform_data->gpio_amp_pwr.gpio, "Speaker_pwr");
        if (ret) {
            printk("get Speaker_pwr error.\n");
            return ret;
        }
    }

    if (codec_platform_data && codec_platform_data->gpio_spk_en.gpio != -1) {
        ret = gpio_request(codec_platform_data->gpio_spk_en.gpio, "Speaker_en");
        if (ret) {
            printk("get Speaker_en error.\n");
            return ret;
        }
    }

    if (codec_platform_data && codec_platform_data->gpio_spk_mute.gpio != -1) {
        ret = gpio_request(codec_platform_data->gpio_spk_mute.gpio, "Speaker_mute");
        if (ret) {
            printk("get Speaker_mute error.\n");
            return ret;
        }
    }

    cmd_snd_class = class_create(THIS_MODULE, "sound_card");
    if(IS_ERR(cmd_snd_class)) {
        printk("unable to create sound_card class!\n");
        ret = PTR_ERR(cmd_snd_class);
        return ret;
    }

    cmd_snd_device = device_create(cmd_snd_class, NULL, 0, NULL, "%s", "speaker_control");
    if(IS_ERR(cmd_snd_device)) {
        printk("unable to create speaker_control device!\n");
        ret = PTR_ERR(cmd_snd_device);
        class_destroy(cmd_snd_class);
        return ret;
    }

    device_create_file(cmd_snd_device, &dev_attr_mute);
    device_create_file(cmd_snd_device, &dev_attr_help);


    ret = snd_soc_register_card(&beethoven_card);
    if (ret) {
        dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
        return ret;
    }

    dev_info(beethoven_card.dev, "Alsa sound card init ok!!!\n");

    return ret;
}

static int snd_beethoven_remove(struct platform_device *pdev)
{
    snd_soc_unregister_card(&beethoven_card);
    device_unregister(cmd_snd_device);
    class_destroy(cmd_snd_class);


    if (codec_platform_data->gpio_amp_mute.gpio != -1) {
        gpio_free(codec_platform_data->gpio_amp_mute.gpio);
    }

    if (codec_platform_data->gpio_amp_pwr.gpio != -1) {
        gpio_free(codec_platform_data->gpio_amp_pwr.gpio);
    }

    if (codec_platform_data->gpio_spk_en.gpio != -1) {
        gpio_free(codec_platform_data->gpio_spk_en.gpio);
    }

    if (codec_platform_data && codec_platform_data->gpio_linein_detect.gpio != -1)
        cancel_work_sync(&linein_switch.work);

    switch_dev_unregister(&linein_switch.sdev);

    platform_set_drvdata(pdev, NULL);
    return 0;
}

static struct platform_driver snd_beethoven_driver = {
        .driver = {
                .owner = THIS_MODULE,
                .name = "ingenic-beethoven",
                .pm = &snd_soc_pm_ops,
        },
        .probe = snd_beethoven_probe,
        .remove = snd_beethoven_remove,
};
module_platform_driver(snd_beethoven_driver);


MODULE_AUTHOR("tjin<tao.jin@ingenic.com>");
MODULE_DESCRIPTION("ALSA SoC Beethoven Snd Card");
MODULE_LICENSE("GPL");
