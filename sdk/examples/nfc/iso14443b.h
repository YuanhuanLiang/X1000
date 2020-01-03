/**
 ****************************************************************
 * @file iso14443b.h
 *
 * @brief 
 *
 * @author 
 *
 *
 ****************************************************************
 */
 
#ifndef ISO_14443B_H
#define ISO_14443B_H

#include "TypeDefine.h"
/////////////////////////////////////////////////////////////////////
//ISO14443B COMMAND
///////////////////////////////////////////////////////////////////// 
#define    ISO14443B_ANTICOLLISION                  0x05
#define    ISO14443B_ATTRIB                         0x1D
#define    ISO14443B_HLTB                           0x50

typedef struct
{
    uchar start_code;  // 1d
    uchar flag[4];   // PUDI
    uchar param[3];   //00 08|58|a8|f8   01
    uchar cid;  // 04
}S_ATTRIB;     



/////////////////////////////////////////////////////////////////////
//����ԭ��
/////////////////////////////////////////////////////////////////////
char pcd_request_b(u8 req_code, u8 AFI, u8 N, u8 *ATQB);
char pcd_slot_marker(u8 N, u8 *ATQB);
char pcd_attri_b(u8 *PUPI, u8 dsi_dri, u8 pro_type,u8 CID, u8 *answer);
char get_idcard_num(u8 *ATQB);
char pcd_halt_b(u8 *PUPI);
char select_sr(u8 *chip_id);
char read_sr176(u8 addr, u8 *readdata);
char write_sr176(u8 addr, u8 *writedata);
char get_prot_sr176(u8 lockreg);
char protect_sr176(u8 lockreg);
char completion_sr();
char pcd_change_rate_b(u8 CID, u8 *ATQB);



#endif
