/**
 ****************************************************************
 * @file mh523.h
 *
 * @brief 
 *
 * @author 
 *
 * 
 ****************************************************************
 */ 
#ifndef MH523_H
#define MH523_H

#include"TypeDefine.h" 
#define BIT0   (1<<0)
#define BIT1   (1<<1)
#define BIT2   (1<<2)
#define BIT3   (1<<3)
#define BIT4   (1<<4)
#define BIT5   (1<<5)
#define BIT6   (1<<6)
#define BIT7   (1<<7)


/*
 * DEFINES Registers Address
 */
// PAGE 0
#define     RFU00                 0x00    // reserved for future use
#define     CommandReg            0x01    // starts and stops command execution
#define     ComIEnReg             0x02    // enable and disable interrupt request control bits
#define     DivIEnReg             0x03    // enable and disable interrupt request control bits
#define     ComIrqReg             0x04    // interrupt request bits
#define     DivIrqReg             0x05    // interrupt request bits
#define     ErrorReg              0x06    // error bits showing the error status of the last command executed
#define     Status1Reg            0x07    // communication status bits
#define     Status2Reg            0x08    // receiver and transmitter status bits
#define     FIFODataReg           0x09    // input and output of 64 byte FIFO buffer
#define     FIFOLevelReg          0x0A    // number of bytes stored in the FIFO buffer
#define     WaterLevelReg         0x0B    // level for FIFO underflow and overflow warning
#define     ControlReg            0x0C    // miscellaneous control registers
#define     BitFramingReg         0x0D    // adjustments for bit-oriented frames
#define     CollReg               0x0E    // bit position of the first bit-collision detected on the RF interface
#define     RFU0F                 0x0F    // reserved for future use 
// PAGE 1     
#define     RFU10                 0x10    // reserved for future use
#define     ModeReg               0x11    // defines general modes for transmitting and receiving
#define     TxModeReg             0x12    // defines transmission data rate and framing
#define     RxModeReg             0x13    // defines reception data rate and framing
#define     TxControlReg          0x14    // controls the antenna driver pins TX1 and TX2
#define     TxASKReg              0x15    // controls the setting of the transmission modulation
#define     TxSelReg              0x16    // selects the internal sources for the antenna driver
#define     RxSelReg              0x17    // selects internal receiver settings
#define     RxThresholdReg        0x18    // selects thresholds for the bit decoder
#define     DemodReg              0x19    // defines demodulator settings
#define     RFU1A                 0x1A    // reserved for future use
#define     RFU1B                 0x1B    // reserved for future use
#define     MfTxReg                  0x1C    // controls MIFARE communication transmit parameters
#define     MfRxReg               0x1D    // controls MIFARE communication receive parameters
#define     TypeBReg              0x1E    // controls the ISO/IEC 14443 B functionality
#define     SerialSpeedReg        0x1F    // selects the speed of the serial UART interface
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg              0x28
#define     ModGsPReg              0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     SpecialReg                0x3F


/*
 * DEFINES Registers bits
 ****************************************************************
 */
#define TxIEn         BIT6
#define RxIEn         BIT5
#define IdleIEn        BIT4
#define ErrIEn        BIT1
#define TimerIEn    BIT0
#define TxIRq         BIT6
#define RxIRq         BIT5
#define IdleIRq        BIT4
#define ErrIRq        BIT1
#define TimerIRq    BIT0

#define CollErr        BIT3
#define CrcErr        BIT2
#define ParityErr    BIT1
#define ProtocolErr BIT0

#define CollPos        (BIT0|BIT1|BIT2|BIT3|BIT4)

#define RxAlign        (BIT4|BIT5|BIT6)
#define TxLastBits    (BIT0|BIT1|BIT2)
/**
 * PCD������
 ****************************************************************
 */
#define PCD_IDLE              0x00               //ȡ����ǰ����
#define PCD_AUTHENT           0x0E               //��֤��Կ
#define PCD_RECEIVE           0x08               //��������
#define PCD_TRANSMIT          0x04               //��������
#define PCD_TRANSCEIVE        0x0C               //���Ͳ���������
#define PCD_RESETPHASE        0x0F               //��λ
#define PCD_CALCCRC           0x03               //CRC����
#define PCD_CMD_MASK          0x0F                 // ����������
#define PCD_CMD_SLEEP          0x10                 // оƬ��������״̬

/** 
 * Mifare Error Codes
 * Each function returns a status value, which corresponds to 
 * the mifare error
 * codes. 
 ****************************************************************
 */ 
#define MI_OK                            0
#define MI_CHK_OK                        0
#define MI_CRC_ZERO                        0

#define MI_CRC_NOTZERO                    1

