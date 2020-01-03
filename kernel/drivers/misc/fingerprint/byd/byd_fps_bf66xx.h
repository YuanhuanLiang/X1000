/*
 * File:         byd_fps_bf66xx.h
 *
 * Created:	     2014-11-07
 * Depend on:    byd_fps_bf66xx.c
 * Description:  BYD Fingerprint IC driver for Android
 *               - Spreadtrum SP8932 configuration
 *
 * Copyright (C) 2014 BYD Company Limited
 *
 * Licensed under the GPL-2 or later.
 *
 * History:
 *
 * Contact: qi.ximing@byd.com;
 *
 */
#ifndef __FPS_BF66XX_H__
#define __FPS_BF66XX_H__
//#define PLATFORM_MTK             // MTK kernel 3.18 platform
//#define PLATFORM_MTK_KERNEL3_10  // MTK kernel 3.10 platform
//#define PLATFORM_QUALCOMM        // Qualcomm msm8909 platform
#define USE_ANDROID_M_ARCH
//#define PLATFORM_SPRD              //Spreadtrum SP8932 platform
#define PLATFORM_INGENIC
/*******************************************************************************
* 第一部分：平台相关设置 - 通讯设置
*     通讯配置是对中断端口及 SPI的初始化配置， SPI配置通常在Kernel代码的配置文件
*  dts中（通常在 arch/arm/boot中），或在主板特定的初始化代码文件中（通常为
*  arch/arm/mach-xxxx/board-xxxx.c，在spi_board_info结构中定义），中断配置通常放
* 在驱动平台代码byd_fps_platform.c的端口配置函数 byd_fps_platform_init中。
*     将 SPI设备挂载到平台的 SPI总线，需要告知平台从设备驱动名BYD_FPS_DEV_NAME，
* 所用SPI总线编号及从设备 SPI通讯配置，驱动需要被告知平台分配给本设备的中断号
* BYD_FPS_IRQ_NUM，或者中断端口号。
*******************************************************************************/

/* 1. 配置中断 */

/* 中断触发类型，需与IC给定的一致，如IRQF_TRIGGER_FALLING, IRQF_TRIGGER_LOW,
IRQF_TRIGGER_RISING, IRQF_TRIGGER_HIGH */
#define EINT_TRIGGER_TYPE  IRQF_TRIGGER_RISING

/* 中断请求号IRQ，定义为一常量。可选 */
//#define BYD_FPS_IRQ_NUM  9

/* 中断端口号，可选
    中断端口号用在函数byd_fps_platform_init() 中进行中断端口配置。此外，如果无法
直接给出上述的中断请求号，必须给出中断端口号*/
#ifndef BYD_FPS_IRQ_NUM
#define BYD_FPS_EINT_PORT  //GPIO_PA(21)移到/arch/mips/xburst/soc-1000/chip-x1000/ilock/ilock-v20/board.h
#endif

/* 2. 复位端口号 和 POWER使能端口号, 可选 */
#define BYD_FPS_RESET_PORT	//GPIO_PA(20)	//GPIO_FP_RST_PIN //BYD_FPS_RST_PIN ==GPIO97 //BYD_FPS_IRQ_PIN == GPIO99
#define BYD_FPS_POWER_PORT	//GPIO_PB(22)


/* 3. SPI端口配置 */
#define BYD_FPS_CSS_PORT	GPIO_SPI_CS_PIN
#define BYD_FPS_SCK_PORT	GPIO_SPI_SCK_PIN
#define BYD_FPS_MISO_PORT	GPIO_SPI_MISO_PIN
#define BYD_FPS_MOSI_PORT	GPIO_SPI_MOSI_PIN
#define BYD_FPS_CSS_PORT_MODE	GPIO_SPI_CS_PIN_M_SPI_CSA
#define BYD_FPS_SCK_PORT_MODE	GPIO_SPI_SCK_PIN_M_SPI_CKA
#define BYD_FPS_MISO_PORT_MODE	GPIO_SPI_MISO_PIN_M_SPI_MIA
#define BYD_FPS_MOSI_PORT_MODE	GPIO_SPI_MOSI_PIN_M_SPI_MOA

/* 4. SPI速度（实现特定于平台，目前TINY4412, QUALCOMM和君正有效） */
#define  BYD_FPS_SPI_SPEED  (1 * 1000 * 1000) // 3， 1.8    (1 * 1000 * 1000)

#define SPI_OS_SPEED  1000 //扫描失调值的spi 速度配置 建议1000k 64次积分
#define SPI_FP_SPEED  6000 //扫描指纹图像时的失调值配置 ：6次积分：6000k ；32次积分：1300k

/* 5. mtk平台主机SPI每次读出有数量限制，某些平台需4BYTE对齐 */
//#define SPI_TRANS_4BYTE 

/*******************************************************************************
* 第二部分：芯片设置
*******************************************************************************/

