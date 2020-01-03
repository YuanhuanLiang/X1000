#include <mach/jzsnd.h>
#include "sa_sound_switch.h"
#include "board_base.h"
#include <linux/gpio.h>

#if 0
#define printk_debug    printk
#else
#define printk_debug(...)
#endif

struct sa_sound_switch_item sa_sound_switch_item_list[] = {
    {
        .name = "headphone",
        .sdev_name = "headset",

        .switch_id = SA_SOUND_SWITCH_FLAG_PMIC | 1,
        .voltage_min = 0,
        .voltage_max = 3900,
        .switch_status = 1,
    },
};

struct sa_sound_switch sa_sound_switch ={
    .item_list = sa_sound_switch_item_list,
    .item_count = ARRAY_SIZE(sa_sound_switch_item_list),
};

struct platform_device sa_sound_swith_device = {
    .name = "sa_sound_switch",
    .dev ={
        .platform_data = &sa_sound_switch,
    }
};
struct platform_device snd_idriver_device = {
    .name = "ingenic-idriver",
};

struct platform_device codec_pcm5102 = {
    .name = "pcm5102",
};

