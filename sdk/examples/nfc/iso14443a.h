/**
 ****************************************************************
 * @file iso14443a.h
 *
 * @brief 
 *
 * @author 
 *
 * 
 ****************************************************************
 */
#ifndef ISO14443A_H
#define ISO14443A_H

#include "TypeDefine.h"
#include "sl2523.h"

/**
 * DEFINES ISO14443A COMMAND
 * commands which are handled by the tag,Each tag command is written to the
 * reader IC and transfered via RF
 ****************************************************************
 */
 
#define PICC_REQIDL           0x26
#define PICC_REQALL           0x52
#define PICC_ANTICOLL1        0x93
#define PICC_ANTICOLL2        0x95
#define PICC_ANTICOLL3        0x97
#define PICC_HLTA             0x50

/*
 * FUNCTION DECLARATIONS
 ****************************************************************
 */
 
char pcd_request(u8 req_code, u8 *ptagtype);
char pcd_cascaded_anticoll(u8 select_code, u8 coll_position, u8 *psnr);
char pcd_cascaded_select(u8 select_code, u8 *psnr,u8 *psak);
char pcd_hlta();
char pcd_rats_a(u8 CID, u8 *ats);

char pcd_pps_rate(transceive_buffer *pi, u8 *ATS, u8 CID, u8 rate);
#endif
