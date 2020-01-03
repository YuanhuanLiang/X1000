/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.
	
	File: sa_playback.h

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/7/27 16:32:49

*******************************************************************************/

#ifndef _sa_playback_h_
#define _sa_playback_h_

#ifdef __cplusplus
extern "C" {
#endif

#define LINEOUT_SELECT_LO           0
#define LINEOUT_SELECT_SPDIF        1

int sa_sound_setting_lineout_select(void);

#define GAIN_SELECT_LOW             0
#define GAIN_SELECT_MIDDLE          1
#define GAIN_SELECT_HIGH            2

#define DIGITAL_FILTER_SHARP_ROLL_OFF 			0
#define DIGITAL_FILTER_SLOW_ROLL_OFF 			1
#define DIGITAL_FILTER_SHORT_DELAY_ROLL_OFF 	2	//default
#define DIGITAL_FILTER_SHORT_DELAY_SLOW_OFF 	3
#define DIGITAL_FILTER_SUPER_ROLL_OFF			4

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
	END OF FILE
*******************************************************************************/
