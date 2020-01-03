/**
 ****************************************************************
 * @file iso14443b.c
 *
 * @brief  iso1443b protocol driver
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

//#include "spi.h"
//#include "uart.h"
#include "sl2523.h"
#include "iso14443b.h" 
 

extern u8  g_fwi;//frame waiting time integer
u8  g_fwi = 4;//frame waiting time integer

#define NFC_DEBUG 0

char pcd_request_b(u8 req_code, u8 AFI, u8 N, u8 *ATQB)
{
    char  status;
    
    transceive_buffer  *pi;
    pi=&mf_com_data;

#if (NFC_DEBUG)
    printf("REQB/WUPB:\n");
#endif

    pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length = 3;
    mf_com_data.mf_data[0] = ISO14443B_ANTICOLLISION;                // APf code
    mf_com_data.mf_data[1] = AFI;                // 
    mf_com_data.mf_data[2] = (req_code & 0x08) | (N&0x07);  // PARAM
 
    status = pcd_com_transceive(pi);

    if (status  !=MI_OK && status !=MI_NOTAGERR)
    {   
        status = MI_COLLERR;   
    }
    if (status == MI_OK && mf_com_data.mf_length != 96)
    {   
        status = MI_COM_ERR;   
    }
    if (status == MI_OK) 
    {    
        memcpy(ATQB, &mf_com_data.mf_data[0], 16);
        pcd_set_tmo(ATQB[11]>>4); // set FWT 
        g_fwi = (ATQB[11]>>4);
    }
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif
    return status;
}                      

//////////////////////////////////////////////////////////////////////
//SLOT-MARKER
//////////////////////////////////////////////////////////////////////
char pcd_slot_marker(u8 N, u8 *ATQB)
{
    char status;
    
    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("SLOT:\n");
#endif
    pcd_set_tmo(5);

    if(!N || N>15)
    {
        status = MI_WRONG_PARAMETER_VALUE;    
    }
    else
    {
        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_length = 1;
        mf_com_data.mf_data[0] = 0x05 |(N << 4); // APn code

        status = pcd_com_transceive(pi);

        if (status != MI_OK && status != MI_NOTAGERR)
        {   
            status = MI_COLLERR;   
        }
        if (status == MI_OK && mf_com_data.mf_length != 96)
        {   
            status = MI_COM_ERR;   
        }
        if (status == MI_OK) 
        {    
          memcpy(ATQB, &mf_com_data.mf_data[0], 16);
          pcd_set_tmo(ATQB[11]>>4); // set FWT 
          g_fwi = ATQB[11]>>4;
        }     
    }
    
    return status;
}                      


char pcd_attri_b(u8 *PUPI, u8 dsi_dri, u8 pro_type, u8 CID, u8 *answer)
{
    char  status;
    
    transceive_buffer  *pi;
    pi = &mf_com_data;
    pro_type = pro_type;


#if (NFC_DEBUG)
    printf("ATTRIB:\n");
#endif
    
    pcd_set_tmo(g_fwi);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 9;
    mf_com_data.mf_data[0] = ISO14443B_ATTRIB;
    memcpy(&mf_com_data.mf_data[1], PUPI, 4);
    //mf_com_data.mf_data[1] = 0x00;
    //mf_com_data.mf_data[2] = 0x00;
    //mf_com_data.mf_data[3] = 0x00;
    //mf_com_data.mf_data[4] = 0x00;
    mf_com_data.mf_data[5] = 0x00;          // EOF/SOF required, default TR0/TR1
    //EOFTEST
    //mf_com_data.mf_data[5] = 0x04; //XU //SOF not,EOF yes
    //mf_com_data.mf_data[5] = 0x08; //XU //SOF yes,EOF no
    //mf_com_data.mf_data[5] = 0x0C; //XU //SOF no,EOF no
    
    
    set_bit_mask(TypeBReg, BIT7 | BIT6); //EOF SOF required
    //write_reg(0x1E, 0x13);    //EOF SOF required
    //write_reg(0x1E, 0xD3);    //EOF SOF not required
    
    mf_com_data.mf_data[6] = ((dsi_dri << 4) | FSDI); //FSDI; // Max frame 64 
    mf_com_data.mf_data[7] = 0x01; //pro_type & 0x0f;  //;  ISO/IEC 14443-4 compliant?;
    mf_com_data.mf_data[8] = (CID & 0x0f);
 
    status  = pcd_com_transceive(pi);
    if (status == MI_OK)
    {    
        *answer = mf_com_data.mf_data[0];
    }     

    return status;
}


//////////////////////////////////////////////////////////////////////
//REQUEST B
//////////////////////////////////////////////////////////////////////
char get_idcard_num(u8 *pid)
{
    char  status;
    
    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("ID_NUM:\n");
#endif
    //pcd_set_tmo(5);
    
    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  =5;
    mf_com_data.mf_data[0] =0x00; //ISO14443B_ANTICOLLISION;                // APf code
    mf_com_data.mf_data[1] =0x36;// AFI;                // 
    mf_com_data.mf_data[2] =0x00; //((req_code<<3)&0x08) | (N&0x07);  // PARAM
    mf_com_data.mf_data[3] =0x00;
    mf_com_data.mf_data[4] =0x08;
 
    status = pcd_com_transceive(pi);

    if (status == MI_OK) 
    {    
        memcpy(pid, &mf_com_data.mf_data[0], 10);
        //pcd_set_tmo(ATQB[11]>>4); // set FWT 
    } 
    return status;
}       


char pcd_halt_b(u8 *PUPI)
{
    char  status;
    
    transceive_buffer  *pi;
    pi = &mf_com_data;
    
#if (NFC_DEBUG)
    printf("HALTB:\n");
#endif
    pcd_set_tmo(g_fwi);
                                               // disable, ISO/IEC3390 enable    
    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 5;
    mf_com_data.mf_data[0] = ISO14443B_ATTRIB;
    memcpy(&mf_com_data.mf_data[1], PUPI, 4);
    
    status = pcd_com_transceive(pi);
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif

    return status;
}   


/**
 ****************************************************************
 * @brief select_sr() 
 *
 * ����ײ����
 * @param: 
 * @param: 
 * @return: status ֵΪMI_OK:�ɹ�
 * @retval: chip_id  �õ���SR��Ƭ��chip_id
 ****************************************************************
 */
