#ifndef RFID_INTERFACE_H
#define RFID_INTERFACE_H

#include "sl2523.h"
#include "iso14443a.h"
#include "iso14443b.h"
#include "iso14443_4.h"
#include "mifare.h"

#define REQA_CMD      0x26
#define WAKEUPA_CMD   0x52

//Type A
#define COM_PKT_CMD_INIT_TYPEA                    0x1E
#define COM_PKT_CMD_INIT_TYPEB                    0x1F
#define COM_PKT_CMD_REQA                        0x20
#define COM_PKT_CMD_TYPEA_HALT                     0x21
#define COM_PKT_CMD_TYPEA_MF1_READ              0x22
#define COM_PKT_CMD_TYPEA_MF1_WRITE             0x23
#define COM_PKT_CMD_TYPEA_MF1_WALLET_INIT       0x24
#define COM_PKT_CMD_TYPEA_MF1_WALLET_INCREMENT  0x25
#define COM_PKT_CMD_TYPEA_MF1_WALLET_DECREMENT  0x26
#define COM_PKT_CMD_TYPEA_MF1_WALLET_READ       0x27
#define COM_PKT_CMD_TYPEA_MF0_READ              0x28
#define COM_PKT_CMD_TYPEA_MF0_WRITE             0x29
#define COM_PKT_CMD_TYPEA_RATS                    0x2A
#define COM_PKT_CMD_EXCHANGE                    0x2B
#define COM_PKT_CMD_DESELECT                    0x2C
#define COM_PKT_CMD_MULTI_EXCHANGE_TEST            0x2D
#define COM_PKT_CMD_TEST_STOP                    0x2E
//TYPE B
#define COM_PKT_CMD_REQB                        0x30
#define COM_PKT_CMD_TYPEB_HALT                  0x31
#define COM_PKT_CMD_TYPEB_SR176_READ            0x32
#define COM_PKT_CMD_TYPEB_SR176_WRITE           0x33
#define COM_PKT_CMD_SELECT_SR                    0x34
#define COM_PKT_CMD_PROTECT_SR176                0x35
#define COM_PKT_CMD_COMPLECT_SR176                0x36
#define COM_PKT_CMD_TYPEB_UID                    0x37

#define UID_4 4
#define UID_7 7

#define PICC_CID 0x00

typedef struct
{
    unsigned char       CLA;
    unsigned char       INS;
    unsigned char       P1;
    unsigned char       P2;
    unsigned char       Lc;
    unsigned char       DataIn[300];
    unsigned char       LeFlag;
    unsigned char       Le;
} APDU_SEND;

typedef struct
{
    unsigned int        LenOut;
    unsigned char       DataOut[300];
    unsigned char       SWA;
    unsigned char       SWB;
} APDU_RESP;

typedef struct tag_info
{
    u8 opt_step;
    u8 uid_length;
    u8 tag_type;
    u8 tag_type_bytes[2];
    u8 serial_num[12];
    u8 uncoded_key[6];
}tag_info;

typedef struct statistics_t
{
    u32 reqa_cnt;
    u32 reqa_fail;
    u32 reqb_cnt;
    u32 reqb_fail;
    u32 write_fail;
    u32 read_fail;
    u32 check_fail;
    u32 atqa_fail;
    u32 cos_fail;
    u32 atqa_cnt;
    u32 cos_cnt;
    u32 lpcd_cnt;
    u32 lpcd_fail;    
}statistics_t;

typedef enum {
    DETECH_START = 0,
    DETECH_ACTIVE,
    DETECH_RET,

} E_DetechStep;

typedef struct card_info {
    volatile uint8_t isCardA;
    volatile uint8_t isCardB;
    E_DetechStep step;
    tag_info aCardInfo;
    tag_info bCardInfo;

} T_CardInfo, *PT_CardInfo;

extern statistics_t  g_statistics;
extern tag_info  g_tag_info;
extern u8 g_cos_loop;
extern u32  g_cos_loop_times;
extern u8  g_loop_buf[60];
extern u8  g_statistic_refreshed;


char reqa_wakeup(tag_info * pinfo);
void rfid_init();
void Rfid_Init(void);
void rfid_operation(u8 *pcmd);
char com_reqa(u8 *pcmd);
char com_reqb(u8 *pcmd);
void statistic_print();
void com_exchange(u8 * pcmd);
void com_typea_rats(u8 * pcmd);
void Rfid_Close(void);

void test_a(u8 rate, tag_info * info);
void test_b(uchar rate);
char reqb_wakeup(uchar baud_speed);
char Detect_ContactlessCard(void);
u16 iISO14443TransMit(u8* pbInData, u16 dwInLen, u8* pbOutData, u16* pwOutLen);
char pcd_anticoll_select(void);
int Rfid_exchange(u8 *senddata,u16 tx_len,u8 *recdata,u16 *rx_len);
uint8_t nfc_demo(void);
uint8_t detech_card_ab(PT_CardInfo info);

#endif