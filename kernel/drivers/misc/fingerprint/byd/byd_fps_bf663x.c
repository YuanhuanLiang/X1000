/*
 * File:         byd_fps_bf663x.c
 *
 * Created:	     2014-11-07
 * Description:  BYD Fingerprint IC driver for Android
 *
 * Copyright (C) 2014 BYD Company Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/spi/spi.h>
#include <linux/delay.h>      // msleep()
#include <linux/input.h>
#include <linux/interrupt.h>  // IRQ_HANDLED
#include <linux/kthread.h>

// byd_algorithm.h must be included after linux inclution and before byd_fps_*.h
#include "byd_algorithm.h"  // must after kernel head include
#include "byd_fps_bf66xx.h" //#include <linux/input/byd_fps_bf66xx.h>
#include "byd_fps_libbf663x.h"
#include  "BF5326.h"




#ifdef BYD_FPS_MISC_NAME
#include<linux/miscdevice.h>
#include<linux/fs.h>  /*异步通知机制 fasync*/
#endif
//-----------------------------------------------------------------------------
//指纹处理的算法放在TrustZone中执行
//-----------------------------------------------------------------------------
#ifdef BYD_FPS_ALG_IN_TZ
	//#define STOP_EVERY_TIME
	#define TZ_ENROL_SENC  0x00010000
#endif

//-----------------------------------------------------------------------------
//算法处理在内核里运行
//-----------------------------------------------------------------------------
#ifdef BYD_FPS_ALG_IN_KERNEL
	unsigned char byd_fps_add_txt;//是否加载模板到内存
#endif
unsigned short *byd_fps_image_data = NULL;
extern unsigned char byd_fps_susflag;//指纹唤醒标志

extern unsigned short byd_chip_info;

// -------------------------------------------------------------------- //
#define FPS_THREAD_MODE_IDLE             0
#define FPS_THREAD_MODE_ALG_PROC         1
#define FPS_THREAD_SCAN_OS_FNGUP         2
#define FPS_THREAD_MODE_EXIT             6

#define BYD_FPS_WORKER_THREAD_NAME		"fps_worker_thread"

//#define BYD_SAVE_IMAGE
// -----------------------------global variable--------------------------------------- //
struct byd_fps_thread_task *thread_task = NULL;
unsigned char work_cmd;
unsigned char byd_app_get_image;
unsigned char byd_last_cmd;

struct mutex byd_fps_mutex_chip;
struct mutex byd_fps_mutex_alg;

#ifdef BYD_FPS_FG_DETECT_DELAY
unsigned char byd_fps_fg_det_state = 0;
#endif

#ifdef BYD_FPS_GESTURE
int byd_fps_gest_init(struct device *dev);
void byd_fps_gest_exit(struct device *dev);
#endif


#if (defined BYD_FPS_ALG_IN_KERNEL) || (defined BYD_FPS_ALG_IN_TZ)
extern unsigned int byd_fps_match_id;
extern unsigned short byd_fp_id;

#define BYD_MULTI_MATCH_ENROL
#ifdef BYD_MULTI_MATCH_ENROL

unsigned char byd_show_flag = 0;
unsigned char byd_enrol_match_flag = 3;

#endif

unsigned char byd_fps_alg_ret[6];
char byd_fps_template_path[] = CONFIG_TEMPLATE_FILE_PATH;
//static 
unsigned char byd_fps_instr_stop_flag = 0;
 #ifdef BYD_FPS_ALG_IN_TZ
	#include "fingerprint_qsee.h"
	unsigned char qsee_run;
#endif
#endif


#ifdef BYD_FINGER_INTR_CNT
u8 finger_intr_flag = 0;
u8 finger_intr_num = 0;
#endif
u8 finger_intr_ctrl = 0;
// ------------------------------------------------------------------------- //
#ifdef BYD_FPS_SYSFS	
#define BUF_TRANS_SIZE 4000
unsigned char trans_num;
unsigned int trans_rest;
unsigned char read_first;
#endif


#ifdef BYD_SAVE_IMAGE
int byd_fps_write_file(char *p_path, char *buf, int len, int offset);
unsigned char byd_fps_img_file1[] = "/sdcard/byd_fps/byd_fps_img00.bin";

static unsigned char byd_fps_img_cnt=0;
#endif

#ifdef BYD_FPS_UP_DOWN_ASYNC
extern unsigned char byd_fps_finger_up_async_flag;
unsigned char g_need_finger_down_scan_flag = 0;
unsigned char g_need_finger_up_scan_flag = 0;
#endif


static unsigned char byd_fps_wait_flag = 0;

#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_FOCUS))
unsigned int byd_fps_keydown_flag;	//0-- 驱动中按键、手势应用的工作模式(默认的模式), 1--指纹采集应用的工作模式(上层应用主动发指令开启,完成后切换回默认模式).
#endif
#ifdef BYD_FPS_REPORT_KEY
extern unsigned char byd_fps_key_mesg;
void byd_fps_report_key_up(void);
#endif
#if defined(BYD_FPS_TIMER_KEY)||defined(BYD_FPS_FOCUS)
int byd_fps_timer_init(struct device *dev);
void byd_fps_timer_exit(struct device *dev);
#endif
extern unsigned char byd_fps_finger_up_flag;
extern unsigned char ReceiVeCmdType;
extern unsigned char byd_fps_ageing_test_flag;
extern unsigned short byd_fps_otp_th;
/*==================外部可用全局变量=====================*/
/*========================修改参数=======================*/
const unsigned char BYD_FPS_MAX_FRAME	= 1;
extern unsigned char byd_fps_int_test;

#ifdef CONFIG_FPS12
#define	BYD_FPS_SENSOR_WIDTH	96			//size of 1 line sensors.
#define	BYD_FPS_SENSOR_HEIGHT	96			//size of lines of sensors.
#define	BYD_FPS_IMG_DATA_WIDTH	144			//size of 1 line image data.
#define	BYD_FPS_IMG_DATA_HEIGHT	96			//size of lines of sensors.
#endif

#ifdef CONFIG_FPS11
#define	BYD_FPS_SENSOR_WIDTH	66			//size of 1 line sensors.
#define	BYD_FPS_SENSOR_HEIGHT	128		    //size of lines of sensors.
#define	BYD_FPS_IMG_DATA_WIDTH	99			//size of 1 line image data.
#define	BYD_FPS_IMG_DATA_HEIGHT	128		    //size of lines of sensors.
#endif


const unsigned int BYD_FPS_DEFAULT_IRQ_TIMEOUT =	(HZ/2);	//200; //((10/1000) * HZ)

unsigned char BYD_FPS_IMAGE_WIDTH = BYD_FPS_SENSOR_WIDTH_5531;
unsigned char BYD_FPS_IMAGE_HEIGHT = BYD_FPS_SENSOR_HEIGHT_5531;
unsigned char BYD_FPS_DATA_WIDTH = BYD_FPS_IMG_DATA_WIDTH_5531;
unsigned char BYD_FPS_DATA_HEIGHT = BYD_FPS_IMG_DATA_HEIGHT_5531;

unsigned int BYD_FPS_IMAGE_SIZE = (BYD_FPS_SENSOR_WIDTH_5531)*(BYD_FPS_SENSOR_HEIGHT_5531)*BYD_FPS_DATA_BYTE;
unsigned int BYD_FPS_IMAGE_BUFFER_SIZE = (((BYD_FPS_SENSOR_WIDTH_5531)*2+(BYD_FPS_SPI_DUMMY_NUM))*(BYD_FPS_SENSOR_HEIGHT_5531)+1);

const unsigned char FINGER_DOWN  = 1;
const unsigned char FINGER_UP  = 0;
const unsigned char BYD_FPS_INT_MODE  = 0;	//0: 帧中断模式, 1: 行中断模式.
const unsigned int BYD_FPS_SPI_DUMMY  = BYD_FPS_SPI_DUMMY_NUM;	//芯片的spi 时序要求:在较高时钟频率,且中断模式为帧中断时, 则需要人为添加dummy data, 以匹配主、从通信.
const unsigned char BYD_FPS_16BIT_IMAGE = BYD_FPS_DATA_BYTE;	//fp image data be fixed to 16 bit.
const unsigned char BYD_FPS_CHECK_SUB_AREA = BYD_FPS_CHECK_SUB_AREA_SET;	//设置为1, 则log中打印SUB_AREA_VALUE, 可据此决定手指检测的阈值.
const unsigned int BYD_FPS_SUB_AREA_SCAN_THRD = BYD_FPS_SUB_AREA_SCAN_THRD_SET;	//设置分块扫描的阈值.

#ifdef CONFIG_FPS22
const unsigned int BYD_FPS_CONFIG_VERSION = CONFIG_FPS22;	//设置芯片版本号,见byd_fps_bf66xx.h
#endif
#ifdef CONFIG_FPS05
const unsigned int BYD_FPS_CONFIG_VERSION = CONFIG_FPS05;
#endif

unsigned char thread_flag=0;
#ifdef BYD_FPS_SYSFS	
const unsigned short BYD_FPS_BUF_SIZE = BUF_TRANS_SIZE;
#endif

#if defined CONFIG_FPS12 || defined CONFIG_FPS11 || defined (CONFIG_FPS22) || defined (CONFIG_FPS05)
unsigned short byd_fps_fae_detect_para[6] = {0};//BYD_FPS_FAE_DETECT;
unsigned char byd_fps_fae_detect[12] = {0};//BYD_FPS_FAE_DETECT;
unsigned char byd_fps_fae_image[3] = {0};//BYD_FPS_FAE_IMAGE;
#ifndef CONFIG_FPS22 
unsigned short byd_fps_fae_fir[64] = {0};//BYD_FPS_FAE_FIR;
#endif
#endif

unsigned long set_timeout_finger_down=100;
unsigned long set_timeout_finger_up = 10;
unsigned long set_timeout_sub_scan = 20;
unsigned long set_timeout_image_scan = 10;//100;
/*==========================================================*/

struct byd_fps_data  *this_byd_fps = NULL;
#ifdef BYD_FPS_FOCUS
unsigned char gest_int_number = 0;
extern unsigned char byd_fps_gest_flag;
void byd_fps_start_gest_cacu(void);

#endif
/* ------------------------input && wakeup---------------------------------- */
#ifdef BYD_FPS_INPUT_WAKEUP
//extern void byd_fps_set_suspend_flag(char suspend_flag);
extern char byd_fps_get_suspend_flag(void);

int byd_fps_intr_work(void);
void byd_fps_wakeup(void);
void byd_fps_wakeup_exit(struct device *dev);
int byd_fps_wakeup_init(struct device *dev);
void byd_fps_disable_irq_wake(struct spi_device *spi);
#endif

#ifdef BYD_FPS_TIMER
int byd_fps_timer_init(struct device *dev);
void byd_fps_timer_exit(struct device *dev);
#endif

#ifdef BYD_FPS_GESTURE
int byd_fps_gest_init(struct device *dev);
void byd_fps_gest_exit(struct device *dev);
#endif