char select_sr(u8 *chip_id)
{
    char status;
    
    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("SELECT_SR:\n");
#endif    
    pcd_set_tmo(5);
    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = 0x06;                //initiate card
    mf_com_data.mf_data[1] = 0;                
    
    status = pcd_com_transceive(pi);

    if (status!=MI_OK && status!=MI_NOTAGERR) 
    {   status = MI_COLLERR;   }          // collision occurs
    
    if(mf_com_data.mf_length != 8)
    {   status = MI_COM_ERR;   }
    
    if (status == MI_OK)
    {    
         pcd_set_tmo(5);
         mf_com_data.mf_command = PCD_TRANSCEIVE;
         mf_com_data.mf_length  = 2;
         mf_com_data.mf_data[1] = mf_com_data.mf_data[0];                
         mf_com_data.mf_data[0] = 0x0E;                 // Slect card
         
         status = pcd_com_transceive(pi); 
         
         if (status!=MI_OK && status!=MI_NOTAGERR)  // collision occurs
         {   status = MI_COLLERR;   }               // collision occurs
         if (mf_com_data.mf_length != 8) 
         {   status = MI_COM_ERR;     }
         if (status == MI_OK)
         {  *chip_id = mf_com_data.mf_data[0];  }
    }     
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif    
    return status;
}  


char read_sr176(u8 addr, u8 *readdata)
{
    char status;
    
    transceive_buffer  *pi;
    pi = &mf_com_data;
    
#if (NFC_DEBUG)
    printf("read_sr176()->\n");
#endif    
    pcd_set_tmo(5);
    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = 0x08;
    mf_com_data.mf_data[1] = addr;
  
    status = pcd_com_transceive(pi);
  
    if ((status==MI_OK) && (mf_com_data.mf_length!=16))
    {   status = MI_BITCOUNTERR;    }
    if (status == MI_OK)
    {
        *readdata     = mf_com_data.mf_data[0];
        *(readdata+1) = mf_com_data.mf_data[1];
    }
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif    

    return status;  
}  

char write_sr176(u8 addr, u8 *writedata)
{
    char status;
     
    transceive_buffer  *pi;
    pi = &mf_com_data;
#if (NFC_DEBUG)
    printf("write_sr176()->\n");
#endif
    pcd_set_tmo(5);
    mf_com_data.mf_command = PCD_TRANSMIT;
    mf_com_data.mf_length  = 4;
    mf_com_data.mf_data[0] = 9;
    mf_com_data.mf_data[1] = addr;
    mf_com_data.mf_data[2] = *writedata;
    mf_com_data.mf_data[3] = *(writedata+1);
    status = pcd_com_transceive(pi);
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif

    return status;  
}      


char protect_sr176(u8 lockreg)
{
    char status;
     
    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("protect_sr176()->\n");
#endif

    pcd_set_tmo(5);
    mf_com_data.mf_command = PCD_TRANSMIT;
    mf_com_data.mf_length  = 4;
    mf_com_data.mf_data[0] = 0x09;
    mf_com_data.mf_data[1] = 0x0F;
    mf_com_data.mf_data[2] = 0;
    mf_com_data.mf_data[3] = lockreg;
    status = pcd_com_transceive(pi);
    
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif

    return status;  
}   


char completion_sr()
{
    char status;
     
    transceive_buffer  *pi;
    pi = &mf_com_data;
#if (NFC_DEBUG)
    printf("completion_sr()->\n");
#endif
    pcd_set_tmo(5);
    mf_com_data.mf_command = PCD_TRANSMIT;
    mf_com_data.mf_length  = 1;
    mf_com_data.mf_data[0] = 0x0F;
    status = pcd_com_transceive(pi);
#if (NFC_DEBUG)
    printf(" sta=%bd\n", status);
#endif
    return status;  
}