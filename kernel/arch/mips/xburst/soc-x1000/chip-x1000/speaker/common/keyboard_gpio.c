#include <linux/platform_device.h>

#include <linux/gpio_keys.h>
#include <linux/input.h>
#include "board_base.h"

struct gpio_keys_button __attribute__((weak)) board_buttons[] = {
#ifdef GPIO_F1_KEY
	{
		.gpio		= GPIO_F1_KEY,
		.code   	= KEY_F1,
		.desc		= "f1 key",
		.active_low	= ACTIVE_LOW_F1,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_F2_KEY
	{
		.gpio		= GPIO_F2_KEY,
		.code   	= KEY_F2,
		.desc		= "f2 key",
		.active_low	= ACTIVE_LOW_F2,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_F3_KEY
	{
		.gpio		= GPIO_F3_KEY,
		.code   	= KEY_F3,
		.desc		= "f3 key",
		.active_low	= ACTIVE_LOW_F3,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_F5_KEY
	{
		.gpio		= GPIO_F5_KEY,
		.code   	= KEY_F5,
		.desc		= "f5 key",
		.active_low	= ACTIVE_LOW_F5,
		.debounce_interval = 10,
	},
#endif

#ifdef GPIO_HOME_KEY
	{
		.gpio		= GPIO_HOME_KEY,
		.code   	= KEY_HOME,
		.desc		= "home key",
		.active_low	= ACTIVE_LOW_HOME,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_MENU_KEY
	{
		.gpio		= GPIO_MENU_KEY,
		.code   	= KEY_MENU,
		.desc		= "menu key",
		.active_low	= ACTIVE_LOW_MENU,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_BACK_KEY
	{
		.gpio		= GPIO_BACK_KEY,
		.code   	= KEY_BACK,
		.desc		= "back key",
		.active_low	= ACTIVE_LOW_BACK,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_VOLUMEDOWN_KEY
	{
		.gpio		= GPIO_VOLUMEDOWN_KEY,
		.code   	= KEY_VOLUMEDOWN,
		.desc		= "volum down key",
		.active_low	= ACTIVE_LOW_VOLUMEDOWN,
		.debounce_interval = 10,
	},
#endif
#ifdef GPIO_VOLUMEUP_KEY
	{
		.gpio		= GPIO_VOLUMEUP_KEY,
		.code   	= KEY_VOLUMEUP,
		.desc		= "volum up key",
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