int byd_fps_spi_write(struct spi_device *spi, u8 reg, u8 val);
/* ------------------------local function declare--------------------------- */
void byd_fps_start_template_merging(void);
/* ------------------------------------------------------------------------- */

#ifdef BYD_FPS_FASYNC
static struct fasync_struct *byd_fps_async_queue; /*异步结构体指针，用于读*/ //sean 2014.12.16

static unsigned char byd_signal_sta = 0;
// -------------------------------------------------------------------- //
//
// 增加支持异步通知的函数 在release 函数中调用 // //sean 2014.12.16
// -------------------------------------------------------------------- //
static int byd_fps_fasync(int fd, struct file *filp, int mode)
{ 
 /*将文件从异步通知列表（相关进程列表）中删除*/

 return fasync_helper(fd, filp, mode, &byd_fps_async_queue);
}

extern int g_need_send_signal;		


void byd_fps_send_fasync(void)
{
	byd_signal_sta = 1;
	DBG_TIME("%s: byd -----------byd_signal_sta = %d------\n", __func__, byd_signal_sta);
	
	//异步通知上层处理.
	if(byd_fps_async_queue) {
		kill_fasync(&byd_fps_async_queue, SIGIO, POLL_IN);
	}
}
#endif

#ifdef BYD_FPS_MISC_NAME
static int byd_fps_misc_release(struct inode *inode, struct file *file)
{
	pr_debug("%s: entered.\n", __func__);
	byd_fps_fasync(-1,file,0);
	return 0;
}

static struct file_operations byd_fps_fops = {
	.owner   = THIS_MODULE,

	#ifdef BYD_FPS_FASYNC
	.fasync = byd_fps_fasync, /*不要忘了在这里也要加上 与内核联系的接口byd_fps_fasync*/
	#endif
	.release = byd_fps_misc_release
};

struct miscdevice byd_fps_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = BYD_FPS_MISC_NAME,
	.fops  = &byd_fps_fops,
};
#endif
/*
*byd_fps_mutex_lock
*@lock_flag --lock or unlock 1:lock 2:unlock
*@type_flag -- lock type 1:chip 2:alg
*return void
*/
void byd_fps_mutex_lock(unsigned char lock_flag,unsigned char type_flag)
{
	if((lock_flag == 1)&&(type_flag == 1)){			//加芯片锁, (1,1).
		mutex_lock(&byd_fps_mutex_chip);	
	} else if((lock_flag == 2)&&(type_flag == 1)){	//解芯片锁, (2,1).
		mutex_unlock(&byd_fps_mutex_chip);
	} else	if((lock_flag == 1)&&(type_flag == 2)){	//加算法锁, (1,2).
		mutex_lock(&byd_fps_mutex_alg);	
	}else if((lock_flag == 2)&&(type_flag == 2)){	//解算法锁, (2,2).
		mutex_unlock(&byd_fps_mutex_alg);	
	}
}
/* -------------------------------------------------------------------- */
/* interrupt service routing                                            */
/* -------------------------------------------------------------------- */

