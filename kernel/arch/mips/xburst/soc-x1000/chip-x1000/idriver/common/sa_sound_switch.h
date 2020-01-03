/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.
	
	File: sa_sound_switch.h

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/12/03 15:06:16

*******************************************************************************/

#ifndef _sa_sound_switch_h_
#define _sa_sound_switch_h_

#include <linux/switch.h>
#ifdef __cplusplus
extern "C" {
#endif

#define SA_SOUND_SWITCH_FLAG_PMIC					(0x80000000)
#define SA_SOUND_SWITCH_FLAG_GPIO					(0x40000000)

#define SA_SOUND_SWITCH_IS_PMIC(id)				    (id & SA_SOUND_SWITCH_FLAG_PMIC)
#define SA_SOUND_SWITCH_IS_GPIO(id)				    (id & SA_SOUND_SWITCH_FLAG_GPIO)
#define SA_SOUND_SWITCH_ID(id) 					    (id & 0x0000ffff)


struct sa_sound_switch{
    struct sa_sound_switch_item*    item_list;
    int                             item_count;
    
	struct delayed_work             switch_work;
    int                             interval;
};

struct sa_sound_switch_item{
    const char* name; //self name
    const char* sdev_name; //support for switch device name
    //
    struct switch_dev sdev;
    //for the detect 1:insert 0:remove
    int new_status;
    int old_status;
    int result; //(adc is voltage) or (gpio is low or high)
    int detect_count;
    //
    unsigned int switch_id;
    int voltage_min;
    int voltage_max;
    // for adc switch_status match the range [voltage_min, voltage_max]
    // for gpio switch_status match the gpio high level
    int switch_status;
    
    int (*init)( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item );
    void (*done)( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item );
    int (*detect)( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item );
    int (*report)( struct sa_sound_switch* psa_sound_switch, struct sa_sound_switch_item* item );
};


extern int axp_get_adc_value(int gpio_index, int *adc_value);
extern int axp_init_gpio_adc(int gpio_index);
#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
	END OF FILE
*******************************************************************************/
