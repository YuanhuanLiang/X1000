/*
 * BYD_drv.h
 *
 *  Created on: 2017-5-3
 */

#ifndef BYD_FPS05_DRV_H_
#define BYD_FPS05_DRV_H_


//#include <stdint.h>


/**************************************************************************
* Global Macro Definition
***************************************************************************/

#define  DIV_VALUE      10
#define  VERVAL         0x80
#define  FGLIMIT_NUM    0x400
#define  ARRAY_LEN      (10)

#define  SPI_WR     1
#define  SPI_RD     0

//--------------return code-------------
#define RT_OK              0x00 //success
#define RT_FAIL            0x01 //fail
#define RT_NO_FG           0x02 //无手指
#define RT_CHIP_RESET_OK   0x03 //芯片复位成功
#define RT_CHIP_RESET_FAIL 0x04 //芯片复位失败
#define RT_CHIP_FAIL       0x05 //芯片不能使用
#define RT_FG_ERROR        0xFF //芯片手指触摸按键检测出错


//#define FINGER_DOWN    0xaa
//#define FINGER_UP      0x55

//------- FPS15 Register definition --------------
#define CONFIG_FPS05_SENSOR

#ifdef  CONFIG_FPS05_SENSOR

//fps
#define REG_FP_SENSOR_MODE     0x01
#define REG_FP_TX_CLK_SEL      0x02
#define REG_FP_TX_CLK_H_TIME   0x03
#define REG_FP_SEN_RST_TIME    0x04
#define REG_FP_INIT_NUM        0x06


//fps
#define REG_FIR_CMD            0x09
#define REG_FIR_DATA           0x0A
#define REG_DIG_INTE_SEL       0x0B

#define REG_ADC_DP_MODE        0x0E
#define REG_DUMMY_CFG_H        0x0F
#define REG_DUMMY_CFG_L        0x10
#define REG_FPD_DATA           0x11

#define REG_SRAM_EMPTY         0x13

//finger
#define REG_FG_SENSOR_MODE     0x14
#define REG_FG_TX_CLK_SEL      0x15
#define REG_FG_TX_CLK_H_TIME   0x16
#define REG_FG_SEN_RST_TIME    0x17
#define REG_FG_INIT_NUM        0x19

//finger

#define REG_FPD_TH_ON_H        0x1B
#define REG_FPD_TH_ON_L        0x1C
#define REG_FPD_TH_OFF_H       0x1D
#define REG_FPD_TH_OFF_L       0x1E
#define REG_INT_FINGER_CFG     0x1F

#define REG_FINGER_STATE       0x21
#define REG_SUB_VALUE_H        0x22
#define REG_SUB_VALUE_L        0x23

//sys
#define REG_CHIP_MODE          0x30
#define REG_AREA_ROW_START     0x31
#define REG_AREA_ROW_END       0x32

 
//sys
#define REG_FG_REST_TI1		     0x35
#define REG_FG_REST_TI2	       0x36
#define REG_CFG_FLAG	         0x37
#define REG_RST_STATE	         0x38
#define REG_SOFT_RST	         0x39
#define REG_INT_STATE	         0x3A
#define REG_INT_STATE_CLR	     0x3B


#define REG_INT_CFG		         0x3D
#define REG_IO_STATE_SEL	     0x3E
#define REG_PD_ANA_A	         0x3F
#define REG_PD_ANA_B	         0x40

//sys

#define REG_ANA_INIT_TIME	    0x42
#define REG_ADC_I_SEL	        0x43
#define REG_SEN_BUF_I	        0x44
#define REG_IN_PHASE	        0x45
#define REG_SH_I_SEL	        0x46
#define REG_TX_CFG	            0x47
#define REG_RING_SEL	        0x4A

#define REG_DUMMY_SFR_1       0x57
//key
#define REG_SYS_CLK_SEL	      0x58
#define REG_FINGER_CFG	      0x59
#define REG_FG_ERROR_STATE	  0x5A
#define REG_IO_CFG	          0x5B

