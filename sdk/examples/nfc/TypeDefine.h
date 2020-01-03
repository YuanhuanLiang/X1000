#ifndef __TYPEDEFINE_H__
#define __TYPEDEFINE_H__


#include <stdio.h>

#define DEBUG_PRINT_EN              //printf enable for debug
#define UART_SIMULATE_OPEN
#define YY_TEST
#define uprintf printf

#ifdef DEBUG
#define printfdbg printf
#else
#define printfdbg if (0) printf
#endif

#ifndef DEBUG_PRINT_EN
#define uprintf if (0) printf

#endif
#define SCREEN_FLAG
//#define    APPLICATION_DEBUG (1)
//end changed
//------------------------------------------------------------------------------


//------------------------------------------------------------------
//                        TypeDefs
//------------------------------------------------------------------
typedef    unsigned char            UINT8;    ///<unsigned char
typedef    unsigned short            UINT16;    ///<unsigned char

#ifndef    _UINT32_TYPE_
#define _UINT32_TYPE_
typedef unsigned int            UINT32;    ///<unsigned int
#endif


typedef    unsigned char            U08;    ///<unsigned char
typedef    unsigned short            U16;    ///<unsigned char
typedef unsigned int            U32;    ///<unsigned int

#ifndef    _INT8_TYPE_
#define _INT8_TYPE_
typedef    signed char             INT8;    ///< char
#endif

typedef    signed short            INT16;    ///<short

#ifndef    _INT32_TYPE_
#define _INT32_TYPE_
typedef    signed int              INT32;    ///<int
#endif

typedef    signed char             I08;    ///< char
typedef    signed short            I016;    ///<short
typedef    signed int              I032;    ///<int


typedef unsigned char            BOOL;    ///<BOOL
typedef unsigned char             U8;
typedef signed short             S16;
typedef signed int                 S32;
typedef signed char             S8;
typedef unsigned long long         U64;

typedef unsigned int             u32;
typedef unsigned short             u16;
typedef unsigned char             u8;

typedef signed int                 i32;
typedef signed short             i16;
typedef signed char             i8;

typedef unsigned char  T_Bool;
typedef unsigned char  T_U08;
typedef unsigned short T_U16;
typedef unsigned long  T_U32;

typedef char  T_S08;
typedef short T_S16;
typedef long  T_S32;

typedef unsigned char uchar;
typedef unsigned int    uint;
typedef unsigned char u8;
typedef unsigned short int  u16;
typedef unsigned int  u32;

/* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;

#define REG8(addr)          (*(volatile UINT8 *) (addr))
#define REG08(addr)          (*(volatile UINT8 *) (addr))
#define REG16(addr)          (*(volatile UINT16 *)(addr))
#define REG32(addr)          (*(volatile UINT32 *)(addr))


#define DATA_X  
#define DATA_I  
#define CONSTA   const

#define bool_t u8


#define HIBYTE(word)                ((U08)((word) >> 8))
#define LOBYTE(word)                ((U08)(word))
#define MAKEWORD(HiByte, LoByte)    ((((U16)(HiByte)<<8)&0xFF00) | ((U16)(LoByte)&0x00FF))
#define BYTE2WORD(HiByte, LoByte)    ((((U16)(HiByte)<<8)&0xFF00) | ((U16)(LoByte)&0x00FF))


//------- VUINT8 type definition --------------
#ifndef    _VUINT8_TYPE_
#define _VUINT8_TYPE_
typedef volatile unsigned char  VUINT8;
#endif

//------- VUINT16 type definition --------------
#ifndef    _VUINT16_TYPE_
#define _VUINT16_TYPE_
typedef volatile unsigned short     VUINT16;
#endif

//------- VUINT32 type definition --------------
#ifndef    _VUINT32_TYPE_
#define _VUINT32_TYPE_
typedef volatile unsigned long  VUINT32;
#endif

#ifndef    _PFILE_TYPE_
#define _PFILE_TYPE_
typedef UINT8*                       PFILE;
#endif

//-------------- General Return Code -------------
#define RT_OK            0x00 //success
#define RT_FAIL          0x01  //fail
#define RT_COMMAND_ERR   0x02  //command error
#define RT_PARAM_ERR     0x03  //param error
#define RT_OVERTIME      0x04  //over time
#define RT_ECC_ERR       0x05  //ecc error
#define RT_WRITE_ERR     0x06  //write flash err
#define RT_READ_ERR      0x07  //read flash err

// define BOOL value
#define FALSE    0
#define TRUE    1


void delay_1ms(unsigned int delay_time );
void pcd_sleep();

#endif

