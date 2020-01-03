#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "Rfid_interface.h"
#include "StatusWord.h"
#include "TypeDefine.h"
#include "sl2523.h"


tag_info  g_tag_info;
statistics_t  g_statistics;
u8  g_statistic_refreshed;

u32 g_cos_loop_times = 0;
u8  g_cos_loop = FALSE;
u8  g_loop_buf[60];

unsigned char sendBuffer[200];
unsigned char recvBuffer[200];
#define NFC_DEBUG 0


void gtag_init()
{
    memset(&g_tag_info, 0, sizeof(g_tag_info));
    memset(&g_statistics, 0, sizeof(g_statistics));
}

void Rfid_Init(void)
{
//  IomInOutSet(11,0);
//  IomSetVal(11,1);
    pcd_init();
    pcd_antenna_off();
    gtag_init();
}

#if 1
// 用唤醒的方法进行训卡
char reqa_wakeup(tag_info * pinfo)
{
    char status;
    u8 sak;
    u8 buf[10];

    //g_statistics.reqa_cnt++;
    //g_statistic_refreshed=TRUE;
    pcd_default_info();
    delay_1ms(1);

    status =   pcd_request(WAKEUPA_CMD,g_tag_info.tag_type_bytes);
    //一次防冲突及选卡
    if (status == MI_OK)
    {

        ///printf("request successful,ATQA:0x%x, 0x%x \r\n",g_tag_info.tag_type_bytes[0], g_tag_info.tag_type_bytes[1]);
        g_tag_info.uid_length = UID_4;
        //make_packet(COM_PKT_CMD_CARD_TYPE, g_tag_info.tag_type_bytes, sizeof(g_tag_info.tag_type_bytes));

        status = pcd_cascaded_anticoll(PICC_ANTICOLL1, 0, &g_tag_info.serial_num[0]);
        if (status == MI_OK)
        {
            //printf("anticoll success:UID: 0x%x 0x%x 0x%x 0x%x\r\n",g_tag_info.serial_num[0],g_tag_info.serial_num[1],g_tag_info.serial_num[2],g_tag_info.serial_num[3]);
            status = pcd_cascaded_select(PICC_ANTICOLL1, &g_tag_info.serial_num[0], &sak);
            if(!status) {
                //printf("select successful: SAK: 0x%x\r\n", sak);
            }
        }
    }
    //二次防冲突及选卡
    if(status == MI_OK && (sak & BIT2))
    {
        //printf("SAK: 0x%x\r\n",sak);
        g_tag_info.uid_length = UID_7;
        status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
        if(status == MI_OK)
        {
            //printf("secend anticoll successful");
            status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
            if(!status)
                printf("secend select successful: SAK: 0x%x\r\n", sak);
        }
    }
    //回复uid
    if (status == MI_OK)
    {
        buf[0] = g_tag_info.uid_length;
        memcpy(buf+1, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0]:&g_tag_info.serial_num[1]), g_tag_info.uid_length);
        //make_packet(COM_PKT_CMD_REQA, buf, g_tag_info.uid_length + 1);
    }

    if(status == MI_OK)
    {
        memcpy(pinfo, &g_tag_info, sizeof(tag_info));
    }
    else
    {
        g_statistics.reqa_fail++;
#if(NFC_DEBUG)
        printf("reqa_fail\r\n");
#endif
        delay_1ms(8);
    }

    return status;
}

#endif