// *******************************************************************************
// * Function    :  byd_fps_bf66xx_interrupt
// * Description :  Interrupt service routine
// * In          :  none
// * Return      :  none
// *******************************************************************************
#ifdef PLATFORM_MTK_KERNEL3_10
void byd_fps_bf66xx_interrupt(void)
{
	struct byd_fps_data *byd_fps = this_byd_fps;
#else
irqreturn_t byd_fps_bf66xx_interrupt(int irq, void *dev_id)
{
	struct byd_fps_data *byd_fps = this_byd_fps;//(struct byd_fps_data *)dev_id;
#endif
	int ret;
	ret = 0;
	DBG_TIME("\nbyd byd_fps_bf66xx_interrupt()IN g_need_send_signal = %d\n", g_need_send_signal);

	

	#ifdef BYD_FPS_INPUT_WAKEUP
	if ((byd_fps_instr_stop_flag != 1)&&(byd_fps_int_test!=1)&&(byd_fps_susflag)) {
		ret = byd_fps_intr_work(); //最优先判断中断是否为异常中断 。suspend_flag:1,3--调用工作队列,在队列中处理: 采集、检测手指、比对指纹、发消息给上层.
		if(ret != 0) {
			goto exit;
		}
	}
	#endif
	
	#ifdef BYD_FINGER_INTR_CNT
	if (finger_intr_flag && finger_intr_ctrl == 2) {
		finger_intr_flag = 2;
		finger_intr_num++;
		DBG_TIME("%s: byd -----------finger_intr_num = %d------\n", __func__, finger_intr_num);
	}
	#endif
	
	if(byd_fps_int_test == 1) {
		byd_fps_int_test = 0;
		DBG_TIME("%s:byd int func\n",__func__);
	}
	
	if (byd_fps_wait_flag==1) {
		byd_fps_wait_flag = 0;
		byd_fps->interrupt_done = 1;
		wake_up_interruptible(byd_fps->waiting_interrupt_return);
	}
		
	byd_fps_susflag = 1;//只在指纹中断置1
	
	return IRQ_HANDLED;
exit:
	#ifndef PLATFORM_MTK_KERNEL3_10
	return IRQ_HANDLED;
	#endif
	pr_err("byd error_state happen!!!!");
}

unsigned short finger_throld=0;
extern unsigned int bpnum;
unsigned short byd_fps_chip_flag = 0;
extern void byd_fps_set_sensor_finger_detected(void);
#ifdef BYD_FPS_SYSFS
// *******************************************************************************
// * Function    :  byd_fps_bf66xx_spi_qsee_show
// * Description :  read data from driver by app in user space.
// * In          :  dev, attr, buf
// * Return      :  -
// *******************************************************************************
static ssize_t byd_fps_bf66xx_spi_qsee_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	int ret = -253;
	unsigned char byd_fps_show[12];
	unsigned char intr_state;
	unsigned int temp = 0;
	unsigned short throldvalue_on = 0;
	unsigned short throldvalue_off = 0;

	struct byd_fps_data  *byd_fps = NULL;
	byd_fps = this_byd_fps;
	buf[0] = 0;

	DBG_TIME("\nbyd show() IN, byd_last_cmd: %d\n", byd_last_cmd);
	switch (byd_last_cmd) {
		case 1:
		if(((byd_chip_info&0xf8c0)==BYD_FPS_CONFIG_VERSION)||((byd_chip_info&0xf8c0)==0xa880)) {
			buf[0] = 0x40;
			buf[1] = 0x40;
		}
		byd_last_cmd = 0;
		ret = 2;
		break;
		case 3:
		
			byd_fps_mutex_lock(1,1);
			buf[0] = (unsigned char)byd_fps_chip_intr_state(this_byd_fps->spi);
			buf[1] = byd_fps_get_finger_state(this_byd_fps->spi);
			byd_fps_mutex_lock(2,1);
			intr_state = buf[0];
			DBG_TIME("%s:intr_state = 0x%x,finger_state = %d\n",__func__,buf[0],buf[1]);
			buf[0] = buf[0]&0x04;
			buf[1] = buf[1]&0x01;
			if((g_need_finger_down_scan_flag == 1)&&(buf[1]!=1)){//需要检测按下，但是上层读到的状态不是按下，此时需要再次发送异步消息
				//g_need_finger_down_scan_flag = 0;//一直到采集到指纹并且比对成功才能取消该状态
				g_need_send_signal = 1;
				DBG_TIME("%s:byd fp need finger down,need async again\n",__func__);
			}			
			#ifdef BYD_FPS_UP_DOWN_ASYNC
			if(( intr_state & 0x10 )&&((g_need_finger_down_scan_flag = 1)&&(buf[1]!=1))){//???????,?????
				g_need_finger_down_scan_flag = 0;
				g_need_send_signal = 1;
				DBG_TIME("%s:ESD int,need finger down send async again\n",__func__);
				byd_fps_set_sensor_finger_detected();
			}
			if(( intr_state & 0x10)&&((g_need_finger_up_scan_flag = 1)&&(buf[1]!=0))){
				g_need_finger_up_scan_flag = 0;
				byd_fps_finger_up_async_flag = 1;
				DBG_TIME("%s:ESD int,need finger up send async again\n",__func__);
				byd_fps_set_sensor_finger_detected();
				g_need_send_signal = 0;
			}
			#endif
			#ifndef BYD_FPS_SYSFS
			error = byd_fps_copy_to_user(buf_u, buf_k, 2);
			if (error != 0) {	
				DBG_TIME("%s: copy_to_user failed() return: %d \n", __func__, error);
				return -254;
			}
			#endif
			byd_last_cmd = 0;
		ret = 2;
		break;
		case 5:
			DBG_TIME("%s:Fp scan,byd_show_flag = %d,byd_enrol_match_flag = %d\n",__func__,byd_show_flag,byd_enrol_match_flag);

		//1.单纯只是读取数据，不录入，不比对
			if (byd_last_cmd == 5 && byd_show_flag == 1) {
				int image_ret = 0;
				image_ret = byd_fps_show_image(buf, byd_fps);
				if (image_ret == 0) { //没有采集到图像
					return -110;
				} else { //APK界面(3201)和解锁,SETTINF ENROL界面(1)，获取到数据
					//byd_last_cmd = 10; //控制两次SHOW调用的地方不同
					return image_ret;
				}
				
			} else {
				if (byd_show_flag == 2) {
					switch (byd_enrol_match_flag) {
						case 1:
							byd_fps_multi_enrol(byd_fps_show);
						break;
						
						case 2:
							byd_fps_multi_match(byd_fps_show);
						break;
						
						default:
							DBG_TIME("%s: switch(byd_enrol_match_flag)=%d,not fit\n", __func__, byd_enrol_match_flag);
						break;
					}
				#ifdef BYD_FPS_UP_DOWN_ASYNC
					g_need_finger_down_scan_flag = 0;
				#endif
				#ifdef BYD_FPS_REPORT_KEY
					if (byd_fps_finger_up_flag == 1) {
						if (byd_fps_key_mesg == 1) {
							byd_fps_report_key_up();
							byd_fps_key_mesg = 2;
						}
					}
				#endif
				#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_FOCUS))
					DBG_TIME("%s:byd_fps_alg_ret[0]=%d\n",__func__,byd_fps_alg_ret[0]);
					if (byd_fps_alg_ret[0] == 1)
						byd_fps_keydown_flag = 0;//成功则开启按键
				#endif
					memmove(buf, byd_fps_show, 12);
					ret = 12;
					#if 0//def BYD_FPS_INPUT_WAKEUP
					byd_fps_disable_irq_wake(byd_fps->spi);
					#endif
					byd_last_cmd = 0;
				}
			}
		break;
		
		case 19:
			byd_last_cmd = 0;
			if (ReceiVeCmdType == 4) {  //读阈值 应该从寄存器里面取数 修改一下
				byd_fps_mutex_lock(1,1);
				temp = read_throld_on(byd_fps->spi);
				byd_fps_mutex_lock(2,1);
				DBG_TIME("%s:cmd19,temp value is=%d\n", __func__,temp);
				/*throldvalue_on = (65535 - temp)/4;//将按下的阈值缩小4倍上传
				DBG_TIME("%s:cmd19,throldvalue_on is=%d\n", __func__,throldvalue_on);
				byd_fps_mutex_lock(1,1);
				temp = read_throld_off(byd_fps->spi);
				byd_fps_mutex_lock(2,1);
				DBG_TIME("%s:cmd19,off temp value is=%d\n", __func__,temp);
				throldvalue_off = (65535 - temp)/4;//将抬起的阈值缩小4倍上传
				DBG_TIME("%s:cmd19,throldvalue_off is=%d\n", __func__,throldvalue_off);*/
			
			
				
				buf[0] = byd_fps_fae_detect_para[0];//throldvalue_on >> 8; 按下系数
				buf[1] = byd_fps_fae_detect_para[1];//throldvalue_on & 0xff;	抬起系数
				buf[2] = (byd_fps_otp_th) >>8; //throldvalue_off >> 8;变化量高位
				buf[3] = (byd_fps_otp_th) & 0xff;//throldvalue_off & 0xff;		
			
				ret = 4;
			} else if (ReceiVeCmdType == 6) { //读子区域数据
				byd_fps_mutex_lock(1,1);
				byd_fps_read_finger_detect_value(byd_fps->spi, &buf[0]);
				byd_fps_mutex_lock(2,1);
				/*#ifdef CONFIG_FPS12
					ret = 18;
				#elif defined CONFIG_FPS11
					ret = 10;
                #endif	*/
				#ifdef CONFIG_FPS22
				ret = 6;
				#endif
			} else if (ReceiVeCmdType == 7) { //读版本号
				byd_chip_info=0x00;
				DBG_TIME("%s:========byd_chip_info-1 = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
				byd_fps_mutex_lock(1,1);
				byd_chip_info = byd_fps_read_version(byd_fps->spi);
				byd_fps_mutex_lock(2,1);
				DBG_TIME("%s:========byd_chip_info = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
				if((byd_chip_info&0xf8c0) == BYD_FPS_CONFIG_VERSION){
					#ifdef CONFIG_FPS22
					if((byd_fps_chip_flag == 0x5531) || (byd_fps_chip_flag == 0x5532)){
						buf[1] = BYD_FPS_SENSOR_HEIGHT_5531;
						buf[2] = BYD_FPS_SENSOR_WIDTH_5531;
					}else if(byd_fps_chip_flag == 0x5541){
						buf[1] = BYD_FPS_SENSOR_HEIGHT_5541;
						buf[2] = BYD_FPS_SENSOR_WIDTH_5541;
					}
					#endif
				}
				ret = 3;
			} else if(ReceiVeCmdType == 8) {
			 
			    if(byd_fps_ageing_test_flag == 1){//spi测试
					byd_chip_info=0x00;
					DBG_TIME("%s:========byd_chip_info-1 = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
					byd_fps_mutex_lock(1,1);
					byd_chip_info = byd_fps_read_version(byd_fps->spi);
					byd_fps_mutex_lock(2,1);
					DBG_TIME("%s:========byd_chip_info = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
				
					if((byd_chip_info&0xf8c0) == BYD_FPS_CONFIG_VERSION){
						buf[0] = 0x1;
					} else { 
						buf[0] = 0x0;
					}
					
				}else if(byd_fps_ageing_test_flag == 2){//int 测试
					byd_fps_mdelay(150);
					if(byd_fps_int_test == 0){					
						DBG_TIME("%s:byd int come\n",__func__);	
						buf[0] = 0x1;
					}else{
						buf[0] = 0x0;
					}
					byd_fps_mutex_lock(1,1);
					byd_fps_int_test_cfg_back(byd_fps->spi);
					byd_fps_mutex_lock(2,1);

				}else if(byd_fps_ageing_test_flag == 3){//坏点测试
					/*byd_os_scan(byd_fps);
					DBG_TIME("%s:bad point num = %d\n",__func__,bpnum);
					if(bpnum<100)
						buf[0] = 0x1;
					else
						buf[0] = 0x0;*/
				}
				
				byd_fps_ageing_test_flag = 0;
				ret = 1;
			}
			
			ReceiVeCmdType = 0;
			#if defined(BYD_FPS_REPORT_KEY)||defined(BYD_FPS_FOCUS)
				byd_fps_keydown_flag = 0;//进入apk测试界面，关闭按键功能，退出测试界面发送09命令打开
			#endif
			
		break;
		default:
			ret = -253;
		break;
	}


	DBG_TIME("byd %s end, ret=%d\n", __func__, ret);

	return ret;
}

// *******************************************************************************
// * Function    :  byd_fps_bf66xx_spi_qsee_store
// * Description :  pass user space command
// * In          :  buf
// * Return      :  Status of command execution
// *******************************************************************************
#ifdef PLATFORM_MTK_KERNEL3_10 // for mtk platform specific bug
#ifdef MTK_SPI_INIT_BUG_FIX  // for client specific bug - SPI init failure
	int byd_fps_init_port(void);
#endif
#endif

static ssize_t byd_fps_bf66xx_spi_qsee_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	int ret=0;
	struct byd_fps_data  *byd_fps = NULL;

	byd_fps = this_byd_fps;
	if(size > 6)
		return -3;

	DBG_TIME("byd byd_fps_bf66xx_spi_qsee_store().[0]%d, [1]%d, [2]%d,[3]%d,[4]%d,[5]%d,work_cmd = %d\n", buf[0], buf[1], buf[2],buf[3],buf[4],buf[5],work_cmd);

	//byd_last_cmd = buf[0];

#if 0//def BYD_FPS_ALG_IN_KERNEL
	if(!byd_fps_add_txt){
		DBG_TIME("%s:byd_fps_add_txt  = %d\n",__func__,byd_fps_add_txt);

		//算法cache初始化，读取文件到内存
		ret = byd_fps_alg_init();
		//ret: -1, 申请内存失败
		//ret: -2, 加载文件失败
		//ret: -3, 文件读写失败
		if(ret < 0){
			byd_fps_alg_ret[0] = 255;
			byd_fps_alg_ret[1] = 250-ret;
			pr_err("%s:byd_fps_alg_init algorith failed,ret = %d\n",__func__, ret);
			if(ret == -1){
				return -5;
			}else{
				byd_fps_add_txt = 1;
			}
		}else{
			byd_fps_add_txt = 1;//加载模板到内存成功
			FP_DBG("%s:byd_fps_alg_init succeed,ret = %d\n",__func__,ret);
		}
	}
#endif
	
	#ifdef 	BYD_FPS_ALG_IN_TZ
		if(qsee_run == 0){
			if(!fp_qsee_start()){
				return -101;
			}else{
				DBG_TIME("byd call fp_ qsee_start()\n");
				qsee_run = 1;
			}
		}
	#endif

#ifdef PLATFORM_MTK_KERNEL3_10 // for mtk platform specific bug
#ifdef MTK_SPI_INIT_BUG_FIX // for client specific bug - SPI init failure
		byd_fps_init_port();
#endif
#endif

	ret = byd_sys_dev_ioctl(buf);//

	DBG_TIME("%s:byd Out,byd_sys_dev_ioctl(): %d\n", __func__, ret);

	return ret;
}

#ifdef PLATFORM_MTK_KERNEL3_10
static DEVICE_ATTR(fp_qsee, 0666, byd_fps_bf66xx_spi_qsee_show, byd_fps_bf66xx_spi_qsee_store);
#endif

#if defined(PLATFORM_QUALCOMM)||defined(PLATFORM_MTK)
static DEVICE_ATTR(fp_qsee, 0664, byd_fps_bf66xx_spi_qsee_show, byd_fps_bf66xx_spi_qsee_store);
#endif

#endif // def BYD_FPS_SYSFS

#ifndef BYD_FPS_SYSFS

/* Class       :  byd_fps_cdev
 * Methods     :  byd_fps_cdev_fops
 * Description :  create, remove FPS character device using devfs
 ******************************************************************************/

#include <linux/poll.h> // copy_from_user()
#include <linux/cdev.h>
#define BYD_FPS_CLASS_NAME			"byd_fps"
#define BYD_FPS_MAJOR				255//235
static int byd_fps_open(struct inode *inode, struct file *file);
static ssize_t byd_fps_write(struct file *file, const char *buff, size_t count, loff_t *ppos);
static ssize_t byd_fps_read(struct file *file, char *buff, size_t count, loff_t *ppos);
static int byd_fps_release(struct inode *inode, struct file *file);
static long byd_fps_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

struct byd_fps_cdev {
	struct class *class;
	struct device *device;
	struct cdev cdev;
	dev_t devno;
};

static const struct file_operations byd_fps_cdev_fops = {
	.owner = THIS_MODULE,
	.open  = byd_fps_open,
	.write = byd_fps_write,
	.read  = byd_fps_read,
	.release = byd_fps_release,
	.unlocked_ioctl = byd_fps_ioctl,
	#ifdef BYD_FPS_FASYNC
	.fasync = byd_fps_fasync
	#endif
};

static struct byd_fps_cdev *fps_cdev = NULL; // *** qiximing ***

static struct byd_fps_cdev *byd_fps_cdev_constr(void) //static int byd_fps_cdev_constr(void)
{
	int ret; // int err;
	struct byd_fps_cdev *fps_cdev = NULL;
	struct byd_fps_cdev *p_err = NULL;

	fps_cdev = kzalloc(sizeof(*fps_cdev), GFP_KERNEL);
	if (!fps_cdev) {
		pr_err("%s: failed to allocate memory for byd_fps_cdev\n", __func__);
		return fps_cdev; // return -ENOMEM;
	}

	fps_cdev->class = class_create(THIS_MODULE, BYD_FPS_CLASS_NAME);
	if (IS_ERR(fps_cdev->class)) {
		pr_err("%s: failed to create class for fps_cdev.\n", __func__);
		p_err = (struct byd_fps_cdev *)(fps_cdev->class); // err = PTR_ERR(fps_cdev->class);
		goto err_kzalloc;
	}

	fps_cdev->devno = MKDEV(BYD_FPS_MAJOR, 0);
	fps_cdev->device = device_create(fps_cdev->class, NULL, fps_cdev->devno, NULL, "%s", BYD_FPS_NAME);
	if (IS_ERR(fps_cdev->device)) {
		pr_err("%s: device_create() failed.\n", __func__);
		p_err = (struct byd_fps_cdev *)(fps_cdev->device); // err = PTR_ERR(fps_cdev->device);
		goto err_class;
	}

	/* create char device, for user space */
	ret = register_chrdev_region(fps_cdev->devno, 1, BYD_FPS_NAME);
	if (ret) {
		pr_err("%s: register_chrdev_region() failed.\n", __func__);
		goto err_chrdev;
	}
	cdev_init(&fps_cdev->cdev, &byd_fps_cdev_fops);
	fps_cdev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&fps_cdev->cdev, fps_cdev->devno, 1);
	if (ret) {
		pr_err("%s: cdev_add() failed.\n", __func__);
		goto err_cdev;
	}
	return fps_cdev; // return 0;

err_cdev:
	cdev_del(&fps_cdev->cdev);

err_chrdev:
	unregister_chrdev_region(fps_cdev->devno, 1);

err_class:
	device_destroy(fps_cdev->class, fps_cdev->devno);

err_kzalloc:
	kfree(fps_cdev);

	return p_err; // return p_err;
}

static void byd_fps_cdev_distr(struct byd_fps_cdev *fps_cdev)
{
	cdev_del(&fps_cdev->cdev);
	unregister_chrdev_region(fps_cdev->devno, 1);
	if (!IS_ERR_OR_NULL(fps_cdev->device))
		device_destroy(fps_cdev->class, fps_cdev->devno);
	class_destroy(fps_cdev->class);
	kfree(fps_cdev);
}

static int byd_fps_open(struct inode *inode, struct file *file)
{
	pr_debug("%s: entered.\n", __func__);
	return 0;
}

int byd_fps_copy_to_user(char *buf,char *buff,size_t count)
{
	int error = 0;
	
	error = copy_to_user(buf,buff,count);
	DBG_TIME("%s: byd_fps_copy_to_user() return: %d \n",__func__, error);
	
	return error;
}

// *******************************************************************************
// * Function    :  byd_fps_write
// * Description :  pass user space command
// * In          :  file, buff, count, ppos
// * Return      :  Status of command execution
// *******************************************************************************
static ssize_t byd_fps_write(struct file *file, const char *buff, size_t count, loff_t *ppos)
{
	unsigned char buf[100];//buf[6];
	int error = 0 ;
	int ret=0;

	struct byd_fps_data  *byd_fps = NULL;
	byd_fps = this_byd_fps;

    DBG_TIME("%s: is writing to driver, copy_from_user()\n", __func__);
	if(buff[0]==1) {
		error = copy_from_user(buf,buff,100);
	} else {
		error = copy_from_user(buf,buff,6);
	}
	if(error != 0){
		DBG_TIME("%s:byd_fps_copy_from_user failed.\n",__func__);
		return -253;
	}
	DBG_TIME("%s: is writing to driver, copy_to_user() return = %d \n", __func__, error);
	
	if(count > 6)
		return -3;

	DBG_TIME("%s: [0]%d, [1]%d, [2]%d,[3]%d,[4]%d,[5]%d,work_cmd = %d\n", __func__,
		buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], work_cmd);

//#ifdef BYD_FPS_ALG_IN_KERNEL
#if 0
	if((!byd_fps_add_txt) && (buf[0]!=1)  ){//上层第一次命令不为1的时候才加载模板到默认路径CONFIG_TEMPLATE_FILE_PATH，为1时加载到上层指定的路径
		DBG_TIME("%s:byd_fps_add_txt  = %d\n",__func__,byd_fps_add_txt);
		ret = byd_fps_alg_init();
		if(ret < 0){
			byd_fps_alg_ret[0] = 255;
			byd_fps_alg_ret[1] = 250 - ret;
			pr_err("%s:byd_fps_alg_init algorith failed,ret = %d\n", __func__, ret);
			if (ret == -1)
				return -5;
			else
				byd_fps_add_txt = 1;
		} else {
			byd_fps_add_txt = 1;//加载模板到内存成功
			FP_DBG("%s:byd_fps_alg_init succeed,ret = %d\n",__func__,ret);
		}
		
	}
#endif
#ifdef 	BYD_FPS_ALG_IN_TZ
	if(qsee_run == 0){
		if(!fp_qsee_start()){
			return -101;
		}else{
			DBG_TIME("byd call fp_ qsee_start()\n");
			qsee_run = 1;
		}
	}
#endif

	ret = byd_sys_dev_ioctl(buf);
	DBG_TIME("%s:byd Out,byd_sys_dev_ioctl(): %d\n", __func__, ret);
	return ret;
}

// *******************************************************************************
// * Function    :  byd_fps_read
// * Description :  read data from driver by app in user space.
// * In          :  file, buf_u, count, ppos
// * Return      :  -
// *******************************************************************************

static ssize_t byd_fps_read(struct file *file, char *buf_u, size_t count, loff_t *ppos)
{
	int ret = -253;
	int error = 0;
	unsigned char byd_fps_show[12];
	unsigned char intr_state;
	unsigned int temp = 0;
	//unsigned short throldvalue_on = 0;
	//unsigned short throldvalue_off = 0;

	struct byd_fps_data *byd_fps = this_byd_fps;
    char buf_k[18];

	
	DBG_TIME("\n %s IN, byd_last_cmd: %d,ReceiVeCmdType =%d,byd_fps_ageing_test_flag = %d\n", __func__, byd_last_cmd,ReceiVeCmdType,byd_fps_ageing_test_flag);
	

#ifdef 	BYD_FPS_ALG_IN_TZ
		if(qsee_run == 0){
			if(!fp_qsee_start()){
				return -254;
			}else{
				DBG_TIME("byd call fp_ qsee_start()\n");
				qsee_run = 1;
			}
		}
#endif

	switch (byd_last_cmd) {
		case 1:
		if ((byd_chip_info & 0xf980) == BYD_FPS_CONFIG_VERSION) {
			buf_k[0] = 0x40;
			buf_k[1] = 0x40;
#ifndef BYD_FPS_SYSFS
			error = byd_fps_copy_to_user(buf_u, buf_k, 2);
			if (error != 0) {	
				DBG_TIME("%s: copy_to_user failed() return: %d \n", __func__, error);
				return -254;
			}
#endif
		}
		byd_last_cmd = 0;
		ret = 2;
		break;
		case 3:
			byd_fps_mutex_lock(1,1);
			buf_k[0] = (unsigned char)byd_fps_chip_intr_state(this_byd_fps->spi);
			buf_k[1] = byd_fps_get_finger_state(this_byd_fps->spi);
			byd_fps_mutex_lock(2,1);
			intr_state = buf_k[0];
			DBG_TIME("%s:intr_state = 0x%x,finger_state = %d\n",__func__,buf_k[0],buf_k[1]);
			buf_k[0] = buf_k[0]&0x04;
			buf_k[1] = buf_k[1]&0x01;
			#ifdef BYD_FPS_UP_DOWN_ASYNC
			if((g_need_finger_down_scan_flag == 1)&&(buf_k[1]!=1)){//需要检测按下，但是上层读到的状态不是按下，此时需要再次发送异步消息
				//g_need_finger_down_scan_flag = 0;//一直到采集到指纹并且比对成功才能取消该状态
				g_need_send_signal = 1;
				DBG_TIME("%s:byd fp need finger down,need async again\n",__func__);
			}
			if(( intr_state & 0x10 )&&((g_need_finger_down_scan_flag = 1)&&(buf_k[1]!=1))){//静电打出的中断，非正常中断
				g_need_finger_down_scan_flag = 0;
				g_need_send_signal = 1;
				DBG_TIME("%s:ESD int,need finger down send async again\n",__func__);
				byd_fps_set_sensor_finger_detected();
			}
			if(( intr_state & 0x10 )&&((g_need_finger_up_scan_flag = 1)&&(buf_k[1]!=0))){
				g_need_finger_up_scan_flag = 0;
				byd_fps_finger_up_async_flag = 1;
				DBG_TIME("%s:ESD int,need finger up send async again\n",__func__);
				byd_fps_set_sensor_finger_detected();
				g_need_send_signal = 0;
			}
			#endif
			#ifndef BYD_FPS_SYSFS
			error = byd_fps_copy_to_user(buf_u, buf_k, 2);
			if (error != 0) {	
				DBG_TIME("%s: copy_to_user failed() return: %d \n", __func__, error);
				return -254;
			}
			#endif
			byd_last_cmd = 0;
			ret = 2;
		break;
		
		case 5:
		
			DBG_TIME("%s:Fp scan,byd_show_flag = %d,byd_enrol_match_flag = %d\n",__func__,byd_show_flag,byd_enrol_match_flag);
		
		//1.单纯只是读取数据，不录入，不比对
			if (byd_last_cmd == 5 && byd_show_flag == 1) {
				int image_len = 0;
				DBG_TIME("byd byd_fps_show_image start!!!\n");
				ret = byd_fps_show_image(byd_fps);
				DBG_TIME("byd byd_fps_show_image end!!!\n");
				if (ret < 0) 
					image_len = 1;
				else
					image_len = ret;

				//APK界面(3201)和解锁,SETTINF ENROL界面(1)，获取到数据
				//byd_last_cmd = 10; //控制两次SHOW调用的地方不同
				if(byd_app_get_image == 1) {	//正常读取数据
					#ifdef BYD_FPS_DECODE_IMAGE
						DBG_TIME("%s:copy to user start,image_len=%d\n",__func__,image_len);
						error = byd_fps_copy_to_user(buf_u, &byd_fps->data_buf_decode[0], image_len);
						//error = byd_fps_copy_to_user(buf_u, &byd_fps->data_buf_decode[0], 7169);
					#else
						error = byd_fps_copy_to_user(buf_u, &byd_fps->data_buf_1[0], image_len);
					#endif
				} else {
					error = byd_fps_copy_to_user(buf_u, &byd_fps->data_buffer[0], image_len);
				}
				if (error != 0) {	
					DBG_TIME("%s: copy_to_user failed() return: %d \n", __func__, error);
					ret = -254;
				}
				return ret;
				//return 7169;
				
			} else {
				if(byd_show_flag == 2){
					switch(byd_enrol_match_flag){
						case 1:
							DBG_TIME("byd byd_fps_multi_enrol start!!!! \n");
							byd_fps_multi_enrol(byd_fps_show);
							DBG_TIME("byd byd_fps_multi_enrol end!!!! \n");
						break;
						
						case 2:
							DBG_TIME("byd byd_fps_multi_match start!!!\n");
							byd_fps_multi_match(byd_fps_show);
							DBG_TIME("byd byd_fps_multi_match end!!!\n");
						break;
						
						default:
							DBG_TIME("%s: switch(byd_enrol_match_flag)=%d,not fit\n", __func__, byd_enrol_match_flag);
						break;
					}
					#ifdef BYD_FPS_UP_DOWN_ASYNC
					g_need_finger_down_scan_flag = 0;
					#endif
					#ifdef BYD_FPS_REPORT_KEY
					if(byd_fps_finger_up_flag == 1){
						//byd_fps_finger_up_flag = 0;
						if(byd_fps_key_mesg == 1){
							byd_fps_report_key_up();
							byd_fps_key_mesg = 2;
						}
					}
				#endif
				#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_FOCUS))
					DBG_TIME("%s:byd_fps_alg_ret[0]=%d\n",__func__,byd_fps_alg_ret[0]);
					if (byd_fps_alg_ret[0] == 1)
						byd_fps_keydown_flag = 0;//成功则开启按键
				#endif
					byd_fps_copy_to_user(buf_u, byd_fps_show, 12);
			        DBG_TIME("%s: copy_to_user2() return: %d \n",__func__, error);
			
					//memmove(buf, byd_fps_show, 12);

					ret = 12;
					//byd_fps_finger_detect_pre_cfg(1, 0);
					#if 0//def BYD_FPS_INPUT_WAKEUP
					byd_fps_disable_irq_wake(byd_fps->spi);
					#endif
					byd_last_cmd = 0;
				}
			}
		break;
		case 19:
			byd_last_cmd = 0;
			if(ReceiVeCmdType == 4){  //读阈值 应该从寄存器里面取数 修改一下
				byd_fps_mutex_lock(1,1);
				temp = read_throld_on(byd_fps->spi);
				byd_fps_mutex_lock(2,1);
				DBG_TIME("%s:cmd19,temp value is=%d\n", __func__,temp);
				/*throldvalue_on = (65535 - temp)/4;//将按下的阈值缩小4倍上传
				DBG_TIME("%s:cmd19,throldvalue_on is=%d\n", __func__,throldvalue_on);
				byd_fps_mutex_lock(1,1);
				temp = read_throld_off(byd_fps->spi);
				byd_fps_mutex_lock(2,1);
				DBG_TIME("%s:cmd19,off temp value is=%d\n", __func__,temp);
				throldvalue_off = (65535 - temp)/4;//将抬起的阈值缩小4倍上传
				DBG_TIME("%s:cmd19,throldvalue_off is=%d\n", __func__,throldvalue_off);*/
				

				buf_k[0] = byd_fps_fae_detect_para[0];//throldvalue_on >> 8;改成上传按下系数
				buf_k[1] = byd_fps_fae_detect_para[1];//throldvalue_on & 0xff;抬起系数	
				buf_k[2] = (byd_fps_otp_th) >>8;//throldvalue_off >> 8;变化量
				buf_k[3] = (byd_fps_otp_th) & 0xff;//throldvalue_off & 0xff;		
				error = byd_fps_copy_to_user(buf_u, buf_k, 4);
				if (error != 0)	{	
					DBG_TIME("%s: copy_to_user failed() return: %d \n",__func__, error);
					return -254;
				}
				ret = 4;
			}else if(ReceiVeCmdType == 6){ //读子区域数据
				byd_fps_mutex_lock(1,1);
				byd_fps_read_finger_detect_value(byd_fps->spi, &buf_k[0]);
				byd_fps_mutex_lock(2,1);
				/*#ifdef CONFIG_FPS12
				error = byd_fps_copy_to_user(buf_u, buf_k, 18);
				#elif defined CONFIG_FPS11
				error = byd_fps_copy_to_user(buf_u, buf_k, 10);
				#endif*/
				#ifdef CONFIG_FPS22
				error = byd_fps_copy_to_user(buf_u, buf_k, 6);
				#endif
				if (error != 0) {	
					DBG_TIME("%s: copy_to_user failed() return: %d \n",__func__, error);
					return -254;
				}
				/*#ifdef CONFIG_FPS12
					ret = 18;
				#elif defined CONFIG_FPS11
					ret = 10;
                #endif	*/
				#ifdef CONFIG_FPS22
					ret = 6;
				#endif
			}else if (ReceiVeCmdType == 7) { //读版本号
				byd_chip_info=0x00;
				DBG_TIME("%s:========byd_chip_info-1 = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
				byd_fps_mutex_lock(1,1);
				byd_chip_info = byd_fps_read_version(byd_fps->spi);
				byd_fps_mutex_lock(2,1);
				DBG_TIME("%s:========byd_chip_info = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
				if((byd_chip_info&0xf8c0) == BYD_FPS_CONFIG_VERSION){
					
					#ifdef CONFIG_FPS22
					buf_k[0] = 22;
					if((byd_fps_chip_flag == 0x5531) || (byd_fps_chip_flag == 0x5532 ) ){
						buf_k[1] = BYD_FPS_SENSOR_HEIGHT_5531;
						buf_k[2] = BYD_FPS_SENSOR_WIDTH_5531;
					}else if(byd_fps_chip_flag == 0x5541){
						buf_k[1] = BYD_FPS_SENSOR_HEIGHT_5541;
						buf_k[2] = BYD_FPS_SENSOR_WIDTH_5541;
					}
					#endif 
				}
				error = byd_fps_copy_to_user(buf_u, buf_k, 3);
				if (error != 0) {	
					DBG_TIME("%s: copy_to_user failed() return: %d \n",__func__, error);
					return -254;
				}
				ret = 3;
			}else if(ReceiVeCmdType == 8){
				buf_k[0] = 0;
				if(byd_fps_ageing_test_flag == 1){//spi测试
					byd_chip_info=0x00;
					DBG_TIME("%s:========byd_chip_info-1 = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
					byd_fps_mutex_lock(1,1);
					byd_chip_info = byd_fps_read_version(byd_fps->spi);
					byd_fps_mutex_lock(2,1);
					DBG_TIME("%s:========byd_chip_info = 0x%x=======\n",__func__,byd_chip_info&0xf8c0);
				
					if((byd_chip_info&0xf8c0) == BYD_FPS_CONFIG_VERSION){
						buf_k[0] = 0x1;
					}else 
						buf_k[0] = 0x0;
					error = byd_fps_copy_to_user(buf_u, buf_k, 1);
					if (error != 0) {	
						DBG_TIME("%s: copy_to_user failed() return: %d \n",__func__, error);
						return -254;
					}
					//byd_fps_ageing_test_flag = 0;
				}else if(byd_fps_ageing_test_flag == 2){//int 测试
					byd_fps_mdelay(150);
					if(byd_fps_int_test == 0){
						
						DBG_TIME("%s:byd int come\n",__func__);
						//byd_fps_mutex_lock(1,1);
						//byd_fps_int_test_cfg_back(byd_fps->spi);
						//byd_fps_mutex_lock(2,1);
						buf_k[0] = 0x1;
					}else{
						buf_k[0] = 0x0;
					}
					byd_fps_mutex_lock(1,1);
					byd_fps_int_test_cfg_back(byd_fps->spi);
					byd_fps_mutex_lock(2,1);
					error = byd_fps_copy_to_user(buf_u, buf_k, 1);
					if (error != 0) {	
						DBG_TIME("%s: copy_to_user failed() return: %d \n",__func__, error);
						return -254;
					}
					//byd_fps_ageing_test_flag = 0;
				}else if(byd_fps_ageing_test_flag == 3){//坏点测试
					/*byd_os_scan(byd_fps);
					DBG_TIME("%s:bad point num = %d\n",__func__,bpnum);
					if(bpnum<100)
						buf_k[0] = 0x1;
					else
						buf_k[0] = 0x0;*/
					error = byd_fps_copy_to_user(buf_u, buf_k, 1);
					if (error != 0) {	
						DBG_TIME("%s: copy_to_user failed() return: %d \n",__func__, error);
						return -254;
					}
					//byd_fps_ageing_test_flag = 0;
				}
				
				byd_fps_ageing_test_flag = 0;
				ret = 1;
			}
	
			ReceiVeCmdType = 0;
			#if defined(BYD_FPS_REPORT_KEY)||defined(BYD_FPS_FOCUS)
			byd_fps_keydown_flag = 0;//进入apk测试界面，关闭按键功能，退出测试界面发送09命令打开
			#endif
		break;
		default:
			ret = -253;
		break;
	}
	FP_DBG("byd %s end, ret=%d\n", __func__, ret);
	
	DBG_TIME("byd show() end, ret=%d\n", ret);
	
	return ret;
}

static int byd_fps_release(struct inode *inode, struct file *file)
{
	pr_debug("%s: entered.\n", __func__);
	byd_fps_fasync(-1,file,0);
	return 0;
}

static long byd_fps_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	pr_debug("%s: entered, cmd=%d.\n", __func__, cmd);
	return 0;
}

/* Class END   :  byd_fps_cdev
 ******************************************************************************/

#endif // ndef BYD_FPS_SYSFS

// *******************************************************************************
// * Function    :  byd_fps_threadfn
// * Description :  sub thread, processing something, like 算法拼接、模板回写.
// * In          :  byd_fps   @ struct byd_fps_data
// * Return      :  0 -- success
// *******************************************************************************
static int byd_fps_threadfn(void *byd_thread)
{
	int ret = 0;
	struct byd_fps_thread_task *thread_task = byd_thread;

	FP_DBG("%s: run IN thread_func ...1 !!!\n ", __func__);
	while (!kthread_should_stop()) {
		FP_DBG("%s: run thread in while()...2 , thread_task->mode = %d!!!\n ", __func__, thread_task->mode);
		up(&thread_task->sem_idle);
		wait_event_interruptible(thread_task->wait_job, thread_task->mode != FPS_THREAD_MODE_IDLE);
		FP_DBG("%s: run thread in while()...3, break wait_event_interruptible(), thread_task->mode = %d!!!\n ", __func__, thread_task->mode);
		down(&thread_task->sem_idle);
		FP_DBG("%s: run switch ()4!!! thread_task->mode = %d\n ", __func__, thread_task->mode);

		switch (thread_task->mode){
			case FPS_THREAD_MODE_ALG_PROC:
				FP_DBG("%s: run in switch () 5!!! \n ", __func__);
#if (defined BYD_FPS_ALG_IN_KERNEL) || (defined BYD_FPS_ALG_IN_TZ)
				FP_DBG("%s: start merging---------\n", __func__);				
				mutex_lock(&byd_fps_mutex_alg);
				FP_DBG("%s:----------mutex in----------\n", __func__);
				ret = byd_fps_alg_back_proc();
				mutex_unlock(&byd_fps_mutex_alg);
				FP_DBG("%s: stop merging---------\n", __func__);
#endif
			break;
			case FPS_THREAD_SCAN_OS_FNGUP:

				//byd_os_update(this_byd_fps); //2017.0524.2017. 有冲突，死机，暂时关闭

			break;
			default:
			break;
		}

		FP_DBG("%s: out of switch ...7 !!!\n ", __func__);
		if(thread_task->mode != FPS_THREAD_MODE_EXIT){
			DBG_TIME("%s: set  FPS_THREAD_MODE_IDLE ... 8!!!\n ", __func__);
			thread_task->mode = FPS_THREAD_MODE_IDLE;
		} else {
			DBG_TIME("%s:   FPS_THREAD_MODE_EXIT schedule... 8-1!!!\n ", __func__);
			schedule_timeout(HZ);
			DBG_TIME("%s:   FPS_THREAD_MODE_EXIT schedule end... 8-2!!!\n ", __func__);
		}
	}
	printk("%s: exit thread func ... 9!!!\n ", __func__);
	thread_flag = 1;
	return 0;
}

// *******************************************************************************
// * Function    :  byd_fps_cleanup
// * Description :  unregister.
// * In          :  byd_fps   @ struct byd_fps_data
// * Return      :  0 -- success
// *******************************************************************************
static int byd_fps_cleanup(struct byd_fps_data *byd_fps)
{
	FP_DBG("%s: IN\n", __func__);
#if 0
	if(!IS_ERR_OR_NULL(byd_fps->device))
		device_destroy(byd_fps->class, byd_fps->devno);
	class_destroy(byd_fps->class);
#endif

	#ifndef BYD_FPS_SYSFS
		byd_fps_cdev_distr(fps_cdev);
	#endif
	if(byd_fps->data_buffer) {
		free_pages((unsigned long)byd_fps->data_buffer, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	}
	if(byd_fps->data_buf_1) {
		free_pages((unsigned long)byd_fps->data_buf_1, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	}

	byd_os_exit(byd_fps);

	#ifdef BYD_FPS_DECODE_IMAGE
	if(byd_fps->data_buf_decode) {
		free_pages((unsigned long)byd_fps->data_buf_decode, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	}
	#endif

	#ifdef BYD_FPS_MISC_NAME
		misc_deregister(&byd_fps_device); //
	#endif

	//kfree(byd_fps);
	kfree(thread_task);
	kfree(byd_fps->waiting_interrupt_return);

	return 0;
}

// *******************************************************************************
// * Function    :  byd_fps_remove
// * Description :  remove variable, unregister .
// * In          :  byd_fps   @ struct byd_fps_data   @struct spi_device *spi
// * Return      :  none
// *******************************************************************************
void byd_fps_remove(struct byd_fps_data *byd_fps,struct spi_device *spi)
{
	int ret;
	ret = 0;
	byd_fps = this_byd_fps;
	byd_fps ->spi = spi;
	//退出算法
#ifdef BYD_FPS_ALG_IN_TZ
	if(qsee_run == 1){
		ret = fp_qsee_stop();
		FP_DBG("%s: byd call fp_ qsee_stop()\n", __func__);
		qsee_run = 0;
	}
#endif
	//退出算法
#ifdef BYD_FPS_ALG_IN_KERNEL
	byd_fps_alg_remove();//释放算法内存
	DBG_TIME("byd_fps_alg_remove out!\n");
#endif
#ifdef BYD_FPS_INPUT_WAKEUP
	byd_fps_wakeup_exit(&byd_fps->spi->dev);
	DBG_TIME("byd_fps_wakeup_exit out!\n");
#endif
#ifdef BYD_FPS_GESTURE
	byd_fps_gest_exit(&byd_fps->spi->dev);
#endif
DBG_TIME("byd_fps_gest_exit out!\n");
#if defined(BYD_FPS_TIMER_KEY)||defined(BYD_FPS_FOCUS)
	byd_fps_timer_exit(&byd_fps->spi->dev);
#endif
DBG_TIME("byd_fps_timer_exit out!\n");
	//退出内核子线程
	if (!IS_ERR(thread_task->thread) && (!thread_flag)){
		DBG_TIME("byd_fps_thread_exit in!\n");
		thread_task->mode = FPS_THREAD_MODE_EXIT;
		ret = kthread_stop(thread_task->thread);
		DBG_TIME(KERN_INFO "First thread function has stopped ,return %d\n", ret);
	}	

	DBG_TIME("byd_fps_thread_exit out!\n");
	//清除驱动
	byd_fps_cleanup(byd_fps);


	printk("%s, exit\n", __func__);
}
// *******************************************************************************
// * Function    :  byd_fps_alg_para_init
// * Description :  initialize the memory&files of ALGORITHM
// * In          :  void
// * Return      :  0 -- success,
// * 
// *******************************************************************************
#ifdef BYD_FPS_ALG_IN_KERNEL
//static int byd_fps_alg_para_init(void)
int byd_fps_alg_para_init(void)
{

	//设置模板路径
	byd_fps_set_template_path(byd_fps_template_path);//设置模板路径
	DBG_TIME("%s: byd_fps_SetTemplatePath : %s\n",__func__, byd_fps_template_path);
	
	
	
	//算法参数初始化值设置
	//设置指纹模板最多支持的手指ID，及每个ID包含的最多模板数
#ifdef CONFIG_ALG_NUM_PARA
	byd_fps_set_template(CONFIG_ALG_MAX_NUM_FINGERS, CONFIG_ALG_NUM_TEMPLATES_PER_FENGER, CONFIG_ALG_MAX_NUM_TEMPLATES, CONFIG_ALG_SAME_FJUDGE);
	DBG_TIME("%s: set template ,CONFIG_ALG_MAX_NUM_FINGERS -%d , CONFIG_ALG_NUM_TEMPLATES_PER_FENGER - %d\n",
			__func__,CONFIG_ALG_MAX_NUM_FINGERS,CONFIG_ALG_NUM_TEMPLATES_PER_FENGER);
#endif
	//设置sensor有效面积
#ifdef CONFIG_ALG_AREA_PARA
	byd_fps_set_sensor_area(CONFIG_ALG_SENSOR_VALID_ENROLL_AREA, CONFIG_ALG_SENSOR_INVALID_ENROLL_BLOCK,
							CONFIG_ALG_SENSOR_VALID_MATCH_AREA, CONFIG_ALG_SENSOR_INVALID_MATCH_BLOCK,
							CONFIG_ALG_SENSOR_INVALID_VARIANCE,CONFIG_ALG_SENSOR_MOVEXY,CONFIG_ALG_SENSOR_CUTMETHOD);
	DBG_TIME("%s: byd_fps_set_sensor_area\n",__func__);
#endif
	//设置安全级别
#ifdef  CONFIG_ALG_SAFE_PARA
	byd_fps_safe_level_get_n_set(CONFIG_ALG_SAFE_LEVEL,CONFIG_ALG_SAFE_ONF);
#endif
	return 0;
}
#endif
// *******************************************************************************
// * Function    :  byd_fps_probe
// * Description :  initiate variable, port, chip status. Called by kernel when load driver
// * In          :  spi   @ struct spi_device
// * Return      :  struct byd_fps_data
// *******************************************************************************
struct byd_fps_data *byd_fps_probe(struct spi_device *spi)
{
	int ret = -1;
	struct byd_fps_data  *byd_fps = NULL;
	
	DBG_TIME("%s, entered\n", __func__);

	DBG_TIME("%s: name:%s, bus_num:%d, num_slave:%d, chip_select:%d, irq:%d \n",
			__func__, spi->modalias, spi->master->bus_num, spi->master->num_chipselect,
			 spi->chip_select, spi->irq);

	/* --- memory allocation ------------------------------------------------ */

	byd_fps = kzalloc(sizeof(*byd_fps), GFP_KERNEL);
	if (!byd_fps) {
		dev_err(&spi->dev, "failed to allocate memory for struct byd_fps_data\n");
		ret = -ENOMEM;
		goto err_out;
	}
	DBG_TIME("%s, byd fps data kzalloc OK\n", __func__);

	thread_task = kzalloc(sizeof(*thread_task), GFP_KERNEL);
	if (!thread_task) {
		dev_err(&spi->dev, "failed to allocate memory for thread_task\n");
		ret = -ENOMEM;
		goto exit1_byd_fps;
	}
	DBG_TIME("%s, kzalloc thread_task OK\n", __func__);

	byd_fps->waiting_interrupt_return = kzalloc(sizeof(*byd_fps->waiting_interrupt_return), GFP_KERNEL);
	if (!byd_fps->waiting_interrupt_return) {
		dev_err(&spi->dev, "failed to allocate memory for waiting_interrupt_return\n");
		ret = -ENOMEM;
		goto exit2_thread;
	}
	DBG_TIME("%s, kzalloc waiting_interrupt_return OK\n", __func__);

	/*byd_fps->data_buffer = (u8 *)__get_free_pages(GFP_KERNEL, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	if (!byd_fps->data_buffer) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->data_buffer, 0xFF, BYD_FPS_IMAGE_BUFFER_SIZE);
	#ifdef BYD_FPS_ALG_IN_KERNEL
	byd_fps_image_data = (unsigned short*)&byd_fps->data_buffer[1];
	#endif
	DBG_TIME("%s, byd get_free_pages OK\n", __func__);
	
	byd_fps->data_buf_1 = (u8 *)__get_free_pages(GFP_KERNEL, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	if (!byd_fps->data_buf_1) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->data_buf_1, 0xFF, BYD_FPS_IMAGE_BUFFER_SIZE);
	DBG_TIME("%s, byd get_free_pages buf_1 OK\n", __func__);

	//#ifdef BYD_FPS_DECODE_IMAGE
	byd_fps->data_buf_decode = (u8 *)__get_free_pages(GFP_KERNEL, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	if (!byd_fps->data_buf_decode) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->data_buf_decode, 0x00, BYD_FPS_IMAGE_BUFFER_SIZE);
	DBG_TIME("%s, byd get_free_pages buf_decode OK\n", __func__);*/
	//#endif
	#ifdef BYD_FPS_FOCUS
	byd_fps->left_right_state = (u8 *)__get_free_pages(GFP_KERNEL, get_order(100));
	if (!byd_fps->left_right_state) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->left_right_state, 0x00, 100);
	DBG_TIME("%s, byd get_free_pages left_right_state OK\n", __func__);
	byd_fps->up_down_state = (u8 *)__get_free_pages(GFP_KERNEL, get_order(100));
	if (!byd_fps->up_down_state) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->up_down_state, 0x00, 100);
	DBG_TIME("%s, byd get_free_pages up_down_state OK\n", __func__);
	#endif

	byd_fps->spi = spi;
	this_byd_fps = byd_fps; //global point
	
	/* --- chip initialization ---------------------------------------------- */

	ret = byd_fps_config_sensor_poweron(spi);
	if (ret != 0) {
		dev_err(&spi->dev, "byd_fps_config_sensor_poweron failed.\n");
		goto exit4_buffer;
	}
	DBG_TIME("%s: Sean Debug --byd_fps_config_sensor_poweron OK!\n",__func__);
	
	if( byd_fps->data_buffer == NULL ) {
		byd_fps->data_buffer = (u8 *)__get_free_pages(GFP_KERNEL, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
		if (!byd_fps->data_buffer) {
			dev_err(&spi->dev, "failed to get free pages\n");
			ret = -ENOMEM;
			goto exit3_wait_int;
		}
		memset(byd_fps->data_buffer, 0xFF, BYD_FPS_IMAGE_BUFFER_SIZE);
	}
	#ifdef BYD_FPS_ALG_IN_KERNEL
	byd_fps_image_data = (unsigned short*)&byd_fps->data_buffer[1];
	#endif
	DBG_TIME("%s, byd get_free_pages OK\n", __func__);
	
	byd_fps->data_buf_1 = (u8 *)__get_free_pages(GFP_KERNEL, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	if (!byd_fps->data_buf_1) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->data_buf_1, 0xFF, BYD_FPS_IMAGE_BUFFER_SIZE);
	DBG_TIME("%s, byd get_free_pages buf_1 OK\n", __func__);

	//#ifdef BYD_FPS_DECODE_IMAGE
	byd_fps->data_buf_decode = (u8 *)__get_free_pages(GFP_KERNEL, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	if (!byd_fps->data_buf_decode) {
		dev_err(&spi->dev, "failed to get free pages\n");
		ret = -ENOMEM;
		goto exit3_wait_int;
	}
	memset(byd_fps->data_buf_decode, 0x00, BYD_FPS_IMAGE_BUFFER_SIZE);
	DBG_TIME("%s, byd get_free_pages buf_decode OK\n", __func__);
	
	
	byd_fps->interrupt_done = 0;
	thread_task->should_stop = 0;
	//byd_fps->avail_data = 0;
	byd_fps->interrupt_done = 0;
	set_timeout_finger_down = BYD_FPS_DEFAULT_IRQ_TIMEOUT;
	byd_app_get_image = 0;
	byd_last_cmd = 0;
	byd_fps_susflag =1;
	#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_FOCUS))
	byd_fps_keydown_flag = 0;//默认打开按键功能
	#endif
	byd_fps_wait_flag = 0;
	
#ifdef BYD_FPS_SYSFS	
	//transfer number when APP read
	//trans_num = BYD_FPS_IMAGE_SIZE/BUF_TRANS_SIZE;
	//trans_rest = BYD_FPS_IMAGE_SIZE%BUF_TRANS_SIZE;
	trans_num = BYD_FPS_IMAGE_WIDTH*BYD_FPS_IMAGE_HEIGHT*2/BUF_TRANS_SIZE;
	trans_rest = BYD_FPS_IMAGE_WIDTH*BYD_FPS_IMAGE_HEIGHT*2%BUF_TRANS_SIZE;
	if(trans_rest > 0) {
		trans_num += 1;
	}
	read_first = trans_num;
#endif

#ifdef BYD_FPS_ALG_IN_KERNEL
	byd_fps_add_txt = 0;
#elif defined(BYD_FPS_ALG_IN_TZ)
	qsee_run = 0;
#endif
	mutex_init(&byd_fps_mutex_chip);
	mutex_init(&byd_fps_mutex_alg);
	init_waitqueue_head(byd_fps->waiting_interrupt_return);
	init_waitqueue_head(&thread_task->wait_job);
	sema_init(&thread_task->sem_idle, 0); // used in byd_fps_threadfn
	thread_task->mode = FPS_THREAD_MODE_IDLE;
	thread_task->thread = kthread_run(byd_fps_threadfn, thread_task, "%s", BYD_FPS_WORKER_THREAD_NAME);
	if ((ret=IS_ERR(thread_task->thread))) {
		dev_err(&byd_fps->spi->dev, "kthread_run failed.\n");
		thread_task->mode = FPS_THREAD_MODE_EXIT;
        thread_task->thread = NULL;
		goto exit4_buffer;
	}
	DBG_TIME("%s: kthread_run end\n", __func__);
#ifdef BYD_FPS_GESTURE
	byd_fps_gest_init(&spi->dev);
#endif
#if defined(BYD_FPS_TIMER_KEY)||defined(BYD_FPS_FOCUS)
	byd_fps_timer_init(&spi->dev);
#endif
#ifdef BYD_FPS_SYSFS
	ret = device_create_file(&spi->dev, &dev_attr_fp_qsee);
	if (ret) 
		dev_err(&spi->dev, "%s creation failed\n", __func__);
	
	DBG_TIME("%s: device_creat dev_attr_fp_qsee end\n", __func__);
#else
	fps_cdev = byd_fps_cdev_constr(); // *** qiximing ***
#endif

#ifdef BYD_FPS_MISC_NAME
	DBG_TIME("%s: misc_register()\n", __func__);
	ret = misc_register(&byd_fps_device); //maybe delete?
	if (ret)
		goto exit7_thread_stop;
	DBG_TIME("misc_register()=%d\n", ret);
#endif

//early suspend init
#ifdef BYD_FPS_INPUT_WAKEUP
	byd_fps_wakeup_init(&spi->dev);
#endif
	//初始化算法
	ret = 0;
#if BYD_SH_METHOD_SEL==2//主动式的算法初始化

#ifdef BYD_FPS_ALG_IN_KERNEL
	//设置版本号
	FP_DBG("%s:go to version\n",__func__);
	ret = byd_fps_alg_version_init();	//no right key ,back error 
	if(ret < 0)
		goto exit4_buffer;
	FP_DBG("%s:the alg version : %s\n",__func__,byd_fps_alg_ret);

	byd_fps_alg_para_init();
#endif //  BYD_FPS_ALG_IN_KERNEL
	//==============read os data========================
	byd_os_init(this_byd_fps);
#endif

	return byd_fps;


#ifdef BYD_FPS_INPUT_WAKEUP

	byd_fps_wakeup_exit(&byd_fps->spi->dev);
#endif

#ifdef BYD_FPS_MISC_NAME
exit7_thread_stop:
	DBG_TIME("%s: byd misc_deregister() \n",__func__);
	misc_deregister(&byd_fps_device);

#endif


	if (thread_task->thread) {
		thread_task->mode = FPS_THREAD_MODE_EXIT;
		wake_up_interruptible(&thread_task->wait_job);
        DBG_TIME("%s: byd kthread_stop() \n",__func__);
		kthread_stop(thread_task->thread);
		DBG_TIME("%s: byd kthread_stop()  end\n",__func__);
        thread_task->thread = NULL;
    }
exit4_buffer:
	if (byd_fps->data_buffer) {
		DBG_TIME("%s: byd free_pages() \n",__func__);
		free_pages((unsigned long)byd_fps->data_buffer, get_order(BYD_FPS_IMAGE_BUFFER_SIZE * BYD_FPS_MAX_FRAME));
	}

exit3_wait_int:
	kfree(byd_fps->waiting_interrupt_return);
exit2_thread:
	if (thread_task)
		kfree(thread_task);

exit1_byd_fps:
	DBG_TIME("%s: byd kfree(this_byd_fps) \n",__func__);
	if (this_byd_fps)
		kfree(this_byd_fps);
	
	DBG_TIME("%s: byd destroyed DONE\n",__func__);

err_out:
	return ERR_PTR(ret);
}



// ============================================================================
// implementation of kernel dependent functions for byd_fps_libbf663x.c
// ===============================================================================

// *******************************************************************************
// * Function    :  byd_fps_check_thread_should_stop
// * Description :  查询是否需要终止当前的进程.
// * In          :  void
// * Return      :  0 -- success, -4 -- 被中断,被取消.
// *******************************************************************************
int byd_fps_check_thread_should_stop(void) {
	FP_DBG("%s: should stop=%d\n" ,__func__, thread_task->should_stop);
	if(thread_task->should_stop) {
		pr_err("%s: thread_task->should_stop\n", __func__);
		thread_task->should_stop = 0;
		return -EINTR;
	}
	return 0;
}

// *******************************************************************************
// * Function    :  byd_fps_wait_event_interruptible_timeout
// * Description :  主动终止还在运行中的进程,以便执行新的进程.
// * In          :  timeout
// * Return      :  0 -- 直到时间耗完前没有被打断(本线程未被唤醒). >0 -- 被中断(唤醒).
// *                -ERESTARTSYS -- 信号被中断
// *******************************************************************************
long byd_fps_wait_event_interruptible_timeout(long timeout)
{
	long ret;
	DBG_TIME("%s: IN\n", __func__);
	DBG_TIME("this_byd_fps->interrupt_done=%d befor\n", this_byd_fps->interrupt_done);
	this_byd_fps->interrupt_done = 0;
	DBG_TIME("this_byd_fps->interrupt_done=%d ,%d present\n", this_byd_fps->interrupt_done,this_byd_fps->waiting_interrupt_return);
	byd_fps_wait_flag = 1;
	ret = wait_event_interruptible_timeout(*this_byd_fps->waiting_interrupt_return, this_byd_fps->interrupt_done, timeout);
	this_byd_fps->interrupt_done = 0;
	DBG_TIME("%s: END\n", __func__);
	return ret;
}

// *******************************************************************************
// * Function    :  byd_fps_stop_thread
// * Description :  主动终止还在运行中的进程,以便执行新的进程.
// * In          :  work_cmd
// * Return      :  void.
// *******************************************************************************
void byd_fps_stop_thread(u8 work_cmd)
{
	if (work_cmd) {

		thread_task->should_stop = 1;
		this_byd_fps->interrupt_done = 1;
		wake_up_interruptible(this_byd_fps->waiting_interrupt_return);
		FP_DBG("%s: Sean_Debug - should_stop = 1 \n", __func__);
	}

}

// *******************************************************************************
// * Function    :  byd_fps_start_template_merging
// * Description :  启动子线程,执行算法后处理(模板拼接、模板回写).
// * In          :  void
// * Return      :  void.
// *******************************************************************************
void byd_fps_start_template_merging(void) 
{
	thread_task->mode = FPS_THREAD_MODE_ALG_PROC;
	wake_up_interruptible(&thread_task->wait_job);
}

void byd_os_start_scan(void)
{
	thread_task->mode = FPS_THREAD_SCAN_OS_FNGUP;
	wake_up_interruptible(&thread_task->wait_job);
}

// *******************************************************************************
// * Function    :  byd_fps_udelay
// * Description :  延时函数，微秒级别.阻塞方式.
// * In          :  void
// * Return      :  void.
// *******************************************************************************
void byd_fps_udelay(unsigned int usecs) {
	udelay(usecs);
};

// *******************************************************************************
// * Function    :  byd_fps_mdelay
// * Description :  延时函数，毫秒级别.阻塞方式.
// * In          :  void
// * Return      :  void.
// *******************************************************************************
void byd_fps_mdelay(unsigned int msecs) {
	mdelay(msecs);
};
// *******************************************************************************
// * Function    :  byd_fps_disable_irq
// * Description :  中断使能关闭.
// * In          :  void
// * Return      :  void.
// *******************************************************************************
void byd_fps_disable_irq(struct spi_device *spi)
{
	disable_irq(spi->irq);
}
// *******************************************************************************
// * Function    :  byd_fps_enable_irq
// * Description :  中断使能关闭.
// * In          :  void
// * Return      :  void.
// *******************************************************************************
void byd_fps_enable_irq(struct spi_device *spi)
{
	enable_irq(spi->irq);
}
// *******************************************************************************
// * Function    :  byd_fps_msleep
// * Description :  延时函数，毫秒级别.非阻塞方式,会进行线程切换.
// * In          :  void
// * Return      :  void.
// *******************************************************************************
void byd_fps_msleep(unsigned int msecs) {
	msleep(msecs);
};

// ============================================================================
// end of implementation of kernel dependent functions for byd_fps_libbf663x.c
// ============================================================================


/*
 * File:         byd_alg_kernel_implement.c
 *
 * Created:	     2015-06-26
 * Called by:    byd_algorithm.c
 * Description:  the kernel implementation of BYD fingerprint algorithm encapsulation
 *
 * Copyright (C) 2015 BYD Company Limited
 *
 * Contact: qi.ximing@byd.com;
 *
 */


/* kernel head files must be included here */

// 2015.8.18 zhuxiang modify, change from kzalloc() to vmalloc()+memset() for MTK6735
//#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10 
#include <linux/vmalloc.h> // vmalloc(), vfree()
//#else
#include <linux/slab.h>    // kzalloc(), kfree()
//#endif
#include <linux/fs.h>      // O_RDONLY, O_RDWR, O_APPEND, SEEK_CUR
#include <asm/uaccess.h>   // get_fs(), set_fs()
#include <linux/kmod.h>    // call_usermodehelper(), UMH_WAIT_PROC

#include "byd_algorithm.h"  // FILE


/*******************
 * kernel file IO API
 *******************/
/*
struct file *filp_open(const char *filename, int flags, int mode)
int filp_close(struct file *filp, fl_owner_t id)
loff_t vfs_llseek(struct file *file, loff_t offset, int whence)
ssize_t vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
ssize_t vfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
*/

#include <linux/fs.h>    // struct file, filp_open(), filp_close()

FILE *byd_fopen(char *path, char *type)
{
    FILE *fp = NULL;
    int flags = 0;

    if (path == NULL) {
        pr_err("byd_fopen(): unknown file path!\n");
        return NULL;
    }

    //pr_debug("byd_fopen(%s, %s)", path, type);
    if (strchr(type, 'b') == NULL || strchr(type, 't') != NULL) {
        pr_debug("byd_fopen(): the file shuld be opened binary (with 'b' specified)!");
    }
    if (strchr(type, '+') != NULL) { // (w+), +(r+), (a+)
        flags |= O_RDWR;
    }
    if (strchr(type, 'a') != NULL) { // (a), (a+), (ar)
        flags |= O_APPEND;
    } else if (strchr(type, 'w') != NULL) {
        if (strchr(type, '+') == NULL && strchr(type, 'r') == NULL) { // (w)
            flags |= O_WRONLY; // use O_RDWR if O_WRONLY is not supported
        }
        flags |= O_TRUNC; // (w), (w+),
    }
    if (strchr(type, 'w') != NULL || strchr(type, 'a') != NULL) {
        if (strchr(type, 'r') != NULL) { // && strchr(type, '+') == NULL //wr, (ar)
            flags |= O_RDWR;
        }
        flags |= O_CREAT; // (a), (w)
    }
    if (strchr(type, 'r') != NULL && strchr(type, '+') == NULL && strchr(type, 'w') == NULL && strchr(type, 'a') == NULL) { // r
        flags |= O_RDONLY; // O_RDONLY is 0
    } else if (flags == 0) {
        pr_err("byd_fopen(): unknown file type!\n");
        return NULL;
	} else {
        flags |= O_SYNC;
    }

    fp = filp_open(path, flags, S_IRWXU|S_IRWXG|S_IRWXO); // owner rights: RWX
    
	//DBG_TIME("Warning: byd_fopen(): filp_open() FAILED: %d,%p", *fp, fp);
    if (IS_ERR(fp)) {
     // 2015-9-2 qiximing delete: pr_warn() call causes serious time delay (about 10ms) in kernel
      //  pr_err("Warning: byd_fopen(): filp_open() FAILED: %s", path);
     
        return NULL;
    }

    return fp;
}

int byd_fclose(FILE *fp)
{
    //pr_debug("byd_fclose(): entered");
    return filp_close(fp, NULL);
}

int byd_fseek(FILE *fp, long offset, int fromwhere)
{
    int ret = -1;
    mm_segment_t old_fs;

    if (fp->f_op == NULL) {
        pr_err("byd_fseek(): fp->f_op is not defined!\n");
        return -1;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    if (fp->f_op->llseek) {
        ret = fp->f_op->llseek(fp, offset, fromwhere);
    } else {
        pr_warning("byd_fseek(): set offset from start of file SEEK_SET");
        fp->f_pos = offset;
        ret = offset;
    }

    set_fs(old_fs);

    return ret;
}

long byd_ftell(FILE *fp)
{
    long ret = -1;
    mm_segment_t old_fs;

    if (fp->f_op == NULL) {
        pr_err("byd_ftell(): fp->f_op is not defined!\n");
        return -1;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    if (fp->f_op->llseek) {
        ret = fp->f_op->llseek(fp, 0, SEEK_CUR);
    } else {
        pr_err("byd_ftell(): fp->f_op->llseek is not defined\n!");
    }

    set_fs(old_fs);

    return ret;
}

int byd_fread(void *buf, int size, int count, FILE *fp)
{
    int ret = 0;
    mm_segment_t old_fs;

    if (fp->f_op == NULL) {
        pr_err("byd_fread(): fp->f_op is not defined!\n");
        return -1;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    if (fp->f_op->read) {
        ret = fp->f_op->read(fp, (char *)buf, size*count, &fp->f_pos);
        //pr_debug("byd_fread(), size*count:%d, ret=%d", size*count, ret);
        if(ret <= 0) {
            pr_err("byd_fread(): fp->f_op->read() FAILED! ret = %d\n", ret);
        }
    } else {
        pr_err("byd_fread(): fp->f_op->read is not defined!\n");
    }

    set_fs(old_fs);

    return ret;
}

int byd_fwrite(void *buf, int size, int count, FILE *fp)
{
    int ret = 0;
    mm_segment_t old_fs;

    if (fp->f_op == NULL) {
        pr_err("byd_fwrite(): fp->f_op is not defined!\n");
        return -1;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);

    if (fp->f_op->write) {
        ret = fp->f_op->write(fp, (const char *)buf, size*count, &fp->f_pos);
        //pr_debug("byd_fwrite(), size*count:%d, ret=%d", size*count, ret);
        if(ret != size*count) {
            pr_err("byd_write(): fp->f_op->write() FAILED! ret = %d\n", ret);
        }
    } else {
        pr_err("byd_fwrite(): fp->f_op->write is not defined!\n");
    }

    set_fs(old_fs);

    return ret;
}

int byd_remove(char *path)
{
    int ret = 0;
    char cmd_path[] = "/bin/rm";//ingenic
    char* cmd_argv[] = {cmd_path, path, NULL};
    //char* cmdEnvp[] = {"HOME=/", "PATH=/sbin:/bin:/usr/bin", NULL};

    //pr_debug("%s: %s %s", __func__, cmd_argv[0], cmd_argv[1]);
    ret = call_usermodehelper(cmd_path, cmd_argv, NULL, UMH_WAIT_PROC);
    if (ret < 0) {
        pr_err("%s: could not remove file %s, errno: %d\n", __func__, path, ret);
    }
    return ret;
}



int byd_rename(char *s_file, char *d_file)
{
    int ret = 0;
	char cmd_path[] = "/system/bin/mv";
	char *cmd_argv[] = {cmd_path, s_file, d_file, NULL};
	//char* cmdEnvp[] = {"HOME=/", "PATH=/sbin:/bin:/usr/bin", NULL};

    //pr_debug("%s: %s %s %s\n", __func__, cmd_argv[0], cmd_argv[1], cmd_argv[2]);
	ret = call_usermodehelper(cmd_path, cmd_argv, NULL, UMH_WAIT_PROC);
	if (ret < 0) {
		pr_err("%s: could not rename file %s as %s, errno: %d\n", __func__, s_file, s_file, ret);
	}

	return ret;
}

int byd_feof(FILE *fp)
{
    int ret = 0;
    char buf;

    ret = byd_fread(&buf, 1, 1, fp);
    //pr_debug("byd_feof(): byd_fread(), buf=0x%x, ret=%d", buf, ret);
    if(ret <= 0) {
      return 1;
    }

    ret = byd_fseek(fp, -1, SEEK_CUR);
    if(ret < 0) {
      pr_err("byd_feof(): byd_fseek() FAILED! ret = %d\n", ret);
    }
    return 0;
}


/*****************
 * Kernel Heap API
 *****************/
/* kernel heap prototype
void *kzalloc(size_t size, gfp_t flags)
void kfree(const void *buf)
*/

void *byd_malloc(unsigned int size)
{
    void *buf = NULL;
    //pr_debug("byd_malloc() entered, size=%d", size);

    // 2015.8.18 zhuxiang modify, change from kzalloc() to vmalloc()+memset() for MTK6735
//#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10 
//	buf = vmalloc(size);
//#else
	DBG_TIME("byd_malloc(): kzalloc(size) size=%d \n", size);
    buf = kzalloc((size_t)size, GFP_KERNEL);
	
//#endif
    if (buf == NULL) {
		pr_warn("byd_malloc(): kzalloc(size) FAILED! size=%d \n", size);
		buf = vmalloc(size);
	    if (buf == NULL) 
			pr_err("byd_malloc():  vmalloc(size) FAILED! size=%d \n", size);
	}
	
    return buf;
}

void byd_free(void *buf)
{
    if(buf != NULL) {
        // 2015.8.18 zhuxiang modify, change from kfree() to vfree() for MTK6735
#if defined PLATFORM_MTK || defined PLATFORM_MTK_KERNEL3_10
		vfree(buf);
#else
        kfree(buf);
#endif
        buf = NULL;
    } else {
        pr_err("byd_free(buf): kfree(buf) FAILED! buf = NULL\n");
    }
    return;
}


/************************
 * String-int convertion
 ************************/
//#include <stdlib.h>
/*
 * str    : string to be converted
 * output : the interger of first data in the str
 */
int atoi(const char* str)
{
    int result = 0;
    int sign = 0;

    if (str == NULL) return 0;

    // proc whitespace characters
    while (*str==' ' || *str=='\t' || *str=='\n')
        ++str;

    // proc sign character
    if (*str=='-') {
        sign = 1;
        ++str;
    } else if (*str=='+') {
        ++str;
    }

    // proc numbers
    while (*str>='0' && *str<='9') {
        result = result*10 + *str - '0';
        ++str;
    }

    // return result
    if (sign == 1)
       return -result;
    else
       return result;
}

/*
 * value : integer to be converted
 * buf   : converted string
 * radix : power for integer
 */
char *itoa(int val, char *buf, unsigned int radix)
{
    char   *p;
    char   *firstdig;
    char   temp;
    unsigned   digval;

    p = buf;
    if (val < 0) {
        *p++ = '-';
        val = (unsigned long)(-(long)val);
    }

    firstdig = p;
    do {
        digval = (unsigned int)(val % radix);
        val /= radix;

        if (digval > 9)
            *p++ = (char)(digval - 10 + 'a');
        else
            *p++ = (char)(digval + '0');
    } while (val > 0);

    *p-- = '\0';
    do {
        temp = *p;
        *p = *firstdig;
        *firstdig = temp;
        --p;
        ++firstdig;
    } while (firstdig < p);

    return buf;
}


// *******************************************************************************
// * Function    :  byd_fps_write_file
// * Description :  将数组写入指定路径下的文件中.若路径不存在,则写入失败.
// *             :  文件不存在,则先创建文件.
// * In          :  void
// * Return      :  void.
// *******************************************************************************
int byd_fps_write_file(char *p_path, char *buf, int len, int offset)
{
	FILE *fp=NULL;
	if(p_path == NULL) {
		return -1;
	}
	fp = byd_fopen(p_path, "wba");
	if(fp!=NULL) {
		byd_fwrite(buf, 1, len, fp);

		byd_fclose(fp);
		return 0;
	} else {
		pr_err("%s:fopen failed\n",__func__);
		return -1;
	}

}

int byd_fps_read_file(char *p_path, char *buf, int len, int offset)
{
	FILE *fp=NULL;
	if(p_path == NULL) {
		return -1;
	}	
	fp = byd_fopen(p_path, "rba");
	if(fp!=NULL) {
		byd_fseek(fp, offset, SEEK_CUR);
		byd_fread(buf, 1, len, fp);
		byd_fclose(fp);
		return 0;
	} else {
		return -1;
	}

}
