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

#ifdef __cplusplus
extern "C" {
#endif

#define SA_SOUND_SWITCH_FLAG_PMIC					(0x80000000)
#define SA_SOUND_SWITCH_FLAG_GPIO					(0x40000000)

#define SA_SOUND_SWITCH_IS_PMIC(id)				    (id & SA_SOUND_SWITCH_FLAG_PMIC)
#define SA_SOUND_SWITCH_IS_GPIO(id)				    (id & SA_SOUND_SWITCH_FLAG_GPIO)
#define SA_SOUND_SWITCH_ID(id) 					    (id & 0x0000ffff)



#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
	END OF FILE
*******************************************************************************/