char pcd_anticoll_select(void)
{
    char status = MI_OK;
    u8 sak;
    u8 buf[10];

    //一次防冲突及选卡
    if (status == MI_OK)
    {
        g_tag_info.uid_length = UID_4;
        //make_packet(COM_PKT_CMD_CARD_TYPE, g_tag_info.tag_type_bytes, sizeof(g_tag_info.tag_type_bytes));

        status = pcd_cascaded_anticoll(PICC_ANTICOLL1, 0, &g_tag_info.serial_num[0]);
        if (status == MI_OK)
        {
            status = pcd_cascaded_select(PICC_ANTICOLL1, &g_tag_info.serial_num[0], &sak);
        }
    }
    //二次防冲突及选卡
    if(status == MI_OK && (sak & BIT2))
    {
        g_tag_info.uid_length = UID_7;
        status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
        if(status == MI_OK)
        {
            status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
        }
    }
    //回复uid
    if (status == MI_OK)
    {
        buf[0] = g_tag_info.uid_length;
        memcpy(buf+1, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0]:&g_tag_info.serial_num[1]), g_tag_info.uid_length);
        //make_packet(COM_PKT_CMD_REQA, buf, g_tag_info.uid_length + 1);
    }

    if(status == MI_OK)
    {
    }
    else
    {
        g_statistics.reqa_fail++;
#if(NFC_DEBUG)
        printf("reqa_fail\r\n");
#endif
        // delay_1ms(8);
    }

    return status;

}


