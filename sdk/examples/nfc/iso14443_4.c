/**
 * =============================================================================
 *
 *
 * File   : iso14443_4.c
 *
 * Author : Lihongxu
 *
 * Date   : 2014-12-03
 *
 * Description:
 *      implement the half duplex communication protocol with ISO14443-4
 *
 * History:
 *
 * =============================================================================
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "iso14443_4.h"
#include "sl2523.h"

#define NFC_DEBUG 0
#define DEBUG_14443_4   1 //print debug info of 14443_4

#if (DEBUG_14443_4)
#define __printf(x) printf(x)
#else
#define __printf(x)
#endif


#define FWI_DEFAULT 4   //默锟较碉拷FWI


ST_PCDINFO   gtPcdModuleInfo; /*the global variable in contactless*/

void pcd_default_info()// COS_TEST
{
    memset(&gtPcdModuleInfo, 0, sizeof(gtPcdModuleInfo));
    gtPcdModuleInfo.uiFsc = 32;
    gtPcdModuleInfo.uiFwi = FWI_DEFAULT;
    gtPcdModuleInfo.uiSfgi = 0;
    gtPcdModuleInfo.ucNadEn = 0;
    gtPcdModuleInfo.ucCidEn = 0;
    gtPcdModuleInfo.ucWTXM = 1; //multi

}


/**
 * implement the half duplex communication protocol with ISO14443-4
 *
 * parameters:
 *             ptPcdInfo  : PCD information structure pointer
 *             pucSrc     : the datas information will be transmitted by ptPcdInfo
 *             iTxNum     : the number of transmitted datas by ptPcdInfo
 *             pucDes     : the datas information will be transmitted by PICC
 *             piRxN      : the number of transmitted datas by PICC.
 * retval:
 *            0 - successfully
 *            others, error.
 */