#define REG_KEY_SCAN_CFG	    0x6E
#define REG_BASE_RST_CFG	    0x6F
#define REG_KEY_THD_H	        0x70
#define REG_KEY_THD_L	        0x71
#define REG_BASE_RST_THD_H	  0x72
#define REG_BASE_RST_THD_L	  0x73
#define REG_KEY_BARREL_THD	  0x74
#define REG_KEY_BASELINE_H	  0x75
#define REG_KEY_BASELINE_L	  0x76
#define REG_KEY_DATA_H	      0x77
#define REG_KEY_DATA_L	      0x78
#define REG_KEY_TEST_MODE	    0x79


//fpd测试
#define REG_FPD_TEST_MODE	   0x24//
#define REG_FPD_TEST_CFG	   0x25

//OTP
#define REG_SFR_OTP_CMD			0x63
#define REG_SFR_OTP_DATA		0x64
#define REG_OTP_CMD				0x65
#define REG_OTP_DATA			0x66
#define REG_OTP_SFR_CMD	     	0x63
#define REG_OTP_SFR_DATA	    0x64
//修调寄存器
#define REG_ADJ_SFR_ADDR	   0x68
#define REG_ADJ_SFR_DATA	   0x6B

//手指失调值寄存器
#define REG_FINGER_OSDATA_H	   0x5B
#define REG_FINGER_OSDATA_L	   0x5C

//产品信息寄存器
#define REG_CHIP_INFO_L   0x69
#define REG_CHIP_INFO_H	  0x6A
#define VAL_CHIP_INFO_LA  0x81
#define VAL_CHIP_INFO_LB  0x94
#define VAL_CHIP_INFO_H	  0x5E

//interrupt state  mask & constants
#define BYD_FPS_IRQ_FG_ERROR      0x10
#define BYD_FPS_IRQ_TIMER	        0x08
#define BYD_FPS_IRQ_FINGER	      0x0c
//#define BYD_FPS_IRQ_NOFINGER	    0x04
#define BYD_FPS_IRQ_DATA_ERROR    0x02
#define BYD_FPS_IRQ_DATA_READY	  0x01

#endif


//------- FPS05 normalmode Register value definition --------------

//fps
#define VAL_FP_SENSOR_MODE     0x02
#define VAL_FP_TX_CLK_SEL      0x00
#define VAL_FP_TX_CLK_H_TIME   0x1f
#define VAL_FP_SEN_RST_TIME    0x1c
#define VAL_FP_INIT_NUM        0x03


//
#define VAL_REG_FIR_CMD_WR     0x58
#define VAL_REG_FIR_CMD_RD     0xa3

//#define VAL_DIG_INTE_SEL       0x07//指纹扫描积分次数 16

//#define VAL_DIG_INTE_SEL       0x03//指纹扫描积分次数 8

#define VAL_DIG_INTE_SEL       0x02//指纹扫描积分次数 6



#define VAL_SRAM_EMPTY         0x00

//finger
#define VAL_FG_SENSOR_MODE    0x02
#define VAL_FG_TX_CLK_SEL     0x14
#define VAL_FG_TX_CLK_H_TIME  0x1f
#define VAL_FG_SEN_RST_TIME   0x1c
#define VAL_FG_INIT_NUM       0x03

#define VAL_FPD_TH_ON_H       0xff
#define VAL_FPD_TH_ON_L       0x0f
#define VAL_FPD_TH_OFF_H      0xff
#define VAL_FPD_TH_OFF_L      0x0f

#define VAL_INT_FINGER_CFG    0x01//0x00 //1：单脉冲 0：连续脉冲
#define VAL_INT_FINGER_NUM    0x00
#define VAL_FINGER_STATE      0x00

//sys
#define VAL_FG_REST_TI1		  0x01//快定时
#define VAL_FG_REST_TI2	   	  0x13//0x04//0x09//0x13//0x27//0x31//0x63//c7慢定时
#define VAL_FG_T1_TI2_STEP	5   //定时步进ms，芯片内部固定值，不能修改

#define VAL_INT_STATE	      0x00
#define VAL_INT_STATE_CLR	  0x00

/////////////////////////////////////////////////////////////////////////////////////////////
#define BYD_SWITCH_INT_WIDTH_EN  0//切换INT脉宽使能，低功耗手指扫描时脉宽变长