/* 1. 芯片型号选择，只能单选（不能同时多选） */
//#define CONFIG_FPS12  0x5040 
//#define CONFIG_FPS11  0x4840 
//#define CONFIG_FPS22    0x7040
#define  CONFIG_FPS05   0x5880
/* 2. 版本信息，如下格式：
      IC编码-IC封装码-驱动版本号-客户编码-项目编码-参数版本编码 */
#if defined(CONFIG_FPS12)
	#define BYD_FPS_DRIVER_DESCRIPTION  "FPS12B-BF663x-DRV4.11-CLIENT0000-PROJ00-PARA1.0"
#elif defined(CONFIG_FPS11)
	#define BYD_FPS_DRIVER_DESCRIPTION  "FPS11A-BF663x-DRV4.11-CLIENT0000-PROJ00-PARA1.0"
#elif defined(CONFIG_FPS22)
	#define BYD_FPS_DRIVER_DESCRIPTION  "FPS22A-BF663x-DRV4.11-CLIENT0000-PROJ00-PARA1.0"
#elif defined(CONFIG_FPS05)	
	#define BYD_FPS_DRIVER_DESCRIPTION  "FPS05A-BF663x-DRV4.11-CLIENT0000-PROJ00-PARA1.0"
#endif

#define BYD_FPS_FAE_FINGER_DOWN_DIFF	300 // 2000 // 按下抬起差值

/*******************************************************************************
* 第三部分：算法参数设置
*******************************************************************************/

#if (defined BYD_FPS_ALG_IN_KERNEL) || (defined BYD_FPS_ALG_IN_TZ)
#ifdef USE_ANDROID_M_ARCH
#define CONFIG_TEMPLATE_FILE_PATH  "/media/"//"/data/system/fingerprint/"
#else
#define CONFIG_TEMPLATE_FILE_PATH  "/data/data/com.fingerprints.fps/files/template/"
#endif
#define CONFIG_ALG_NUM_PARA		//如果需要配置算法模板数量参数，打开该宏
#define CONFIG_ALG_AREA_PARA	//如果需要配置算法有效面积参数，打开该宏


// config maximum number of fingers and the number of templates per finger
#ifdef  CONFIG_ALG_NUM_PARA
#define CONFIG_ALG_MAX_NUM_FINGERS			  	    CONFIG_BYD_FINGERPRINT_MAX_NUM
#define CONFIG_ALG_NUM_TEMPLATES_PER_FENGER	  		3//8
#define CONFIG_ALG_MAX_NUM_TEMPLATES          		15//99//20		//一个手指最多模板数量
#define CONFIG_ALG_SAME_FJUDGE                		1	//是否允许统一手指录入同一ID，0：允许；1：不允许
#endif

// config sensor area
#ifdef  CONFIG_ALG_AREA_PARA
#define CONFIG_ALG_SENSOR_VALID_ENROLL_AREA			50
#define CONFIG_ALG_SENSOR_INVALID_ENROLL_BLOCK		50
#define CONFIG_ALG_SENSOR_VALID_MATCH_AREA			60
#define CONFIG_ALG_SENSOR_INVALID_MATCH_BLOCK		60
#define CONFIG_ALG_SENSOR_INVALID_VARIANCE			6
#define CONFIG_ALG_SENSOR_MOVEXY					20
#endif

#endif // BYD_FPS_ALG_IN_KERNEL or BYD_FPS_ALG_IN_TZ


/*******************************************************************************
* 第四部分：公共声明 - 研发内部使用
*******************************************************************************/

/* 设备ID及驱动名，必须与平台的设备配置文件所定义的SPI设备名称一致 */
#define BYD_FPS_NAME "byd_fps"
#define BYD_FPS_FASYNC

/* 端口配置方式：
    如果端口配置在平台端（设备配置文件或端口配置函数）中，请打开该定义，否则表示
调用驱动的端口配置函数 */
//#define BYD_FPS_PLATFORM_INIT

/* 以EARLYSUSPEND、FB、PM_SLEEP、LINUX SUSPEND/RESUME为优先顺序，请undef所采用休
   眠方式之前的所有定义*/
//#undef CONFIG_HAS_EARLYSUSPEND
#undef CONFIG_HAS_EARLYSUSPEND
#undef CONFIG_FB
#undef CONFIG_PM_SLEEP

/* SPI在probe中初始化失败问题，仅在个别客户平台出现 */
#ifdef PLATFORM_MTK_KERNEL3_10
#define MTK_SPI_INIT_BUG_FIX
#endif

#define FPS_THREAD_MODE_IDLE             0
#define FPS_THREAD_MODE_ALG_PROC         1
#define FPS_THREAD_MODE_EXIT             6

#include <linux/semaphore.h>

/**
 * struct byd_fps_thread_task - thread task data
 * @mode:	current thread mode
 * @should_stop:	thread is supposed to be stopped
 * @sem_idle:
 * @wait_job:
 * @thread:
 *
 */
struct byd_fps_thread_task {
	int mode;
	int should_stop;
	struct semaphore sem_idle;
	wait_queue_head_t wait_job;
	struct task_struct *thread;
};


#endif