int ISO14443_4_HalfDuplexExchange( struct ST_PCDINFO *ptPcdInfo,
                                   u8 *pucSrc,
                                   u16 iTxN,
                                   u8 *pucDes,
                                   u16 *piRxN )
{
    int status = 0;
    int            iTxNum= 0; /*the length of transmitted datas*/
    int            iTxLmt= 0; /*the maxium I-block length*/
    int            iTxCn = 0; /*the last transmitted I block length*/
    //unsigned char  aucTxBuf[256];/*{PCB} + [NAD] + [CID] + [INF] + {CRC16}*/
    unsigned char* aucTxBuf = mf_com_data.mf_data;
    //unsigned char   aucRxBuf[256];/*{PCB} + [NAD] + [CID] + [INF] + {CRC16}*/
    unsigned char* aucRxBuf = mf_com_data.mf_data;
    unsigned char* pucTxBuf  = aucTxBuf;
    unsigned char* pucRxBuf  = aucRxBuf;

    int            iSRetry   = 0; /*continuous s block count*/
    int            iIRetry   = 0; /*i block re-transmission count*/
    int            iErrRetry = 0; /*error cause re-transmission count*/

    unsigned int   uiWtx     = 1;/*the extra wait time in communication*/

    int s_swt_limit_count; /*after the special counts(user control), pcd responds timeout,if the card always sends s-block
                               add by nt for paywave test  2013/03/11 */

    enum EXC_STEP  eExStep; /*excuting stage*/

    transceive_buffer  *pi;
    pi = &mf_com_data;

    /*the first step is ready for transmission I block <$10.3.4.1>*/
    eExStep   = ORG_IBLOCK;
    *piRxN    = 0;
    uiWtx     = 1;
    iSRetry   = 0;
    iIRetry   = 0;
    iErrRetry = 0;

    /*if supported the CID or NAD*/
    if( ptPcdInfo->uiFsc < 16 )ptPcdInfo->uiFsc = 32;
    iTxLmt = ptPcdInfo->uiFsc - 3;/*PCB and CRC16*/

    if( ptPcdInfo->ucNadEn )iTxLmt--;
    if( ptPcdInfo->ucCidEn )iTxLmt--;

    /*the number of transmitted data bytes*/
    iTxNum = iTxN;
    s_swt_limit_count=0;/* add by nt for paywave test 2013/03/11 */
    /*protocol process*/
    do
    {
        switch( eExStep )
        {
            case ORG_IBLOCK:/*I block*/
#if(NFC_DEBUG)
                __printf( "ORG_IBLOCK\r\n" );
#endif
                /*point to block buffer header*/
                pucTxBuf  = aucTxBuf;
           

                /*ready for I block*/
                if( iTxNum > iTxLmt )/*chaninning block*/
                {
                    aucTxBuf[0] = 0x12 | ( ptPcdInfo->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN );/*serial number and chaninning flag*/
                    if( ptPcdInfo->ucCidEn )
                        aucTxBuf[0] |= ISO14443_CL_PROTOCOL_CID;/*support CID*/

                    if( ptPcdInfo->ucNadEn )
                        aucTxBuf[0] |= ISO14443_CL_PROTOCOL_NAD;/*support NAD*/

                    pucTxBuf++;

                    if( ptPcdInfo->ucCidEn )
                        *pucTxBuf++ = ptPcdInfo->ucCid & 0x0F;

                    if( ptPcdInfo->ucNadEn )
                        *pucTxBuf++ = ptPcdInfo->ucNad & 0x0F;

                    iTxCn    = iTxLmt;
                    iTxNum  -= iTxLmt;
                }
                else /*non-chaninning block*/
                {
                    aucTxBuf[0] = 0x02 | ( ptPcdInfo->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN );/*serial number and non-chaninning flag*/
                    if( ptPcdInfo->ucCidEn )
                        aucTxBuf[0] |= ISO14443_CL_PROTOCOL_CID;/*support CID*/

                    if( ptPcdInfo->ucNadEn )
                        aucTxBuf[0] |= ISO14443_CL_PROTOCOL_NAD;/*support NAD*/

                    pucTxBuf++;

                    if( ptPcdInfo->ucCidEn )
                        *pucTxBuf++ = ( ptPcdInfo->ucCid & 0x0F );

                    if( ptPcdInfo->ucNadEn )
                        *pucTxBuf++ = ( ptPcdInfo->ucNad & 0x0F );

                    iTxCn   = iTxNum;
                    iTxNum  = 0;
                }
                memcpy( pucTxBuf, pucSrc, iTxCn );/*the INF of I block*/
                pucTxBuf += iTxCn;
                pucSrc   += iTxCn;/*the pointer of datas*/
         
               ptPcdInfo->ucPcdPcb = aucTxBuf[0]; //by xlwang
              //   ptPcdInfo->ucPcdPcb = 0x0b;
                eExStep = ORG_TRARCV;
                break;
            case ORG_ACKBLOCK:/*R(ACK) response*/
#if(NFC_DEBUG)
                __printf( "ORG_ACKBLOCK\r\n" );
#endif
                pucTxBuf  = aucTxBuf;
                aucTxBuf[0] = 0xA2 | ( ptPcdInfo->ucPcdPcb & 1 );
                if( ptPcdInfo->ucCidEn )aucTxBuf[0] |= ISO14443_CL_PROTOCOL_CID;/*support CID*/
                pucTxBuf++;
                if( ptPcdInfo->ucCidEn )*pucTxBuf++ = ( ptPcdInfo->ucCid & 0x0F );
                eExStep = ORG_TRARCV;
                break;
            case ORG_NACKBLOCK:/*R(NACK) reponse*/
                __printf( "ORG_NACKBLOCK\r\n" );
                pucTxBuf  = aucTxBuf;
                aucTxBuf[0] = 0xB2 | ( ptPcdInfo->ucPcdPcb & 1 );
                if( ptPcdInfo->ucCidEn )aucTxBuf[0] |= ISO14443_CL_PROTOCOL_CID;/*support CID*/
                pucTxBuf++;
                if( ptPcdInfo->ucCidEn )*pucTxBuf++ = ( ptPcdInfo->ucCid & 0x0F );
                eExStep = ORG_TRARCV;
                break;
            case ORG_SBLOCK:/*S(uiWtx) block response*/
#if(NFC_DEBUG)
                __printf( "ORG_SBLOCK\r\n" );
#endif
                pucTxBuf  = aucTxBuf;
                *pucTxBuf++ = 0xF2;
                *pucTxBuf++ = uiWtx & 0x3F;
                eExStep = ORG_TRARCV;
                break;
            case ORG_TRARCV:
#if(NFC_DEBUG)
                __printf( "ORG_TRARCV\r\n" );
#endif
                /*statistical transmission error count*/
                iErrRetry++;

                /*because if S-uiWtx requested by PICC, must be response with the same value
                 *process the case with uiWtx > 59 here.
                 */
                // if( uiWtx > 59 )uiWtx = 59;
                if( uiWtx > 59 )
                {
                    uiWtx = 1;/* EMV CL TB417.6 */
                    eExStep = NON_EVENT;
                    status = ISO14443_4_ERR_PROTOCOL;
                    break;
                }

                //ptPcdInfo->uiFwi = uiWtx * ptPcdInfo->uiFwi;
                gtPcdModuleInfo.ucWTXM = uiWtx;


                pcd_delay_sfgi(ptPcdInfo->uiSfgi);
                pcd_set_tmo(ptPcdInfo->uiFwi);


                /*transmitted datas and configurating the timeout parameters*/
                pi->mf_length = pucTxBuf - aucTxBuf;
                memcpy(pi->mf_data, aucTxBuf, pi->mf_length);
                pi->mf_command = PCD_TRANSCEIVE;
                status = pcd_com_transceive(pi);
                if( ( status == MI_OK ) || ( status == 0x9a ) )
                {
                ptPcdInfo->uiPcdTxRNum = pi->mf_length / 8 + !!(pi->mf_length % 8);
                memcpy(aucRxBuf, pi->mf_data, ptPcdInfo->uiPcdTxRNum);
                ptPcdInfo->uiPcdTxRLastBits = pi->mf_length % 8;
                    status = MI_OK;
                }        


                if (status != MI_OK)
                {

                    if( iErrRetry > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED ||
                        iSRetry > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED )
                    {
                        eExStep = RCV_INVBLOCK;/*consider as protocol error*/
                    }
                    else
                    {
                        if( ptPcdInfo->ucPiccPcb & ISO14443_CL_PROTOCOL_CHAINED )/*<$10.3.5.6> & <$10.3.5.8>*/
                        {
                            eExStep = ORG_ACKBLOCK;
                        }
                        else /*<$10.3.5.3> & <$10.3.5.5>*/
                        {
                            eExStep = ORG_NACKBLOCK;
                        }
                    }
                }
                else
                {
                    iErrRetry = 0;

                    //if( !iRev )/*transmission without error*/
                    if (status == MI_OK)
                    {
                        /*<$10.3.5.4 & $10.3.5.7>*/
                        pucRxBuf = aucRxBuf;

                        /*class the block type to process*/
                        if( 0x02 == ( aucRxBuf[0] & 0xE2 ) )
                        {
                            if( ( 0 == ( aucRxBuf[0] & 0x2 ) ) || ( ptPcdInfo->uiPcdTxRNum > 254 ) ||
                                ( ( ISO14443_CL_PROTOCOL_CID | ISO14443_CL_PROTOCOL_NAD ) & aucRxBuf[0] )
                              )
                            {
                                /*EMV CL requirements <$10.3.5.4 & $10.3.5.7>,ptPcdInfo's ifsd=256*/
                                // __printf( "invblock1\r\n" );
                                eExStep = RCV_INVBLOCK;
                            }
                            else
                            {
                                iSRetry = 0;
                                eExStep  = RCV_IBLOCK;
                            }
                        }
                        else if( 0xA0 == ( aucRxBuf[0] & 0xE0 ) )
                        {
                            /*EMV CL requirements <$10.3.5.4 & $10.3.5.7>*/
                            if( ( ( ISO14443_CL_PROTOCOL_CID | ISO14443_CL_PROTOCOL_NAD ) & aucRxBuf[0] ) ||
                                ( ptPcdInfo->uiPcdTxRNum > 2 )
                              )/*EMV CL requirements <$10.3.5.4 & $10.3.5.7>*/
                            {
                                //  __printf( "invblock2\r\n" );
                                eExStep = RCV_INVBLOCK;
                            }
                            else
                            {
                                iSRetry = 0;
                                eExStep  = RCV_RBLOCK;
                            }
                        }
                        else if( 0xC0 == ( aucRxBuf[0] & 0xC0 ) )
                        {
                            /*EMV CL requirements <$10.3.5.4 & $10.3.5.7>*/
                            if( ( ( ISO14443_CL_PROTOCOL_CID | ISO14443_CL_PROTOCOL_NAD ) & aucRxBuf[0] )||
                                ( ptPcdInfo->uiPcdTxRNum > 2 )
                              )/*EMV CL requirements <$10.3.5.4 & $10.3.5.7>*/
                            {
                                //   __printf( "invblock3\r\n" );
                                eExStep = RCV_INVBLOCK;
                            }
                            else
                            {
                                iSRetry++;
                                eExStep = RCV_SBLOCK;
                            }
                        }
                        else
                        {
                            //__printf( "invblock4\r\n" );
                            eExStep = RCV_INVBLOCK;/*protocol error*/
                        }
                    }
                    else
                    {
                        /*PICC enter auto-recovery step*/
                    }
                }

                //ptPcdInfo->uiFwi /= uiWtx;
                uiWtx       = 1;/*restore the FWT*/
                gtPcdModuleInfo.ucWTXM = uiWtx;
                break;
            case RCV_IBLOCK:
#if(NFC_DEBUG)
                __printf( "RCV_IBLOCK\r\n" );
#endif
                if( iTxNum )/*command datas has be not completed*/
                {
                    //__printf( "invblock5\r\n" );
                    eExStep = RCV_INVBLOCK;
                }
                else
                {
                    pucRxBuf++;/*jump over PCB*/

                    /*the serail number equal to ptPcdInfo's current serial number*/
                    if( ( ptPcdInfo->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN ) ==
                        ( aucRxBuf[0] & ISO14443_CL_PROTOCOL_ISN ) )
                    {
                        ptPcdInfo->ucPiccPcb = aucRxBuf[0];/*the last receipt of a block*/

                        if( aucRxBuf[0] & ISO14443_CL_PROTOCOL_CHAINED )/*chainning block*/
                        {
                            if( aucRxBuf[0] & ISO14443_CL_PROTOCOL_CID )pucRxBuf++;/*jump over CID*/
                            if( aucRxBuf[0] & ISO14443_CL_PROTOCOL_NAD )pucRxBuf++;/*jump over NAD*/

                            /*R(ACK)*/
                            eExStep = ORG_ACKBLOCK;
                        }
                        else/*non-chaninning block*/
                        {
                            if( aucRxBuf[0] & ISO14443_CL_PROTOCOL_CID )pucRxBuf++;/*jump over CID*/
                            if( aucRxBuf[0] & ISO14443_CL_PROTOCOL_NAD )pucRxBuf++;/*jump over NAD*/

                            eExStep = NON_EVENT;
                        }

                        /*process received datas informations*/
                        if( ptPcdInfo->uiPcdTxRNum >= ( pucRxBuf - aucRxBuf ) )
                        {
                            memcpy( pucDes, pucRxBuf, ( ptPcdInfo->uiPcdTxRNum - ( pucRxBuf - aucRxBuf ) ) );
                            pucDes  += ptPcdInfo->uiPcdTxRNum - ( pucRxBuf - aucRxBuf );
                            *piRxN  += ptPcdInfo->uiPcdTxRNum - ( pucRxBuf - aucRxBuf );
                        }
                        /*toggle serial number <$10.3.3.3>*/
                        ptPcdInfo->ucPcdPcb ^= ISO14443_CL_PROTOCOL_ISN;
                    }
                    else /*the serail number don't equal to ptPcdInfo's current serial number*/
                    {
                        //__printf( "invblock6\r\n" );
                        eExStep = RCV_INVBLOCK;
                    }
                }
                break;
            case RCV_RBLOCK:
#if(NFC_DEBUG)
                __printf( "RCV_RBLOCK\r\n" );
#endif
                if( aucRxBuf[0] & 0x10 )/*R(NAK) <$10.3.4.6>*/
                {
                    //__printf( "invblock7\r\n" );
                    eExStep = RCV_INVBLOCK;
                }
                else
                {
                    /*the serail number equal to ptPcdInfo's current serial number*/
                    if( ( ptPcdInfo->ucPcdPcb & ISO14443_CL_PROTOCOL_ISN ) ==
                        ( aucRxBuf[0] & ISO14443_CL_PROTOCOL_ISN ) )
                    {
                        /*receiving R(ACK), send the next i block <$10.3.4.5>*/
                        if( ptPcdInfo->ucPcdPcb & ISO14443_CL_PROTOCOL_CHAINED )
                        {
                            /*toggle serial number <$10.3.3.3>*/
                            ptPcdInfo->ucPcdPcb ^= ISO14443_CL_PROTOCOL_ISN;

                            iIRetry = 0;
                            eExStep = ORG_IBLOCK;
                        }
                        else
                        {
                            //__printf( "invblock8\r\n" );
                            eExStep = RCV_INVBLOCK;
                        }
                    }
                    else /*re-transmitted last i block <$10.3.4.4>*/
                    {
                        iIRetry++;

                        if( iIRetry > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED )
                        {
                            //__printf( "invblock9\r\n" );
                            eExStep = RCV_INVBLOCK;/*protocol error*/
                        }
                        else
                        {
                            iTxNum += iTxCn;
                            pucSrc -= iTxCn;
                            eExStep = ORG_IBLOCK;
                        }
                    }
                }
                break;
            case RCV_SBLOCK:/*S(uiWtx) request*/
#if(NFC_DEBUG)
                __printf( "RCV_SBLOCK\r\n" );
#endif
                if( 0xF2 != ( aucRxBuf[0] & 0xF7 ) )/*receiving the S(uiWtx)*/
                {
                    //__printf( "invblock10\r\n" );
                    eExStep = RCV_INVBLOCK;
                }
                else
                {
                    pucRxBuf = aucRxBuf + 1;
                    if( aucRxBuf[0] & ISO14443_CL_PROTOCOL_CID )pucRxBuf++;/*jump over CID*/
                    if( 0 == ( *pucRxBuf & 0x3F ) )
                    {
                        //__printf( "invblock11\r\n" );
                        eExStep = RCV_INVBLOCK;
                    }
                    else
                    {
                        s_swt_limit_count++; /*add by nt 20121224 for zhoujie paypass 3.0*/
                        uiWtx = ( *pucRxBuf & 0x3F );/*response S(uiWtx) using the same value with PICC*/
                        eExStep = ORG_SBLOCK;
                    }
                }
                break;
            case RCV_INVBLOCK:/*protocol error*/
#if(NFC_DEBUG)
                __printf( "RCV_INVBLOCK\r\n" );
#endif
                //if( ISO14443_HW_ERR_COMM_TIMEOUT != iRev && iRev != ISO14443_PCD_ERR_USER_CANCEL)/* modify by nt 2013/03/11 */
                //  iRev = ISO14443_4_ERR_PROTOCOL;
                if (MI_NOTAGERR != status)
                {
                    status = ISO14443_4_ERR_PROTOCOL;
                }
                eExStep = NON_EVENT;/*ready to return*/

                //PcdCarrierOff( ptPcdInfo );/*Liubo 2011-11-30*/
                //PcdCarrierOn( ptPcdInfo );

                break;
            default:
                break;
        }
    }
    while( NON_EVENT != eExStep );

    //return iRev;
    return status;
}

