/**************************************************************************************************
//--------------文件信息---------------------------------------------------------------------------
// 文 件 名:        StatusWord.h
// 版    本:        Ver1.0
// 创 建 人:        peterzheng
// 创建日期:        2012.12.14
// 文件描述:            定义R-APDU的状态字
//=================================================================================================
//-----------------修改记录------------------------------------------------------------------------
// 修改内容:
// 当前版本:        Ver1.0
// 修 改 人:
// 修改日期:
// 注    意:
//-------------------------------------------------------------------------------------------------
**************************************************************************************************/
#ifndef _STATUSWORD_H_
#define _STATUSWORD_H_

#define SW_OK                        0x9000//正常结束
#define SW_OK_APP                    0x9099//正常结束
#define SW_DATA_LEFT                0x6100//返回数据+61XX ，剩余数据长度
#define SW_DATA_LEN                    0x6C00//返回6CXX ，剩余数据长度
#define SW_INVALID_FILE                0x6283//应用锁定 或选择文件无效
#define SW_PIN_CNT                    0x63C0//返回0x63CX,校验错误,剩余X次尝试次数
#define SW_Blue_FAILD                0x6481//蓝牙通信失败
#define SW_OP_FLASH_FAILD            0x6581//操作FLASH失败
#define SW_LCLE_ERROR                0x6700//数据域错误,包括LC长度错误,对数据区数据解密结果错误
#define SW_FILETYPE_ERROR            0x6981//文件类型不匹配
#define SW_AC_ERROR                    0x6982//安全状态不满足
#define SW_KEY_LOCKED                0x6983//密钥已经被锁住
#define SW_NEED_RANDOM                0x6984//未取得随机数
#define SW_USE_CONDITION_ERROR        0x6985//使用条件不满足以及应用临时锁定
#define SW_MAC_ERROR                0x6988//安全报文数据项不正确
#define SW_CRYPT_ERROR                0x6989//加解密处理异常
#define SW_DATA_ERROR                0x6A80//数据域参数不正确
#define SW_UNSUPPORT                0x6A81//功能不支持
#define SW_NO_FILE                    0x6A82//没有找到文件
#define SW_NO_REECORD                0x6A83//没有找到记录
#define SW_NO_SPACE                    0x6A84//没有足够的空间
#define SW_PARAM_ERROR                0x6A86//P1，P2参数不正确
#define SW_INS_ERROR                0x6D00//不正确的INS
#define SW_CLA_ERROR                0x6E00//不正确的CLA
#define SW_UNDEFINED                0x6F00//未定义的错误(包括RSA计算的一些错误以及取响应的FLAG错误)
#define SW_APP_LOCKED                0x9303//应用永久锁定
#define SW_NO_KEY                    0x9403//密钥索引不支持
#define SW_APP_LOCKED                0x9303//应用永久锁定
//自定义SW
#define SW_CANT_DEAL                0x6900//不能处理
#define SW_STATE_ERROR                0x6901//当前状态错误,指的是当前状态不符合预期要求
#define SW_DATA_LACK                0x6902//关键数据缺失
#define SW_EMPTY_SAL_ERROR            0x6903//终端候选列表为空
#define SW_RCD_ERROR                0x6904//模板格式错误
#define SW_GPO_RSP_ERROR            0x6905//GPO响应数据异常
#define SW_GAC_RSP_ERROR            0x6906//GAC响应数据异常
#define SW_RDCACHE_ERROR            0x6907//数据缓存溢出
#define SW_DATA_AREA_ERROR            0x6908//数据超出了本身的值域
#define SW_NO_SERVER                0x6909//不允许服务
#define SW_TRANS_ERROR                0x690A//读卡模块传输错误
#define SW_ONLINE_ERROR                0x690B//联机失败
#define SW_FCI_ERROR                0x690C//FCI格式错误
#define SW_TL_ERROR                    0x690D//模板格式错误
#define SW_LOG_NULL                    0x690E//无相关信息
#define SW_GAC_AAC                    0x6910

//qxj自定义
#define SW_BLUE_INIT_ERROR          0x6D01         //个人化错误
#define SW_INIT_CRC_ERROR           0x6D02        //CRC校验错误
#define SW_INIT_LEN_ERROR           0x6D03       //91指令长度错误

#define SW_INIT_FINISHED            0x6D04 //设备已经个人化过
#define SW_INIT_FLASH_ERROR         0x6D05 //写flash失败
#define SW_RAND_ERROR               0x6D0B

#define SW_TEST_IC_ERROR            0x6D06 //操作IC卡错误
#define SW_TEST_MC_ERROR            0x6D07 //操作磁条卡错误
#define SW_TEST_TIMEOUT         0x6D08 //超时
#define  SW_TEST_M25_M25_FAILED         0x6D09
#define  SW_TEST_M25_M25_OTHER_FAILED         0x6D0A
//new define
#define SW_PUBKEY_NULL                    0x6A88//无有效RSA公钥

#endif