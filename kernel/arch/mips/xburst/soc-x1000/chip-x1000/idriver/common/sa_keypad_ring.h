/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.
	
	File: sa_keypad_ring.h

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/12/31 00:17:51

*******************************************************************************/

#ifndef _sa_keypad_ring_h_
#define _sa_keypad_ring_h_

#ifdef __cplusplus
extern "C" {
#endif

struct device;

#define RING_NONE                   0
#define RING_CLOCKWISE              1
#define RING_COUNTER_CLOCKWISE      2

struct ring_keys_button {
	/* Configuration parameters */
	unsigned int code;	/* input event code (KEY_*, SW_*) */
	const char *desc;
    int direction;
	int wakeup;		/* configure the button as a wake-up source */
	bool can_disable;
};

struct ring_keys_platform_data {
	struct ring_keys_button *buttons;
	int nbuttons;
	unsigned int poll_interval;	/* polling interval in msecs -
					   for polling driver only */
	unsigned int rep:1;		/* enable input subsystem auto repeat */
    unsigned int timer_debounce;
    int         ring1_gpio;
    int         ring2_gpio;
	int (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
	const char *name;		/* input device name */
};

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
	END OF FILE
*******************************************************************************/