#define VAL_INT_LEVEL       0x01//1:H;0:L
#define VAL_INT_WIDTH       0x00//(VAL_INT_WIDTH+1)*8ms <= (VAL_FG_REST_TI1+1)*5
#define VAL_INT_CFG		      ((VAL_INT_LEVEL<<4)|VAL_INT_WIDTH)

#define VAL_LOWPOWER_INT_WIDTH  0x05//(VAL_LOWPOWER_INT_WIDTH+1)*8ms <= (VAL_FG_REST_TI2+1)*5
#define VAL_LOWPOWER_INT_CFG		((VAL_INT_LEVEL<<4)|VAL_LOWPOWER_INT_WIDTH)
//////////////////////////////////////////////////////////////////////////////////////////////
#define VAL_SPI_PULLDOWN_EN   0x00
#define VAL_FINGERFLAG_LEVEL  0x00//0:H; 1:L
#define VAL_FINGERFLAG_OUTPUT 0x00//0:ENABLE; 1:CLOSE
#define VAL_IO_STATE_SEL	    ((VAL_SPI_PULLDOWN_EN<<5)|(VAL_FINGERFLAG_LEVEL<<4)|(VAL_FINGERFLAG_OUTPUT<<3)|0x00)

//
#define VAL_ANA_INIT_TIME	    0x02
#define VAL_ADC_I_SEL	        0x02
#define VAL_SEN_BUF_I	        0x01
#define VAL_IN_PHASE	        0x0a
#define VAL_SH_I_SEL	        0x01


#define BYD_SH_METHOD_SEL   0//采失调值方案 
                             //0 被动采失调方案，需要操作FLASH; 
                             //2 主动式方案，不操作FLASH

														 
#if (BYD_SH_METHOD_SEL ==0) || (BYD_SH_METHOD_SEL == 1)
#define VAL_ADC_DP_MODE       0x00
#elif BYD_SH_METHOD_SEL == 2
#define VAL_ADC_DP_MODE       0x02
#else
#define VAL_ADC_DP_MODE       0x03
#endif

#if BYD_SH_METHOD_SEL==0
#define VAL_RING_SEL	        0xf3
#else
#define VAL_RING_SEL	        0xf8
#endif

#if BYD_SH_METHOD_SEL==0
#define VAL_RING_SEL_EN     0//铁环应用，1使能 0关闭
#else
#define VAL_RING_SEL_EN     1//必须使能，不能修改
#endif

#if VAL_RING_SEL_EN==0
#define VAL_PD_ANA_A	      0x00//0x20//
#define VAL_PD_ANA_B	      0x00//0x02//
#define VAL_TX_CFG	        0x31//0x01//
#else
#define VAL_PD_ANA_A	      0x00
#define VAL_PD_ANA_B	      0x00
#if BYD_SH_METHOD_SEL==0
#define VAL_TX_CFG	        0x71//内外激励结合
#else
#define VAL_TX_CFG	        0x70//外部激励，固定不能修改
#endif
#endif

#if (BYD_SH_METHOD_SEL==0) && (VAL_RING_SEL_EN==1)
#define BYD_FINGER_DETECT_TX_SEL  0   //0 与指纹扫描同样的tx配置; 1 手指检测时切换为只有外部激励
#define FINGER_VAL_TX_CFG         0x70//手指检测时关闭内部激励
#else
#define BYD_FINGER_DETECT_TX_SEL  0 //固定不能修改
#endif

//key
#define VAL_SYS_CLK_SEL	  		0x00//0x03//

#define TOUCH_KEY_EN      		0x00//兼容5323方案要关闭按键检测,无铁环方案也关闭检测即配置0x00;配置成0x01表示开按键检测功能。
#define TOUCH_SEN_EN      		0x01
#define KEY_ERROR_NUM     		0x0f//bit[5:2](KEY_ERROR_NUM+1)*16
#define VAL_FINGER_CFG	  		((KEY_ERROR_NUM<<2)|(TOUCH_KEY_EN<<1)|TOUCH_SEN_EN)
     
