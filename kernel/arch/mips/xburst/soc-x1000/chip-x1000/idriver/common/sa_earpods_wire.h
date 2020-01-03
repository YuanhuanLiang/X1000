/*******************************************************************************
	Copyright SmartAction Tech. 2015.
	All Rights Reserved.
	
	File: earpods_wire.h

	Description:

	TIME LIST:
	CREATE By Ringsd   2015/12/04 15:07:46

*******************************************************************************/

#ifndef _sa_earpods_wire_h_
#define _sa_earpods_wire_h_

#ifdef __cplusplus
extern "C" {
#endif

struct earpods_wire_platform_data {
    char *desc;
    int  gpio;
    int  active_low;
};


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
	END OF FILE
*******************************************************************************/