//////////////////////////////////////////////////////////////////////
//ISO14443 DESELECT
//////////////////////////////////////////////////////////////////////
char iso14443_4_deselect(u8 CID)
{
    char status;

    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("DESELECT:\n");
#endif
    pcd_set_tmo(4);
    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = 0xca;
    mf_com_data.mf_data[1] = CID & 0x0f;
    status = pcd_com_transceive(pi);
    return status;
}




/**
 * send 'DESELECT' command to PICC
 *
 * param:
 *       ptPcdInfo  :  PCD information structure pointer
 *       ucSPcb     :  S-block PCB
 *                       0xC2 - DESELECT( no parameter )
 *                       0xF2 - WTX( one byte wtx parameter )
 *       ucParam    :  S-block parameters
 * reval:
 *       0 - successfully
 *       others, failure
 */
/*
int ISO14443_4_SRequest( struct ST_PCDINFO *ptPcdInfo,
                        unsigned char      ucSPcb,
                        unsigned char      ucParam )
{
   unsigned char  aucTxBuf[5];
   unsigned char *pucTxBuf;
   unsigned char  aucRxBuf[5];

   int            ieReTransCount = 0;

   int            iRev = 0;

   pucTxBuf = aucTxBuf;
   *pucTxBuf++ = ucSPcb;
   if( 0xF2 == ucSPcb )*pucTxBuf++ = ucParam;

   do
   {
       ieReTransCount++;
       iRev = PcdTransCeive( ptPcdInfo,
                             aucTxBuf,
                             ( pucTxBuf - aucTxBuf ),
                             aucRxBuf,
                             5 );
       if( ISO14443_HW_ERR_COMM_TIMEOUT == iRev  )
       {
           if( ieReTransCount > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED )
           {
               break;
           }
       }
       else
       {
           if( iRev )
           {
               if( ieReTransCount > ISO14443_PROTOCOL_RETRANSMISSION_LIMITED )
               {
                   iRev  = ISO14443_4_ERR_PROTOCOL;
                   break;
               }
           }
       }
   }while( iRev );

   return iRev;
}
*/

