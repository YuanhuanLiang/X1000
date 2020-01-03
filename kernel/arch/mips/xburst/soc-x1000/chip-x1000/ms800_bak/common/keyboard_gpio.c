#include <linux/platform_device.h>
#include <linux/input/matrix_keypad.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include "board_base.h"


#ifdef CONFIG_KEYBOARD_GPIO
struct gpio_keys_button __attribute__((weak)) board_buttons[] = {
#ifdef GPIO_HOME_KEY
    {
        .gpio = GPIO_HOME_KEY,
        .code = KEY_HOME,
        .desc = "home key",
        .active_low = 1,
        .wakeup = 1,
        .debounce_interval = 10,
    },
#endif
#ifdef GPIO_MENU_KEY
    {
        .gpio = GPIO_MENU_KEY,
        .code = KEY_MENU,
        .desc = "menu key",
        .active_low = 1,
        .debounce_interval = 10,
    },
#endif
#ifdef GPIO_BACK_KEY
    {
        .gpio = GPIO_BACK_KEY,
        .code = KEY_BACK,
        .desc = "back key",
        .active_low = 1,
        .debounce_interval = 10,
    },
#endif
#ifdef GPIO_VOLUMEDOWN_KEY
    {
        .gpio = GPIO_VOLUMEDOWN_KEY,
        .code = KEY_VOLUMEDOWN,
        .desc = "volum down key",
        .active_low = 1,
        .debounce_interval = 10,
    },
#endif
#ifdef GPIO_VOLUMEUP_KEY
    {
        .gpio = GPIO_VOLUMEUP_KEY,
        .code = KEY_VOLUMEUP,
        .desc = "volum up key",
        .active_low = 1,
        .debounce_interval = 10,
    },
#endif
#ifdef GPIO_POWER_KEY
    {
        .gpio = GPIO_POWER_KEY,
        .code = KEY_POWER,
        .desc = "power key",
        .active_low = 1,
        .wakeup = 1,
        .gpio_pullup = 1,
        .debounce_interval = 10,
    },
#endif
#ifdef GPIO_SCAN_KEY
    {
        .gpio = GPIO_SCAN_KEY,
        .code = KEY_CAMERA,
        .desc = "scan key",
        .active_low = 1,
        .gpio_pullup = 1,
        .debounce_interval = 10,
    },
#endif
};
struct gpio_keys_button __attribute__((weak)) jz_logo_button[] = {
#ifdef GPIO_POWER_KEY
        {
            .gpio = GPIO_POWER_KEY,
            .code = KEY_POWER,
            .desc = "power key",
            .active_low = 1,
            .wakeup = 1,
            .gpio_pullup = 1,
            .debounce_interval = 10,
        },
#endif
#ifdef GPIO_CHARGE_KEY
        {
            .gpio = GPIO_CHARGE_KEY,
            .code = KEY_BATTERY,
            .desc = "charge key",
            .active_low = 1,
            .wakeup = 1,
            .gpio_pullup = 1,
            .debounce_interval = 10,
        },
#endif
#ifdef GPIO_CHARGING_LIGHT
        {
            .gpio = GPIO_CHARGING_LIGHT,
            .code = KEY_RED,
            .desc = "charge light",
            .active_low = 1,
            .wakeup = 1,
            .gpio_pullup = 1,
            .debounce_interval = 10,
        },
#endif
#ifdef GPIO_CHARGEFULL_LIGHT
        {
            .gpio = GPIO_CHARGEFULL_LIGHT,
            .code = KEY_GREEN,
            .desc = "full light",
            .active_low = 1,
            .wakeup = 1,
            .gpio_pullup = 1,
            .debounce_interval = 10,
        },
#endif


};
static struct gpio_keys_platform_data jz_logo_button_data = {
    .buttons = jz_logo_button,
    .nbuttons = ARRAY_SIZE(jz_logo_button),
};

struct platform_device jz_logo_device = {
    .name       = "jz_logo",
    .id     = -1,
    .num_resources  = 0,
    .dev        = {
    .platform_data  = &jz_logo_button_data,
     }
};

static struct gpio_keys_platform_data board_button_data = {
    .buttons = board_buttons,
    .nbuttons = ARRAY_SIZE(board_buttons),
};

struct platform_device jz_button_device = {
    .name = "gpio-keys",
    .id = -1,
    .num_resources = 0,
    .dev = {
        .platform_data = &board_button_data,
    }
};
#endif /* CONFIG_KEYBOARD_GPIO */


#ifdef CONFIG_KEYBOARD_MATRIX
static const uint32_t keys_map[] = {
    KEY(0, 0, KEY_1),
    KEY(0, 1, KEY_2),
    KEY(0, 2, KEY_3),

    KEY(1, 0, KEY_4),
    KEY(1, 1, KEY_5),
    KEY(1, 2, KEY_6),

    KEY(2, 0, KEY_7),
    KEY(2, 1, KEY_8),
    KEY(2, 2, KEY_9),

    KEY(3, 0, KEY_LEFT),
    KEY(3, 1, KEY_0),
    KEY(3, 2, KEY_RIGHT),

    KEY(4, 0, KEY_CANCEL),
    KEY(4, 1, KEY_CLEAR),
    KEY(4, 2, KEY_OK),
};

static struct matrix_keymap_data m_keymap_data = {
    .keymap = keys_map,
    .keymap_size = ARRAY_SIZE(keys_map),
};

static const unsigned int row_gpios[] = {
    MATRIX_KEYPAD_ROW0,
    MATRIX_KEYPAD_ROW1,
    MATRIX_KEYPAD_ROW2,
    MATRIX_KEYPAD_ROW3,
    MATRIX_KEYPAD_ROW4,
};

static const unsigned int col_gpios[] = {
    MATRIX_KEYPAD_COL0,
    MATRIX_KEYPAD_COL1,
    MATRIX_KEYPAD_COL2,
};

static struct matrix_keypad_platform_data matrix_keypad_pdata = {
    .keymap_data = &m_keymap_data,
    .row_gpios = row_gpios,
    .col_gpios = col_gpios,
    .num_row_gpios = ARRAY_SIZE(row_gpios),
    .num_col_gpios = ARRAY_SIZE(col_gpios),
    .col_scan_delay_us = 10,
    .debounce_ms = 50,
    .clustered_irq = 0,
    .active_low = 1,
    .wakeup = 0,
    .no_autorepeat = 1,
};

struct platform_device matrix_keypad_device = {
    .name = "matrix-keypad",
    .id = -1,
    .dev = {
        .platform_data = &matrix_keypad_pdata,
    },
};
#endif /* CONFIG_KEYBOARD_MATRIX */