#define RXD_PULLUP_EN     		0x01//bit6
#define TXD_PULLUP_EN     		0x01//bit5
#define KEY_IO_STATE          0x01//bit4
#define TX_SR_EN              0x00//bit3
#define TX_SR_SEL             0x07//bit[2:0]
#define VAL_IO_CFG	          ((RXD_PULLUP_EN<<6)|(TXD_PULLUP_EN<<5)|(KEY_IO_STATE<<4)|(TX_SR_EN<<3)|TX_SR_SEL)

#define VAL_KEY_SCAN_CFG	    0x01

#define BASE_RST_SEL          0x02//0:1/1; 1:1/2; 2:1/4; 3:1/8
#define BASE_RST_NUM          0x04
#define VAL_BASE_RST_CFG	    ((BASE_RST_SEL<<4)|BASE_RST_NUM)

#define VAL_KEY_THD_H	        0x00
#define VAL_KEY_THD_L	        0x50//0x3c//60
#define VAL_BASE_RST_THD_H	  0x00
#define VAL_BASE_RST_THD_L	  0x28//0x1e//30
#define VAL_KEY_BARREL_THD	  0x0f

#define VAL_KEY_TEST_MODE	    0x00

#define BYD_KEY_RESET_EN	    1//0
#define VAL_KEY_BASELINE_H	  0x00
#define VAL_KEY_BASELINE_L	  0x64

//---------------------------------------------------------------

//------- FPS05 testmode Register value definition -------------- 


//指纹扫描数据寄存器
#define TEST_VAL_DIG_INTE_SEL       0x1F//64 指纹扫描积分次数
#define TEST_VAL_TX_CFG	            0x30
#define TEST_VAL_ADC_DP_MODE        0x00

//---------------------------------------------------------------

//------- FPS05 dummy Register and spi comunication mode --------------

#define BYD_FPS_READ_DMA_ENABLE         0//DMA读指纹数据使能,实现并行处理数据，节约时间提高帧频，推荐该方式

//当配置SPI速率高于指纹扫描速度时，要插入dummy,方式 1 SPI通信; 0 延时;
#if BYD_FPS_READ_DMA_ENABLE

#define BYD_FPS_READ_DUMMY_SELECT       1//不能修改
#define BYD_FPS_SET_DUMMY_REG_EN        0//配置获取图像dummy寄存器使能
#define BYD_FPS_SET_TESTDUMMY_REG_EN    0//配置获取失调dummy寄存器使能

#else //

#define BYD_FPS_READ_DUMMY_SELECT       0//普通SPI通信方式推荐延时

#if BYD_FPS_READ_DUMMY_SELECT==0

#define BYD_FPS_SET_DUMMY_REG_EN        0//不能修改
#define BYD_FPS_SET_TESTDUMMY_REG_EN    0//不能修改

#define BYD_FPS_DUMMY_DELAYTIME_EN      0//1//获取图像延时使能
#define BYD_FPS_DUMMY_DELAYTIME         7//延时us 获取图像

#define BYD_FPS_TESTDUMMY_DELAYTIME_EN  0//1//获取失调值延时使能
#define BYD_FPS_TEST_DUMMY_DELAYTIME    190//延时us 获取失调值

#else

#define BYD_FPS_SET_DUMMY_REG_EN        0//配置获取图像dummy寄存器使能
#define BYD_FPS_SET_TESTDUMMY_REG_EN    0//配置获取失调dummy寄存器使能

#endif
#endif

#if BYD_FPS_SET_DUMMY_REG_EN

#define VAL_DUMMY_CFG_H        0x00
#define VAL_DUMMY_CFG_L        0x09 //0x09(6次积分) //0x17(8次积分) //0x32(16次积分)
//#define VAL_DUMMY_CFG_L        0x07 //0x07(6次积分) //0x0f(8次积分) //0x2f(16次积分)

#else //

#define VAL_DUMMY_CFG_H        0x00
#define VAL_DUMMY_CFG_L        0x00

#endif

#if BYD_FPS_SET_TESTDUMMY_REG_EN

#define TEST_VAL_DUMMY_CFG_H        0x00
#define TEST_VAL_DUMMY_CFG_L        0xfa