#define MI_NOTAGERR                        (-1)
#define MI_CHK_FAILED                   (-1)
#define MI_CRCERR                        (-2)
#define MI_CHK_COMPERR                    (-2)
#define MI_EMPTY                        (-3)
#define MI_AUTHERR                        (-4)
#define MI_PARITYERR                    (-5)
#define MI_CODEERR                        (-6)
#define MI_SERNRERR                        (-8)
#define MI_KEYERR                        (-9)
#define MI_NOTAUTHERR                   (-10)
#define MI_BITCOUNTERR                  (-11)
#define MI_BYTECOUNTERR                    (-12)
#define MI_IDLE                            (-13)
#define MI_TRANSERR                        (-14)
#define MI_WRITEERR                        (-15)
#define MI_INCRERR                        (-16)
#define MI_DECRERR                        (-17)
#define MI_READERR                        (-18)
#define MI_OVFLERR                        (-19)
#define MI_POLLING                        (-20)
#define MI_FRAMINGERR                   (-21)
#define MI_ACCESSERR                    (-22)
#define MI_UNKNOWN_COMMAND                (-23)
#define MI_COLLERR                        (-24)
#define MI_RESETERR                        (-25)
#define MI_INITERR                        (-25)
#define MI_INTERFACEERR                 (-26)
#define MI_ACCESSTIMEOUT                (-27)
#define MI_NOBITWISEANTICOLL            (-28)
#define MI_QUIT                            (-30)
#define MI_INTEGRITY_ERR                (-35) //�����Դ���(crc/parity/protocol)


#define MI_ERR_PARAM                    (-36)  //��������
#define MI_ERR_TRANSMIT                 (-37)

#define MI_RECBUF_OVERFLOW              (-50) 
#define MI_SENDBYTENR                   (-51)    
#define MI_SENDBUF_OVERFLOW             (-53)
#define MI_BAUDRATE_NOT_SUPPORTED       (-54)
#define MI_SAME_BAUDRATE_REQUIRED       (-55)
#define MI_WRONG_PARAMETER_VALUE        (-60)
#define MI_BREAK                        (-99)
#define MI_NY_IMPLEMENTED                (-100)
#define MI_NO_MFRC                        (-101)
#define MI_MFRC_NOTAUTH                    (-102)
#define MI_WRONG_DES_MODE                (-103)
#define MI_HOST_AUTH_FAILED                (-104)
#define MI_WRONG_LOAD_MODE                (-106)
#define MI_WRONG_DESKEY                    (-107)
#define MI_MKLOAD_FAILED                (-108)
#define MI_FIFOERR                        (-109)
#define MI_WRONG_ADDR                    (-110)
#define MI_DESKEYLOAD_FAILED            (-111)
#define MI_WRONG_SEL_CNT                (-114)
#define MI_WRONG_TEST_MODE                (-117)
#define MI_TEST_FAILED                    (-118)
#define MI_TOC_ERROR                    (-119)
#define MI_COMM_ABORT                    (-120)
#define MI_INVALID_BASE                    (-121)
#define MI_MFRC_RESET                    (-122)
#define MI_WRONG_VALUE                    (-123)
#define MI_VALERR                        (-124)
#define MI_COM_ERR                     (-125)
#define PROTOCOL_ERR                    (-126)
///�û�ʹ�ô���
#define USER_ERROR                        (-127)

#define MAX_TRX_BUF_SIZE    255
#define INT_USE_CHECK_REG 1
typedef struct transceive_buffer{
    u8 mf_command;
    u16 mf_length;
    u8 mf_data[MAX_TRX_BUF_SIZE];
}transceive_buffer;

#define FSDI 8 //Frame Size for proximity coupling Device, in EMV test. ����֤����FSDI = 8

#define MOTHERBOARD_V20 0
#if (MOTHERBOARD_V20)
    #define INT_PIN = IomGetVal(5);
#endif

#if(INT_USE_CHECK_REG)
    #undef INT_PIN
    #define INT_PIN (read_reg(0x07) & 0x10)

#endif

extern transceive_buffer  mf_com_data;

#define P12 //= P1^2;
#define NRSTPD //= P0^2;
#define pcd_poweron()    //NRSTPD = 1
#define pcd_powerdown()    //NRSTPD = 0

void pcd_init();
void pcd_reset();
char pcd_config(u8 tag_type);

void write_reg(u8 addr, u8 val);
u8 read_reg(u8 addr);
char pcd_com_transceive(struct transceive_buffer *pi);

void set_bit_mask(u8 reg, u8 mask);  
void clear_bit_mask(u8 reg,u8 mask);  
void pcd_set_tmo(u8 fwi);
void pcd_set_rate(u8 rate);
void pcd_antenna_on();
void pcd_antenna_off();
void page45_lock();
void page4_unlock();
void page5_unlock();
void pcd_lpcd_start();
void pcd_lpcd_end();
u8 pcd_lpcd_check();
void pcd_delay_sfgi(u8 sfgi);
void pcd_lpcd_start();
void pcd_lpcd_config_start(u8 delta, u32 t_inactivity_ms, u8 skip_times,u8  t_detect_us);
void pcd_lpcd_end();
u8 pcd_lpcd_check();

#endif

