/**
 ****************************************************************
 * @file mifare.c
 *
 * @brief  mifare protocol driver
 *
 * @author
 *
 *
 ****************************************************************
 */


#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "mifare.h"

#define NFC_DEBUG 0

/**
 ****************************************************************
 * @brief pcd_auth_state()
 *
 * ���ܣ��ô����FIFO�е���Կ�Ϳ��ϵ���Կ������֤
 *
 * @param: auth_mode=��֤��ʽ,0x60:��֤A��Կ,0x61:��֤B��Կ
 * @param: block=Ҫ��֤�ľ��Կ��
 * @param: psnr=���к��׵�ַ,��UID
 * @return: status ֵΪMI_OK:�ɹ�
 *
 ****************************************************************
 */
char pcd_auth_state(u8 auth_mode, u8 block, u8 *psnr, u8 *pkey)
{
    char status;
    u8 i;

    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("AUTH:\n");
#endif
    write_reg(BitFramingReg,0x00);  // // Tx last bits = 0, rx align = 0
 //  clear_bit_mask(RxModeReg, BIT7); //��ʹ�ܽ���crc�� added by xlwang,20180404

    pcd_set_tmo(4);

    mf_com_data.mf_command = PCD_AUTHENT;
    mf_com_data.mf_length = 12;
    mf_com_data.mf_data[0] = auth_mode;
    mf_com_data.mf_data[1] = block;
    for (i = 0; i < 6; i++)
    {
        mf_com_data.mf_data[2+i] = pkey[i];
    }
    memcpy(&mf_com_data.mf_data[8], psnr, 4);

    status = pcd_com_transceive(pi);

    if (MI_OK == status)
    {
        if (read_reg(Status2Reg) & BIT3) //MFCrypto1On
        {
            status = MI_OK;
        }
        else
        {
            status = MI_AUTHERR;
        }
    }

    return status;

}



/**
 ****************************************************************
 * @brief pcd_read()
 *
 * ���ܣ���mifare_one����һ��(block)����(16�ֽ�)
 *
 * @param: addr = Ҫ���ľ��Կ��
 * @param: preaddata = ��Ŷ��������ݻ��������׵�ַ
 * @return: status ֵΪMI_OK:�ɹ�
 * @retval: preaddata  ����������
 *
 ****************************************************************
 */
char pcd_read(u8 addr,u8 *preaddata)
{
    char status;

    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("READ:\n");
#endif

    write_reg(BitFramingReg,0x00);  // // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7);
    set_bit_mask(RxModeReg, BIT7);
    pcd_set_tmo(4);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_READ;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status == MI_OK)
    {
        if (mf_com_data.mf_length != 0x80)
        {
            status = MI_BITCOUNTERR;
        }
        else
        {
            memcpy(preaddata, &mf_com_data.mf_data[0], 16);
        }
    }

    return status;
}


/**
 ****************************************************************
 * @brief pcd_write()
 *
 * ���ܣ�д���ݵ����ϵ�һ��
 *
 * @param: addr = Ҫд�ľ��Կ��
 * @param: pwritedata = ���д�����ݻ��������׵�ַ
 * @return: status ֵΪMI_OK:�ɹ�
 *
 ****************************************************************
 */
char pcd_write(u8 addr,u8 *pwritedata)
{
    char status;
    // u8 ret;
    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("WRITE:\n");
#endif

    write_reg(BitFramingReg,0x00);  // // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7);
    clear_bit_mask(RxModeReg, BIT7);
  
    pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_WRITE;
    mf_com_data.mf_data[1] = addr;

    status = pcd_com_transceive(pi);
    if (status != MI_NOTAGERR)
    {
        if(mf_com_data.mf_length != 4)
        {
            status=MI_BITCOUNTERR;
        }
        else
        {
            mf_com_data.mf_data[0] &= 0x0F;
            switch (mf_com_data.mf_data[0])
            {
                case 0x00:
                    status = MI_NOTAUTHERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
    }
    if (status == MI_OK)
    {
        pcd_set_tmo(5);

        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_length  = 16;
        memcpy(&mf_com_data.mf_data[0], pwritedata, 16);

        status = pcd_com_transceive(pi);
        if (status != MI_NOTAGERR)
        {
            mf_com_data.mf_data[0] &= 0x0F;
            switch(mf_com_data.mf_data[0])
            {
                case 0x00:
                    status = MI_WRITEERR;
                    break;
                case 0x0A:
                    status = MI_OK;
                    break;
                default:
                    status = MI_CODEERR;
                    break;
            }
        }
        pcd_set_tmo(4);
    }
    return status;
}

/**
 ****************************************************************
 * @brief pcd_write_ultralight()
 *
 * ���ܣ�д���ݵ����ϵ�һ��
 *
 * @param: addr = Ҫд�ľ��Կ��
 * @param: pwritedata = ���д�����ݻ��������׵�ַ
 * @return: status ֵΪMI_OK:�ɹ�
 *
 ****************************************************************
 */
char pcd_write_ultralight(u8 addr,u8 *pwritedata)
{
    char status;

    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("WRITE_UL:\n");
#endif

    write_reg(BitFramingReg,0x00);  // // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7);
    clear_bit_mask(RxModeReg, BIT7);
    pcd_set_tmo(5);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 6; //a2h ADR D0 D1 D2 D3
    mf_com_data.mf_data[0] = PICC_WRITE_ULTRALIGHT;
    mf_com_data.mf_data[1] = addr;
    memcpy(&mf_com_data.mf_data[2], pwritedata, 4);

    status = pcd_com_transceive(pi);

    if (status != MI_NOTAGERR)
    {
        mf_com_data.mf_data[0] &= 0x0F;
        switch(mf_com_data.mf_data[0])
        {
            case 0x00:
                status = MI_WRITEERR;
                break;
            case 0x0A:
                status = MI_OK;
                break;
            default:
                status = MI_CODEERR;
                break;
        }
    }
    pcd_set_tmo(4);

    return status;
}