#else
#define TEST_VAL_DUMMY_CFG_H        0x00
#define TEST_VAL_DUMMY_CFG_L        0x00

#endif

#define VAL_DUMMY_CFG          ((VAL_DUMMY_CFG_H<<8)|VAL_DUMMY_CFG_L)
#define TEST_VAL_DUMMY_CFG     ((TEST_VAL_DUMMY_CFG_H<<8)|TEST_VAL_DUMMY_CFG_L)
//---------------------------------------------------------------

//修调寄存器
#define VAL_ADJ_SFR_ADDR_H	   0x69
#define VAL_ADJ_SFR_ADDR_L	   0x6A
#define VAL_ADJ_SFR_DATA_H	   0x00
#define VAL_ADJ_SFR_DATA_L	   0xC0


//初版模式配置
#define VAL_CHIP_MODE_IDLE              0x00//空闲
#define VAL_CHIP_MODE_FINGER            0x33//手指扫描,快定时
#define VAL_CHIP_MODE_FINGER_LOWPOWER   0x55//手指扫描,慢定时,低功耗手指扫描
#define VAL_CHIP_MODE_FPD               0xAA//指纹扫描
#define VAL_CHIP_MODE_SLEEP             0xCC//休眠

//改版模式配置
#define VAL_CHIP_MODE_IDLE_SEC              0x55//空闲
#define VAL_CHIP_MODE_FINGER_SEC            0x66//手指扫描,快定时
#define VAL_CHIP_MODE_FINGER_LOWPOWER_SEC   0x00//手指扫描,慢定时,低功耗手指扫描
#define VAL_CHIP_MODE_FPD_SEC               0xff//指纹扫描
#define VAL_CHIP_MODE_SLEEP_SEC             0x99//休眠

#define CHIP_NORMALMODE                 0x11//正常工作模式,获取图像数据
#define CHIP_TESTMODE                   0x22//模组测试工作模式,获取失调值

#define VAL_CHIP_IDENTIFY_ONE   0x01//原版
#define VAL_CHIP_IDENTIFY_SEC   0x02//改版

#define BYD_OTP_OS_NUM         20
#define OS_STA_MAX_NUM         25
#define COLUMN_DATA_NUM        20//一行每1/8列数据个数 = 160/8

//-------------------- debug definition --------------
//#define DEBUG_MSG

//#define BYD_FPS_IMAGE_WIDTH	      160
//#define BYD_FPS_IMAGE_HEIGHT	    160
//#define BYD_FPS_IMAGE_BUFFER_SIZE 51200//(160 * 160 * 2)
#define BYD_FPS_READ_ROW_BUFMAX   240//((BYD_FPS_IMAGE_WIDTH*3)/2)//读芯片一行数据个数，不能修改

#define BYD_FPS_IMAGEDATA_MAXVALUE      4095//数据12bit最大值是4095

#define BYD_FPS_DEADPOINT_MAX           30//最大坏点个数                                     

#define BYD_FPS_WAIT_IRQ_ENABLE         0//等待指纹中断使能
//#define BYD_FPS_DEFAULT_IRQ_TIMEOUT     500//指纹检测等待中断电平时间

//#define BYD_FINGER_WAIT_IRQ_ENABLE      0//等待手指中断使能
//#define BYD_FINGER_DEFAULT_IRQ_TIMEOUT  500//手指检测等待中断电平时间

																				 
#define BYD_FPS_IMADGEDATA_MEMORY_SELECT  0 //存储最终图像数据方式,组合方式决定了读取时间和代码处理的复杂度 
																					  //0 存储37.5k(3个字节组合两个12bit方式 160*160*3/2)
																				    //1 存储50k(高低8位直接存储不组合 160*160*2)

#define BYD_FPS_DEADPOINT_HANDLE        1//0//坏点处理，1 使能 0关闭


#define IMAGE_DIFFERDATA_THD            100//判断有没有触摸
																					 //判断方式是计算所选区域里指纹数据最大值与最小值之差，小于该值认为没有触摸
#define IMAGE_WHITEDATA_PERCENT_A       9//统计图像的白点数量，超过百分比(160*160)*IMAGE_WHITEDATA_PERCENT_A/16认为无效