char reqb_wakeup(uchar baud_speed)
{
    char  status;
    u8  i;
    u8  cnt;
    u8  ATQB[16];
    u8  req_code;
    //u8 dsi_dri; // param [1]
    S_ATTRIB attrib_val;
    //g_statistics.reqb_cnt++;
    //g_statistic_refreshed=TRUE;

    req_code = 8;//pcmd[1];

    cnt = 3;//应用中可以使用轮询N次
    while (cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if(status == (char)MI_COLLERR)    //有冲突超过一张卡
        {
            if((status = pcd_request_b(req_code, 0, 2, ATQB)) != MI_OK)
            {
                for (i = 1; i < 4; i++)
                {
                    if((status = pcd_slot_marker(i, ATQB)) == MI_OK)
                    {
                        break;
                    }
                }
                if (status == MI_OK)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else if (status == MI_OK)
        {
            //printf("rq-B ,n=%d\r\n", cnt);
            break;
        }
    }

    if (status == MI_OK)
    {
        //printf("rq-B OK\r\n");
        //typeB 106????
        switch(baud_speed)
        {
            case 0x1:
                //dsi_dri =0x0;
                attrib_val.param[1]=0x0;
                break;
            case 0x2:
                //   dsi_dri=0x5;
                attrib_val.param[1]=0x5;
                break;
            case 0x3:
                attrib_val.param[1]=0xa;
                break;
            case 0x04:
                attrib_val.param[1]=0xf;
                break;
            default:
                attrib_val.param[1]=0x0;
                break;
        }

        status = pcd_attri_b(&ATQB[1], attrib_val.param[1], ATQB[10]&0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50;//恢复默认值
            //make_packet(COM_PKT_CMD_REQB, ATQB, 12);
            //printf("ATA-B OK!!\n");
        }
    }

    if(status == MI_OK)
    {
    }
    else
    {
#if(NFC_DEBUG)
        printf("reqb_fail\n");
#endif
    }
    return status;
}



int Rfid_exchange(u8 *senddata,u16 tx_len,u8 *recdata,u16 *rx_len)
{
    return(ISO14443_4_HalfDuplexExchange(&gtPcdModuleInfo, senddata, tx_len, recdata, rx_len));
}

u16 iISO14443TransMit(u8* pbInData, u16 dwInLen, u8* pbOutData, u16* pwOutLen)
{
    u16 wRet;

    if(0 == Rfid_exchange(pbInData,dwInLen, pbOutData, pwOutLen))
    {
        wRet = *(pbOutData+*pwOutLen-2)<<8|*(pbOutData+*pwOutLen-1);//Convert(pbOutData+*pwOutLen-2);
        *pwOutLen -= 2;
    }
    else
    {

        wRet = SW_TRANS_ERROR;
        *pwOutLen = 0;
    }
    return wRet;
}




int Rfid_isoexchange(u8 cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv)
{
    int ucRet = MI_OK;
    u8 ucInData[300];
    u8 ucOutData[300];
    u16  uiSendLen = 0;
    u16   uiRecLen = 0;



    if((ApduSend == NULL) || (ApduRecv == NULL) || cid > 14 || ApduSend->Lc > 255)
    {
        return MI_ERR_PARAM;
    }
    memset(ucInData, 0x00, sizeof(ucInData));
    memset(ucOutData, 0x00, sizeof(ucOutData));

    memcpy(ucInData, (u8*)&(ApduSend->CLA), 4);
    uiSendLen = 4;

    if((ApduSend->Lc == 0) && (ApduSend->Le == 0))
    {
        ucInData[uiSendLen] = 0x00;
        uiSendLen++;
    }

    if(ApduSend->Lc)
    {
        ucInData[uiSendLen] = ApduSend->Lc;
        uiSendLen++;
        memcpy(ucInData+uiSendLen, ApduSend->DataIn, ApduSend->Lc);
        uiSendLen = uiSendLen + ApduSend->Lc;
    }

    if(ApduSend->Le)
    {
        if(ApduSend->Le == 256)
        {
            ucInData[uiSendLen] = 0x00;
        }
        else
        {
            ucInData[uiSendLen] = ApduSend->Le;
        }
        uiSendLen++;
    }

    ucRet = Rfid_exchange(ucInData,uiSendLen,ucOutData,&uiRecLen);

    if(ucRet != MI_OK)
    {
        return ucRet;
    }

    if (uiRecLen < 2)
    {
        ApduRecv->SWA = 0;
        ApduRecv->SWB = 0;
        return MI_ERR_TRANSMIT; // RET_RF_ERR_TRANSMIT
    }


    ApduRecv->LenOut = uiRecLen - 2;
    if((ApduSend->Le < ApduRecv->LenOut)&&(ApduSend->Le > 0))
        ApduRecv->LenOut = ApduSend->Le;
    memcpy(ApduRecv->DataOut, ucOutData, ApduRecv->LenOut);
    ApduRecv->SWA = ucOutData[uiRecLen - 2];
    ApduRecv->SWB = ucOutData[uiRecLen - 1];

    return MI_OK;
}



void Rfid_Open(void)
{
    // pcd_hlta();
    //Soft_Powerdown(0);
    delay_1ms(8);
    pcd_antenna_on();


}


void Rfid_Close(void)
{
    // pcd_hlta();
    pcd_antenna_off();
    pcd_sleep();
}

char Detect_ContactlessCard(void)
{
    u8 ats[15];
    char status = 1;
    tag_info info;

    if(reqa_wakeup(&info) == MI_OK)
    {
        if(pcd_rats_a(0,ats) == MI_OK)
        {
            status = 0;
        }
    }

    return status;
}


void test_write_0x09(void)
{
    int i, j;
    int value;
    for (j = 0; j < 255; j++) {
        for (i = 0; i < 10; i++) {
            write_reg(j, i);
            value = read_reg(j);
            printf("-> %0x %02x %02x\n", j, i, value);
        }
    }
}

#if 0
void test_a(u8 rate, tag_info * info)
{
    u16 i,j,testcnd;
    u16 okcnt;
    u16 cur_rate;
    uchar ret,I_block,val;
    u16 uiRecLen;
    uchar len;
    // uchar rate;
    u8 ats[30],ucRet;
    APDU_SEND apdu_s;
    APDU_RESP apdu_r;
    transceive_buffer  *pi;
    unsigned int uid;

    // test_write_0x09();

    pi = &mf_com_data;
    pcd_config('A');
    pcd_antenna_on();  //开场强
    delay_1ms(5);
    int once=1;
    while(once)
    {
        usleep(1000 * 10);//延时10ms让线程调度
        // once = 0;
        char c = reqa_wakeup(info);
        if(c == MI_OK)
        {
            memcpy(&uid, info->serial_num, 4);
            printf("reqa_wakeup OK, UID: 0x%08X\n", uid);
        #if 0
            if(pcd_rats_a(0,ats) == MI_OK)
            {
                printf("Rats_success:recv len = %d\r\n",    mf_com_data.mf_length/8);
                printf("TL:0x%x T0:0x%x TA(1):0x%x TB(1):0x%x TC(1):0x%x \r\n",ats[0],ats[1],ats[2],ats[3],ats[4]);

                pcd_set_tmo(4);
                //rate = 3;
                if( pcd_pps_rate(pi, ats, 0, rate) ==MI_OK)
                    printf("pcd_pps_rate_successful  \r\n");
                // rate ++;
                //  if(rate == 4)rate = 1;
                pcd_set_rate(rate);
                okcnt = 0;
                testcnd = 10;
                while(testcnd)
                {
                    testcnd--;
                    len =5;

                    sendBuffer[0] = 0x00;
                    sendBuffer[1] = 0x84;
                    sendBuffer[2] = 0x00;
                    sendBuffer[3] = 0x00;
                    sendBuffer[4] = 0x04;

                    if(Rfid_exchange(sendBuffer,len,recvBuffer,&uiRecLen) == MI_OK)
                    {
                        okcnt ++;
                        //   printf("block_exchange successful \r\n");
                        continue;
                    }
                    else
                    {
                        printf("block_exchange fail %d times \r\n",testcnd);
                        break;
                    }

                }

                if(rate == 1)
                    cur_rate = 106;
                else if(rate == 2)
                    cur_rate = 212;
                else if(rate == 3)
                    cur_rate = 424;
                else if(rate == 4)
                    cur_rate = 848;

                printf("## TEST 10 times total in rata %d, pass %d times ##\r\n",rate,okcnt);
                iso14443_4_deselect(0);
                delay_1ms(10);          
            }
            else
            {
                // printf("Rats_fail:");

            }
            #endif
        }
        else
        {
            // printf("Rats_fail:");
        }

    }
}
#endif


char test_auth(tag_info * info)
{
    //u8 ats[15];
    int i,j;
    uchar ret;

    //u8 val = -1;
    char status = 1;

    pcd_config('A');
    pcd_antenna_on();
    delay_1ms(5);

    if(reqa_wakeup(info) == MI_OK)
    {
        if(pcd_auth_state(PICC_AUTHENT1A,0,g_tag_info.serial_num,(u8 *)"\xff\xff\xff\xff\xff\xff") != MI_OK)
        {
            printf("pcd_auth_state_Failed\r\n");
            return 1;
        }
        else
            printf("pcd_auth_state_succed\r\n");
        for(j=0; j<4; j++)
        {
            for(i=0; i<100; i++)
            {
                sendBuffer[i] = i;//val;
            }

            ret = pcd_write(1,sendBuffer);
            if(ret != 0x00)
            {
                printf("write_block  fail erron:0x%x\r\n",ret);
                // break;
                //      continue;
            }

            ret = pcd_read(1,recvBuffer);
            if(ret != 0x00)
            {
                printf("read_block  fail erron:0x%x\r\n",ret);
                return 1;
            }

            for(i=0; i<16; i++)
            {
                if( sendBuffer[i ] != recvBuffer[i])
                    while(1);
            }

        }



    }
    iso14443_4_deselect(0);
    return status;

}

#if 0
//unsigned char block_1[22] = {0x02, 0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59, 0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00};
void test_b(uchar rate)
{
    u16 i,j,testcnd;
    uchar ret = 0,I_block,val;
    u16 uiRecLen;
    u16 okcnt;
    u16 cur_rate;
    u16 txlen;
    u8 ats[30],ucRet;
    APDU_SEND apdu_s;
    APDU_RESP apdu_r;
    transceive_buffer  *pi;
    pi = &mf_com_data;
    txlen = 0;
   // pcd_init();
    pcd_config('B');
    pcd_antenna_on();
    delay_1ms(3);
    while (1)
    {
        pcd_antenna_off();
        delay_1ms(50);
        pcd_antenna_on();
        delay_1ms(1000);
        if(reqb_wakeup(rate) == MI_OK)
        {
            //printf("reqb_wakeup_success:recv len = %d\r\n",    mf_com_data.mf_length/8);
            pcd_set_rate(rate);
            u8 card_id[32];
            get_idcard_num(card_id);
            printf("card_id:");
            for (int i = 0; i < 10; i++) {
                printf("%4d", card_id[i]);
            }
            printf("\n");
        #if 0
            okcnt = 0;
            testcnd = 10;
            while(testcnd)
            {
                testcnd--;
                txlen =5;

                sendBuffer[0] = 0x00;
                sendBuffer[1] = 0x84;
                sendBuffer[2] = 0x00;
                sendBuffer[3] = 0x00;
                sendBuffer[4] = 0x04;

                //   delay_1ms(5000);
                if(Rfid_exchange(sendBuffer,txlen,recvBuffer,&uiRecLen) == MI_OK)
                {
                    okcnt ++;
                    //   printf("block_exchange successful \r\n");
                    continue;
                }
                else
                {
                    printf("block_exchange fail status 0x%x  \r\n",ret);
                    break;
                }

            }
            iso14443_4_deselect(0);
            if(rate == 1)
                cur_rate = 106;
            else if(rate == 2)
                cur_rate = 212;
            else if(rate == 3)
                cur_rate = 424;
            else if(rate == 4)
                cur_rate = 848;

            printf("## TEST 10 times total in rata %d, pass %d times ##\r\n",rate,okcnt);
        #endif
            iso14443_4_deselect(0);
            //pcd_antenna_off();
        }
        else
        {
             //printf("type B active fail:\n");
        }
    }

}
#endif



volatile uint8_t card_A;
volatile uint8_t card_B;
#define TPA       ('A')
#define TPB       ('B')
tag_info info_tmp;

int8_t pcd_polling(void)
{
    int8_t  status;

    status = -1;
    if(!card_A) //card_A
    {
        pcd_config('A');
        pcd_antenna_on();
        delay_1ms(5);
        status = reqa_wakeup(&info_tmp);
        if(status  == MI_OK)
        {
            card_A = 1;
            printf("reques typeA successful!\r\n");
            delay_1ms(2);
        }
    }
    if(!card_B && !card_A)
    {
        pcd_config('B');
        delay_1ms(2);
        status = reqb_wakeup(1);
        if(status  == MI_OK)
        {
            card_B = 1;
            printf("reques typeB successful!\r\n");
            delay_1ms(2);
        }
    }
    return status;
}

volatile uint8_t demo_step;
volatile uint8_t g_ret;
uint8_t  ATQB[16];
int8_t pcd_active_b(uint8_t rate)
{
    int8_t  status;
    S_ATTRIB attrib_val;

    switch(rate)
    {
    case 0x1:
        //dsi_dri =0x0;
        attrib_val.param[1]=0x0;
        break;
    case 0x2:
        //   dsi_dri=0x5;
        attrib_val.param[1]=0x5;
        break;
    case 0x3:
        attrib_val.param[1]=0xa;
        break;
    case 0x04:
        attrib_val.param[1]=0xf;
        break;
    default:
        attrib_val.param[1]=0x0;
        break;
    }

    status = pcd_attri_b(&ATQB[1], attrib_val.param[1], ATQB[10]&0x0f, PICC_CID, ATQB);

    if(status == MI_OK)
    {
        ATQB[0] = 0x50;
        pcd_set_rate(rate);
    }
    else
    {
        if(rate == 1)
        {
            printf("106K,Unsupport\n");
        }
        else if(rate == 2)
        {
            printf("212K,Unsupport\n");
        }
        else if(rate == 3)
        {
            printf("424K,Unsupport\n");
        }
        else if(rate == 4)
        {
            printf("848K,Unsupport\n");
        }

#if(NFC_DEBUG)
        printf("pcd_active_b \n");
#endif
    }
    return status;
}

int8_t pcd_active(uint8_t rate)
{
    int8_t status = 0;
    uint8_t ats[30];
    transceive_buffer  *pi;
    pi = &mf_com_data;
    if(card_A) //card_A
    {
        status = pcd_rats_a(0,ats);
        if(status == MI_OK)
        {
            printf("Rats_success:recv len = %d\r\n",    mf_com_data.mf_length/8);
            printf("TL:0x%x T0:0x%x TA(1):0x%x TB(1):0x%x TC(1):0x%x \r\n",ats[0],ats[1],ats[2],ats[3],ats[4]);
        }
        pcd_set_tmo(4);
        status = pcd_pps_rate(pi, ats, 0, rate) ;
        if(status ==MI_OK)
        {
            //printf("pcd_pps_rate_successful  \r\n");
        }

    }
    else if(card_B)
    {
        status =  pcd_active_b(rate);
        if(status == MI_OK)
        {
            pcd_set_rate(rate);
        }
    }
    return status;

}



int8_t pcd_polling_card(PT_CardInfo info)
{
    int8_t  status;

    status = -1;
    if(!info->isCardA) //card_A
    {
        pcd_config('A');
        pcd_antenna_on();
        delay_1ms(5);
        status = reqa_wakeup(&info->aCardInfo);
        if(status  == MI_OK)
        {
            info->isCardA = 1;
            printf("reques typeA successful!\r\n");
            delay_1ms(2);
        }
    }
    if(!info->isCardB && !info->isCardA)
    {
        pcd_config('B');
        delay_1ms(2);
        status = reqb_wakeup(1);
        if(status  == MI_OK)
        {
            info->isCardB = 1;
            printf("reques typeB successful!\r\n");
            delay_1ms(2);
        }
    }
    return status;
}

uint8_t detech_card_ab(PT_CardInfo info)
{
    static int8_t  card_det = 0;
    uint8_t rate = 0;
    uint8_t ret;

    info->isCardA = 0;
    info->isCardB = 0;
    info->step = DETECH_START;

    while (1)
    {
        switch(info->step)
        {
        case 0:
            info->isCardA = 0;
            info->isCardB = 0;
            
            pcd_antenna_off();
            delay_1ms(3);
            pcd_set_rate(1);
            ret = pcd_polling_card(info);
            if(ret == 0)
            {
                info->step = DETECH_ACTIVE;
                rate = 1;
            }
            else
            {
                info->step = DETECH_START;
                card_det = 0;
                return card_det;
            }
            break;

        case 1:
            ret = pcd_active(rate);
            if(ret == 0)
            {
               info->step = DETECH_RET;
            }
            else
            {
                info->isCardA = 0;
                info->isCardB = 0;
                info->step = DETECH_START;
            }
            break;

        case 2:
            //Iblock_getrand(rate);
            if (info->isCardA)
            {
                uint32_t uid;
                memcpy(&uid, info->aCardInfo.serial_num, 4);
                printf("reqa_wakeup OK, UID: 0x%08X\n", uid);
                card_det = 1;
                return card_det;
            }
            if (info->isCardB)
            {
                u8 card_id[32];
                get_idcard_num(card_id);
                memcpy(info->bCardInfo.serial_num, card_id, 10);
                printf("card_id:");
                for (int i = 0; i < 10; i++) {
                    printf("%4d", card_id[i]);
                }
                printf("\n");
                 card_det = 2;
                 return card_det;
            }
            info->isCardA = 0;
            info->isCardB = 0;
            info->step = DETECH_START;
            iso14443_4_deselect(0);
            break;

        default:
            pcd_set_rate(1);
            pcd_antenna_off();
            delay_1ms(3);
            pcd_antenna_on();

            info->isCardA = 0;
            info->isCardB = 0;
            info->step = DETECH_START;
            break;
        }
    }
    return card_det;

}