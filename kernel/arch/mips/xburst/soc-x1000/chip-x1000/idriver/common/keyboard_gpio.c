#include <linux/platform_device.h>

#include <linux/gpio_keys.h>
#include <linux/input.h>
#include "board_base.h"

struct gpio_keys_button __attribute__((weak)) board_buttons[] = {
#ifdef GPIO_PLAY_KEY
	{
		.gpio		= GPIO_PLAY_KEY,
		.code   	= KEY_PLAYPAUSE,
		.desc		= "ok key",
		.type 		= EV_KEY,
		.active_low	= ACTIVE_LOW_PLAY,
		.wakeup         = 1
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_MENU_KEY
	{
		.gpio		= GPIO_MENU_KEY,
		.code   	= KEY_MENU,
		.desc		= "menu key",
		.type 		= EV_KEY,
		.active_low	= ACTIVE_LOW_MENU,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_BACK_KEY
	{
		.gpio		= GPIO_BACK_KEY,
		.code   	= KEY_BACK,
		.desc		= "back key",
		.type 		= EV_KEY,
		.active_low	= ACTIVE_LOW_BACK,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_NEXT_KEY
	{
		.gpio           = GPIO_NEXT_KEY,
		.code           = KEY_NEXTSONG,
		.desc           = "next key",
		.type 			= EV_KEY,
		.active_low     = ACTIVE_LOW_NEXT,
		.debounce_interval = 10,
	},
#endif

#ifdef GPIO_PREV_KEY
	{
		.gpio           = GPIO_PREV_KEY,
		.code           = KEY_PREVIOUSSONG,
		.desc           = "prev key",
		.type 			= EV_KEY,
		.active_low     = ACTIVE_LOW_PREV,
		.debounce_interval = 10,
	},
#endif

#ifdef GPIO_VOLUMEDOWN_KEY
	{
		.gpio		= GPIO_VOLUMEDOWN_KEY,
		.code   	= KEY_VOLUMEDOWN,
		.desc		= "volum down key",
		.type 		= EV_KEY,
		.active_low	= ACTIVE_LOW_VOLUMEDOWN,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_VOLUMEUP_KEY
	{
		.gpio		= GPIO_VOLUMEUP_KEY,
		.code   	= KEY_VOLUMEUP,
		.desc		= "volum up key",
		.type 		= EV_KEY,
		.active_low	= ACTIVE_LOW_VOLUMEUP,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_ENDCALL_KEY
	{
		.gpio           = GPIO_ENDCALL_KEY,
		.code           = KEY_POWER,
		.desc           = "end call key",
		.active_low     = ACTIVE_LOW_ENDCALL,
		.wakeup         = 1,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_POWER_KEY
	{
		.gpio		= GPIO_POWER_KEY,
		.code   	= KEY_POWER,
		.desc		= "power key",
		.type 		= EV_KEY,
		.active_low	= ACTIVE_LOW_POWER,
		.debounce_interval = 10,
	},
#endif
};

static struct gpio_keys_platform_data board_button_data = {
	.buttons	= board_buttons,
	.nbuttons	= ARRAY_SIZE(board_buttons),
};

struct platform_device jz_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
                .platform_data	= &board_button_data,
	}
};