#define BYD_FPS_CHANGE_8BITS           1//16转8bit
#define IMAGE_JUDGE_ENABLE             0//图像判断,默认关闭
#define IMAGE_SHOW_ENABLE              0//按256*288分辨率显示图像

#if BYD_SH_METHOD_SEL==0
#define READ_OFFSETVAL_FROM_FLASH_EN    1//从flash中读取失调值使能
#define OFFSETVAL_FLASH_ADDRESS         ((uint32_t)0x08020000U) //存放失调值的flash地址
#define OFFSETVAL_FLASH_WRITE_SELECT    0 //flash存储失调值方式选择,组合方式决定了读取时间和代码处理的复杂度 
																				  //0 存储37.5k(3个字节组合两个12bit方式 160*160*3/2)
																				  //1 存储50k(高低8位直接存储不组合 160*160*2)
#if OFFSETVAL_FLASH_WRITE_SELECT
#define BYD_FPS_ERASE_PAGE_NUM          26
//#define MODULEID_FLASH_ADDRESS          ((uint32_t)0x0802c800U)//(OFFSETVAL_FLASH_ADDRESS+50*1024)存储模组ID的flash地址
//#define FINGERTHD_FLASH_ADDRESS         ((uint32_t)0x0802c80AU)//存储手指阈值的flash地址

#else
#define BYD_FPS_ERASE_PAGE_NUM          19
//#define MODULEID_FLASH_ADDRESS          ((uint32_t)0x08029600U) //(OFFSETVAL_FLASH_ADDRESS+37.5*1024)存储模组ID的flash地址
//#define FINGERTHD_FLASH_ADDRESS         ((uint32_t)0x0802960AU) //存储手指阈值的flash地址

#endif


#define BYD_FPS_MODULEID_CHECK_EN      1//模组ID判断，1 使能 0 关闭,样品调试阶段关闭,此时样品里还未烧录模组ID，ID值都是0xff

#define BYD_FPS_SUBVALUE_JUDGE_EN      1//0//判断手指失调值是否准确，1 使能 0关闭
#define BYD_FPS_AVESUBVALUE_THD        70//手指子区域平均值判断阈值，小于该值认为正确
#endif

#if BYD_SH_METHOD_SEL==0
#define BYD_FPS_SUBVALUE_DIFF          1850//手指子区域值减去该值得到手指阈值
#define BYD_FPS_SUBVALUE_DIFF_DOWN          1800//1800
#define BYD_FPS_SUBVALUE_DIFF_UP           	100//1000
#else
#define BYD_FPS_SUBVALUE_DIFF_DOWN          1800//1800
#define BYD_FPS_SUBVALUE_DIFF_UP           	100//1000
#endif


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
#pragma pack(1)
typedef struct 
{
	 uint8_t chipidentify;   //标志
	 uint8_t idlemode;			//空闲模式
   uint8_t fingermode;		//快速手指检测模式
   uint8_t lowpfingermode;//低功耗手指检测模式
   uint8_t fpdmode;				//指纹扫描模式
   uint8_t sleepmode;			//休眠模式
	 uint8_t fgresetti2;		//慢定时寄存器配置值
	 uint8_t intcfg;		    //快定时INT，即快速手指检测时
	 uint8_t lowintcfg;	    //慢定时INT，即低功耗手指检测时
	 uint8_t iostate;       //端口配置
	
}STRUCTCHIPMODIFYINFOR;
#pragma pack()



#define OTP_MODULE_ID             		0x80
#define OTP_MODULE_ID_END             	0x87
#define OTP_MODULE_BYTE           		8

#define OTP_BAD_POINT_CHIP_START		0x88
#define OTP_BAD_POINT_CHIP				0x89
#define OTP_BAD_POINT_CHIP_END			0xC4
#define OTP_BAD_POINT_CHIP_LEN			60

#define OTP_BAD_POINT_POTTING_START		0xC5
#define OTP_BAD_POINT_POTTING			0xC6
#define OTP_BAD_POINT_POTTING_END		0x101
#define OTP_BAD_POINT_POTTING_LEN		60



#define OTP_MARK						0x17F


#endif
