/*
 * File:         byd_fps_wakeup.c
 *
 * Created:	     2015-11-22
 * Description:  BYD Fingerprint IC driver for Android
 *
 * Copyright (C) 2015 BYD Company Limited
 *
 * Licensed under the GPL-2 or later.
 *
 * History:
 *
 * Contact: qi.ximing@byd.com;
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
#include "BF5326.h"


/* ======================== public global constant ========================= */
extern const unsigned int BYD_FPS_IMAGE_SIZE;
//extern const unsigned char BYD_FPS_16BIT_IMAGE;
extern unsigned char byd_fps_fg_det_state;
extern u8 work_cmd;
extern struct byd_fps_thread_task *thread_task;
extern struct mutex byd_fps_mutex_chip;
extern struct mutex byd_fps_mutex_alg;
extern unsigned char byd_fps_instr_stop_flag;	//中断停止标志.
extern unsigned char byd_fps_int_test;//工厂测试标志？？
#if (defined BYD_FPS_ALG_IN_KERNEL) || (defined BYD_FPS_ALG_IN_TZ)
extern unsigned char byd_fps_alg_ret[6];
extern unsigned int byd_fps_match_id;
#endif

#ifdef BYD_FPS_ALG_IN_KERNEL
extern unsigned char byd_fps_add_txt;//是否加载模板到内存
extern unsigned short *byd_fps_image_data;
#endif

extern unsigned char byd_last_cmd;


//extern unsigned char finger_intr_ctrl;
extern const unsigned char FINGER_DOWN;
extern const unsigned char FINGER_UP;
//extern const unsigned char byd_fps_fae_detect[6];
//extern unsigned char byd_fps_fae_detect_temp[6];
extern unsigned char finger_present;
extern unsigned long set_timeout_finger_down;
extern unsigned long set_timeout_finger_up;

extern unsigned char byd_fps_sub_value[6];
extern struct byd_fps_data  *this_byd_fps;
struct input_dev *byd_input_dev;
unsigned char byd_fps_gesture_suspend = 0;
unsigned char byd_fps_susflag = 0;
//#ifdef BYD_FPS_REPORT_KEY
#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_FOCUS))
extern unsigned int byd_fps_keydown_flag;
extern unsigned int byd_fps_keyopen_flag;
#endif

#ifdef BYD_FPS_TIMER_KEY
unsigned char byd_fps_get_timer_flag(void);
void byd_fps_downif(void);
#endif
unsigned char byd_fps_finger_up_flag = 0;
#ifdef BYD_FPS_REPORT_KEY_FOCUS_CONTROLLED
unsigned char byd_fps_key_func_flag = 0;
#endif
#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_REPORT_KEY_FOCUS_CONTROLLED))
unsigned char byd_fps_key_mesg = 0;
#endif
#ifdef BYD_FPS_FASYNC
 void byd_fps_send_fasync(void);
#endif
#ifdef BYD_FPS_FOCUS
void byd_fps_timer_up_stop(void);
extern unsigned char gest_int_number;
#endif

//#if (defined(BYD_FPS_REPORT_KEY))&&(defined(BYD_FPS_REPORT_KEY_FOCUS_CONTROLLED))
#ifdef BYD_FPS_GEST
unsigned char byd_fps_gest_key_flag = 0;
#endif

/***  Module    :  byd_os  ***/
extern unsigned char Os_Scan_Expired;
extern unsigned char Os_Otp_or_Scan;
/***  Module end:  byd_os  ***/
/**
 * struct byd_fps_wakeup_data_t - system wakeup data
 * @early_suspend:	use Android's early suspend
 * @fb_notif:	use FB if configured
 *
 */
 
struct byd_fps_wakeup_data_t
{
  #if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_suspend;
  #elif defined(CONFIG_FB)
	struct notifier_block fb_notif;
  #endif
  	struct workqueue_struct *byd_fps_workqueue_key;
	struct work_struct byd_fps_work_key;
	
	//struct __wait_queue_head *waiting_finger_state; // wait_queue_head_t
	//int finger_state_done;
	
	struct workqueue_struct *byd_fps_workqueue_susp;
	struct work_struct byd_fps_work_susp;
	
	
	
	//struct input_dev *byd_input_dev;
	unsigned char byd_fps_wakeup_lock;//指纹芯片能否休眠唤醒的开关
	unsigned char byd_fps_suspend_flag;//休眠标志
	unsigned char byd_fps_susp_match_cnt;
	unsigned char byd_fps_suspend_qual;
};


struct byd_fps_wakeup_data_t *byd_fps_wakeup_data = NULL;

extern void byd_fps_mutex_lock(unsigned char lock_flag,unsigned char type_flag);

#ifdef BYD_FPS_TIMER
#ifdef BYD_TIMER_FNG_UP
int byd_fps_upif(void);
#endif

#ifdef BYD_TIMER_FNG_DOWN
void byd_fps_scan_set(unsigned char flag);
#endif
#endif//end BYD_FPS_TIMER

//#define BYD_FPS_WAKE_REPORT_KEY

//#define BYD_FPS_TASK_RPT
#ifdef BYD_FPS_TASK_RPT
static void byd_fps_input_pwrkey_report(unsigned long data);
static DECLARE_TASKLET(byd_fps_input_pwrkey_tasklet, byd_fps_input_pwrkey_report, 0);
#endif

#define BYD_FPS_WORKQUEUE_NAME_KEY  "byd_fps_workqueue_name_key"
#define BYD_FPS_WORKQUEUE_NAME_SUSP  "byd_fps_workqueue_name_susp"
void byd_fps_rd_fpd(struct byd_fps_data *byd_fps);
//long byd_fps_wait_event_interruptible_timeout(long timeout);
void byd_fps_stop_thread(u8 work_cmd);
void byd_fps_start_template_merging(void);
static int byd_fps_register_input_device(struct device *dev, struct input_dev **input_dev);
void byd_fps_wakeup_exit(struct device *dev);

int byd_fps_spi_write(struct spi_device *spi, u8 reg, u8 val);
extern void byd_fps_mutex_lock(unsigned char lock_flag,unsigned char type_flag);
void byd_fps_msleep(unsigned int msecs);

void byd_fps_start_timer_key(void);
void byd_fps_enable_irq_wake(struct spi_device *spi)
{

#ifdef BYD_FPS_IRQ_NUM
	enable_irq_wake(BYD_FPS_IRQ_NUM);
#endif
#ifdef BYD_FPS_EINT_PORT
	enable_irq_wake(spi->irq);
#endif

}

void byd_fps_disable_irq_wake(struct spi_device *spi)
{
#ifdef BYD_FPS_IRQ_NUM
	disable_irq_wake(BYD_FPS_IRQ_NUM);
#endif
#ifdef BYD_FPS_EINT_PORT
	disable_irq_wake(spi->irq);
#endif
}

#if (defined(BYD_FPS_REPORT_KEY))&&(defined(BYD_FPS_REPORT_KEY_FOCUS_CONTROLLED))
void byd_fps_set_gest_key_flag(unsigned char flag)
{
	byd_fps_gest_key_flag = flag;
	
}
#endif

// *******************************************************************************
// * Function    :  byd_fps_set_wakelock
// * Description :  set byd_fps_wakeup_data->byd_fps_wakeup_lock.Screen unlock mode.
// * In          :  wakelock
// * Return      :  void
// *******************************************************************************
void byd_fps_set_wakelock(char wakelock)
{
	if(byd_fps_wakeup_data == NULL) {
		return ;//return -1;
	}
	byd_fps_wakeup_data->byd_fps_wakeup_lock = wakelock;
	
	
}

char byd_fps_get_wakelock(void)
{
	if(byd_fps_wakeup_data == NULL) {
		return -1;
	}
	return byd_fps_wakeup_data->byd_fps_wakeup_lock;
}

void byd_fps_set_suspend_flag(char suspend_flag)
{
	if(byd_fps_wakeup_data == NULL) {
		return ;//return -1;
	}
	byd_fps_wakeup_data->byd_fps_suspend_flag = suspend_flag;
}

char byd_fps_get_suspend_flag(void)
{
	if(byd_fps_wakeup_data == NULL) {
		return -1;
	}
	return byd_fps_wakeup_data->byd_fps_suspend_flag;
}

void byd_fps_set_susp_match_cnt(char susp_match_cnt)
{
	byd_fps_wakeup_data->byd_fps_susp_match_cnt = susp_match_cnt;
}

char byd_fps_get_susp_match_cnt(void)
{
	return byd_fps_wakeup_data->byd_fps_susp_match_cnt;
}

//------------------------------input device register-------------------------
static int byd_fps_register_input_device(struct device *dev, struct input_dev **input_dev_p)
{
	int ret = 0;
	struct input_dev *input_dev;
	
	input_dev = input_allocate_device();
	if (!input_dev) {
		pr_err("%s: input allocate device failed\n", __FUNCTION__);
		return -ENOMEM;
	}
	FP_DBG("%s: Sean Debug -- input_allocate_device() OK!\n",__func__);
	input_dev->name = "byd_fps_vkey";
	input_dev->id.bustype = BUS_SPI;
	input_dev->dev.parent = dev;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0010;

	set_bit(EV_SYN, input_dev->evbit); // can be omitted?
	set_bit(EV_KEY, input_dev->evbit); // for button only?
	//set_bit(EV_ABS, input_dev->evbit); // must
//#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_REPORT_KEY_FOCUS_CONTROLLED))
	set_bit(KEY_WAKEUP,  input_dev->keybit);
	input_set_capability(input_dev, EV_KEY, KEY_WAKEUP);
#ifdef BYD_FPS_REPORT_KEY
	set_bit(KEY_FINGERPRINT,  input_dev->keybit);
	input_set_capability(input_dev, EV_KEY, KEY_FINGERPRINT);
#endif
#if (defined(BYD_FPS_GESTURE)||defined(BYD_FPS_FOCUS))
	set_bit(KEY_ENTER,  input_dev->keybit);
	input_set_capability(input_dev, EV_KEY, KEY_ENTER);
	/////////////////////////////////////////2017.02.22 added by cgh//////////
	set_bit(KEY_BACK,  input_dev->keybit);
	input_set_capability(input_dev, EV_KEY, KEY_BACK);
	//////////////////////////////////////////////////
	set_bit(KEY_UP,  input_dev->keybit);	
	input_set_capability(input_dev, EV_KEY, KEY_UP);
	set_bit(KEY_LEFT,  input_dev->keybit);	
	input_set_capability(input_dev, EV_KEY, KEY_LEFT);
	set_bit(KEY_DOWN,  input_dev->keybit);	
	input_set_capability(input_dev, EV_KEY, KEY_DOWN);
	set_bit(KEY_RIGHT,  input_dev->keybit);	
	input_set_capability(input_dev, EV_KEY, KEY_RIGHT);
#endif
	ret = input_register_device(input_dev);
	if (ret < 0) {
		pr_err("%s: input device regist failed\n", __func__);
		//input_unregister_device(byd_input_dev);
		input_free_device(input_dev); // kfree(input_dev);
		input_dev = NULL;
	}
	
	*input_dev_p = input_dev;
	
	return ret;
}


void byd_fps_wakeup_susp_worker(void)
{
	if(!work_pending(&byd_fps_wakeup_data->byd_fps_work_susp)){
		queue_work(byd_fps_wakeup_data->byd_fps_workqueue_susp,&byd_fps_wakeup_data->byd_fps_work_susp);
	}
}

//#if (defined(BYD_FPS_REPORT_KEY))||(defined(BYD_FPS_REPORT_KEY_FOCUS_CONTROLLED))
#ifdef BYD_FPS_REPORT_KEY
void byd_fps_report_key_up(void)
{
	input_report_key(byd_input_dev, KEY_FINGERPRINT, 0);
	input_sync(byd_input_dev);
	
	DBG_TIME("%s:report KEY_FINGERPRINT 0 for \n", __func__);
}
#endif

int g_need_send_signal = 0;		
//static unsigned char fng_state=0, intr_state=0;
static unsigned char byd_err_state=0;
// *******************************************************************************
// * Function    :  byd_fps_intr_work
// * Description :  put work to workqueue when in interrupt service routing.
// * In          :  void
// * Return      :  void
// *******************************************************************************
#ifdef BYD_FPS_UP_DOWN_ASYNC
extern unsigned char byd_fps_finger_up_async_flag;
extern unsigned char g_need_finger_down_scan_flag;
#endif
int byd_fps_intr_work(void)
{

	DBG_TIME("%s: byd  IN\n",__func__);
	#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
	byd_fps_set_gest_key_flag(0);
	#endif
	/*Put work "byd_fps_work_key", whose working function is "byd_fps_worker_key()", into the work queue "byd_fps_workqueue_key".*/
	if(!work_pending(&byd_fps_wakeup_data->byd_fps_work_key)){
		queue_work(byd_fps_wakeup_data->byd_fps_workqueue_key,&byd_fps_wakeup_data->byd_fps_work_key);
	}
	//if((g_need_send_signal == 1)&&((intr_state&0x04) ==0x4)&&(fng_state == 1))
/*	if(g_need_send_signal == 1)
	{
	#ifdef BYD_FPS_TIMER_KEY
		byd_fps_downif();//开启timer定时
	#endif
	#ifdef BYD_FPS_FASYNC
		//异步通知上层处理解锁事物.
		byd_fps_send_fasync();
		g_need_send_signal = 0;
		return 0;
	#endif
	}*/
	if( (byd_err_state == 0) &&(byd_fps_instr_stop_flag != 1)&&(byd_fps_int_test!=1)) {
		/*if(g_need_send_signal == 1) {
			#ifdef BYD_FPS_TIMER_KEY
			byd_fps_downif();//开启timer定时
			#endif
			#ifdef BYD_FPS_FASYNC
			//异步通知上层处理解锁事物.
			byd_fps_send_fasync();
			g_need_send_signal = 0;
			DBG_TIME("%s: byd  exit 11\n", __func__);
			return 0;
			#endif
			}*/
		//Check fng "up", then wait fng "down". 
		//Calling "byd_fps_set_suspend_flag(8)" in byd_sys_dev_ioctl() cmd=12.
		if(byd_fps_get_suspend_flag() == 8 ) {
			
		    #if 0//def BYD_FPS_UP_DOWN_ASYNC
				if(byd_fps_finger_up_async_flag == 1){
				#ifdef BYD_FPS_FASYNC
					//异步通知上层处理解锁事物.
					DBG_TIME("sync:finger up\n");
					byd_fps_send_fasync();
				#endif
					byd_fps_finger_up_async_flag = 0;
				//	g_need_send_signal = 1;
					
				}
			#endif
			DBG_TIME("%s: byd fng up now,Suspend! \n", __func__);
			DBG_TIME("%s: byd_fps_os_worker is start! Os_Scan_Expired is = %d \n", __func__, Os_Scan_Expired);
			if ((Os_Otp_or_Scan != OS_IN_OTP) && (Os_Otp_or_Scan != OS_IN_OTP_LEGACY) && (Os_Scan_Expired == 1)) {
				/*wake up thread "thread_task->wait_job" to scan OS data.*/
				byd_os_start_scan();
			}
			DBG_TIME("%s: byd_fps_os_worker is end! \n", __func__);
	
			DBG_TIME("%s: byd check fng down,Suspend! \n", __func__);
			
			FP_DBG("%s:-----------byd interrupt workqueue_fng_down------\n", __func__);
			
			/*Put work "byd_fps_work_susp", whose working function is "byd_fps_worker_susp()", into work queue "byd_fps_workqueue_susp".*/
			if(!work_pending(&byd_fps_wakeup_data->byd_fps_work_susp)){
				queue_work(byd_fps_wakeup_data->byd_fps_workqueue_susp,&byd_fps_wakeup_data->byd_fps_work_susp);
			}
			FP_DBG("%s:-----------byd interrupt workqueue_fng_down END------\n", __func__);
		}
	}  else if (byd_err_state !=0) {//error interrupt happen.
		byd_err_state = 0;
		return -1;
	  } else { //可能是在采指纹或者工厂测试
		return 0;
	    }
	DBG_TIME("%s: byd  exit \n", __func__);
	return 0;
}

extern STRUCTCHIPMODIFYINFOR structChipModifyinfo;
static void byd_fps_worker_susp(struct work_struct *work)
{
	//return ;
	
	unsigned char ret;
	int time;
	
	DBG_TIME("%s: IN\n", __func__);
	if(byd_fps_wakeup_data->byd_fps_suspend_flag == 8) {
		byd_fps_wakeup_data->byd_fps_suspend_flag = 0;
		DBG_TIME("%s: check fng down\n", __func__);
		byd_fps_mutex_lock(1,1);
#if 0//def BYD_FPS_INPUT_WAKEUP
		byd_fps_enable_irq_wake(this_byd_fps->spi);	//deep sleep , wake up
#endif
		//byd_fps_finger_detect_pre_cfg(1,1);//按下+快速, wait fng down
		time = (structChipModifyinfo.fgresetti2+1)*5+5;
		byd_fps_mdelay(time);//30 给够时间去探测手指的状态
		ret = byd_fps_get_finger_state(this_byd_fps->spi);
		byd_fps_mutex_lock(2,1);
		DBG_TIME("%s:finger_state=%d,fgresetti2=%d\n",__func__,ret,time);
		if((ret&0x01)==0) { //up_interrupt,set signal 1
			g_need_send_signal = 1;
			#ifdef BYD_FPS_UP_DOWN_ASYNC
			g_need_finger_down_scan_flag = 1;
			#endif
		}
		else { // down_interrupt don't set signal 1,reset byd_fps_suspend_flag=8
			g_need_send_signal = 0;
			byd_fps_wakeup_data->byd_fps_suspend_flag = 8;
		}
		
		DBG_TIME("%s:  waiting fng down  g_need_send_signal=%d...\n",__func__,g_need_send_signal);
	}
}

unsigned char byd_fps_get_fingeup_flag(void)
{
	return byd_fps_finger_up_flag;
}


static void byd_fps_worker_key(struct work_struct *work)
{
	unsigned char fng_state=0, intr_state=0;

	/*if(byd_last_cmd != 5)*/{
		byd_fps_mutex_lock(1,1);
		intr_state = (unsigned char)byd_fps_chip_intr_state(this_byd_fps->spi);
		fng_state = byd_fps_get_finger_state(this_byd_fps->spi);
		//byd_fps_get_finger_detect_value(this_byd_fps->spi);//读取三块子区域的手指值，用以判断是否在正常范围（ESD防护）
		byd_fps_mutex_lock(2,1);

		DBG_TIME("%s:read intr state=0x%x\n", __func__, intr_state);
		DBG_TIME("%s:read fng state=0x%x\n", __func__, fng_state);		
	}
	
	/*Added by sean, FPS22 some error checked by chip, triggering interrupt*/
	if((intr_state&0x10) ==0x10 ) {
		//unsigned char byd_err_state=0;
		byd_fps_mutex_lock(1,1);
		byd_err_state = byd_fps_chip_error_state(this_byd_fps->spi);
		
		byd_fps_get_finger_threshold(this_byd_fps->spi);
		//Clear "REG_INT_STATE", also Clear "REG_FG_ERROR_STATE".
		byd_fps_chip_fg_error_flag_clr(this_byd_fps->spi, 0);
		byd_fps_mutex_lock(2,1);
		
		#ifdef BYD_SW_RESET
		byd_fps_restart_chip();
		#endif
		
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);	//fng state? wait what?
		#endif
		intr_state = 0;
		fng_state = 0;
		
		return;
	}
	////////////////////////move from byd_fps_intr_work function for judge has the finger tuch then send fasync////////////////////////////////
	if((fng_state&0x01) && (intr_state&0x04)) {//(g_need_send_signal == 1) && 
		#ifdef BYD_FPS_TIMER_KEY
		byd_fps_downif();//开启timer定时
		#endif
		#ifdef BYD_FPS_FASYNC
		//异步通知上层处理解锁事物.
		byd_fps_send_fasync();
		DBG_TIME("sync: finger down\n");
		#endif
		g_need_send_signal = 0;
		#ifdef BYD_FPS_UP_DOWN_ASYNC
		//byd_fps_finger_up_async_flag = 1;
		#endif
		DBG_TIME("%s: byd  exit 11\n", __func__);
		return ;
		
	}
	#ifdef BYD_FPS_UP_DOWN_ASYNC
	DBG_TIME("byd_fps_finger_up_async_flag=%d,fng_state=%d\n",byd_fps_finger_up_async_flag,fng_state&0x01);
	if( !(fng_state&0x01)){//(byd_fps_finger_up_async_flag == 1) &&
		#ifdef BYD_FPS_FASYNC
		//异步通知上层处理解锁事物.
		DBG_TIME("sync:finger up\n");
		byd_fps_send_fasync();
		#endif
		//g_need_finger_down_scan_flag = 1;
		byd_fps_finger_up_async_flag = 0;
		//g_need_send_signal = 1;
		
		return ;			
	}
	#endif
	
	#if 0
	if((intr_state&0x04) ==0x04)
	{
		byd_fps_finger_up_flag = 1-fng_state;	/*fng_state: 0--fng off, 1--fng on*/
	}

	if((byd_last_cmd != 5) &&(byd_last_cmd != 1)&&(byd_fps_instr_stop_flag != 1)&&(byd_fps_int_test!=1)) {	/*byd_last_cmd: 1--reset chip, 5--capture &algorithm process.*/
		
		DBG_TIME("%s:read intr state=0x%x\n", __func__, intr_state);
		
		#ifdef BYD_FPS_TIMER_KEY
		DBG_TIME("%s:timer_flag = %d,byd_fps_keydown_flag =%d,byd_fps_keyopen_flag =%d\n",__func__, byd_fps_get_timer_flag(),byd_fps_keydown_flag,byd_fps_keyopen_flag);
		if(byd_fps_get_timer_flag() != 1)	//定时时间到,则表示脱离指纹采集模式,或者初始化.
		#endif
		{
			#ifdef BYD_FPS_POWER_CTRL
			//非指纹采集模式下,中断异常(中断类型不是手指中断),则说明出现错误,应当处理该异常.
			//BYD_FPS_ONLY_FOCUS_ON, BYD_FPS_GEST_ON为非指纹采集模式的标识.
			//byd_fps_keydown_flag == 0,驱动内部的标识,用于区分当前应用是指纹采集的应用还是按键、手势的应用.
			if(((byd_fps_keyopen_flag == BYD_FPS_ONLY_FOCUS_ON)||(byd_fps_keyopen_flag == BYD_FPS_GEST_ON)) && ((intr_state&0x04) != 0x4)&&(byd_fps_keydown_flag == 0)){
				byd_fps_power_restart(this_byd_fps->spi,0);
			}
			#endif
			
			#ifdef BYD_FPS_GEST 
			if((byd_fps_keydown_flag == 0)&&(byd_fps_keyopen_flag == BYD_FPS_GEST_ON)){
				
				byd_fps_gest_cfg(this_byd_fps->spi);
			}
			if((byd_fps_keydown_flag == 1)||(byd_fps_keyopen_flag == BYD_FPS_GEST_OFF)){
				
				byd_fps_gest_cfg_back(this_byd_fps->spi);
			}
			#endif
			
			#ifdef BYD_FPS_FOCUS
			if(((intr_state&0x04) ==0x4)&&(byd_fps_keydown_flag == 0)&&((byd_fps_keyopen_flag == BYD_FPS_ONLY_FOCUS_ON)||(byd_fps_keyopen_flag == BYD_FPS_GEST_ON))){
				if(byd_fps_finger_up_flag == 0){
					gest_int_number = 0;
					byd_fps_start_gest_cacu();//开启timer定时,读取fpd_state,
				}else if(byd_fps_finger_up_flag == 1){
					byd_fps_timer_up_stop();
					if((gest_int_number-1)>=5){
						DBG_TIME("%s:finger up,time enough,caculate gesture\n",__func__);
						//byd_fps_gest_key_flag = 1;
						byd_fps_gest_judge(this_byd_fps,gest_int_number-1);
					}
					gest_int_number = 0;
				}
			}
			#endif
		
			#ifdef BYD_FPS_GEST
			if((byd_fps_keyopen_flag != BYD_FPS_GEST_OFF)&&(byd_fps_finger_up_flag == 1)&&(byd_fps_keydown_flag == 0)&&(byd_fps_gest_key_flag ==0)){
				
				//byd_fps_start_timer_key();
			}
			#endif
			#ifdef BYD_FPS_REPORT_KEY
			if((byd_fps_keydown_flag == 0)&&((byd_fps_keyopen_flag == BYD_FPS_ONLY_KEY_ON)||(byd_fps_keyopen_flag == BYD_FPS_GEST_ON))&&((intr_state&0x04) ==0x4))	{
				
				if(byd_input_dev != NULL) {
					if((fng_state == 1)&&(byd_fps_gest_key_flag == 0)) {
						input_report_key(byd_input_dev, KEY_FINGERPRINT, 1);
						input_sync(byd_input_dev);
						byd_fps_key_mesg = 1;
						DBG_TIME("%s:report KEY_FINGERPRINT 1\n", __func__);
					}else{
						if((fng_state == 0)&&(byd_fps_key_mesg == 1)&&(byd_fps_gest_key_flag == 0)) {
							input_report_key(byd_input_dev, KEY_FINGERPRINT, 0);
							input_sync(byd_input_dev);
							byd_fps_key_mesg = 2;
		
							DBG_TIME("%s:report KEY_FINGERPRINT 0\n", __func__);
						}
					}
					#ifdef BYD_FPS_POWER_CTRL
					byd_fps_power_restart(this_byd_fps->spi,byd_fps_key_mesg);
					#endif
				}
			}
		#endif
		}
	}
	#endif
}


int byd_fps_qual_suspend(void)
{
	if (byd_fps_wakeup_data->byd_fps_suspend_qual == 1) {//休眠时进行指令判断的标志
		if (byd_fps_wakeup_data->byd_fps_suspend_flag == 0) {
			return 1;//表示立刻返回，有了优先级更高的操作
		}
	}
	return 0;
}

//kernel休眠
int byd_fps_suspend(struct spi_device *spi, pm_message_t mesg)
{
	spi = this_byd_fps->spi;
	
	byd_fps_susflag = 0;
	byd_fps_chip_lowpower_fng_detect(spi);
	return 0;
}

//kernel唤醒
int byd_fps_resume(struct spi_device *spi)
{
	unsigned char fng_state=0, intr_state=0;
	spi = this_byd_fps->spi;
	
	if(byd_fps_susflag) {

		DBG_TIME("\n byd resume send KEY_WAKEUP!\n");
		input_event(byd_input_dev,EV_KEY,KEY_WAKEUP,2);
		input_sync(byd_input_dev);
		byd_fps_mutex_lock(1,1);
		intr_state = (unsigned char)byd_fps_chip_intr_state(this_byd_fps->spi);
		fng_state = byd_fps_get_finger_state(this_byd_fps->spi);
		byd_fps_mutex_lock(2,1);
		if((fng_state&0x01) && (intr_state&0x04)) {
			#ifdef BYD_FPS_FASYNC
			//异步通知上层处理解锁事物.
			byd_fps_send_fasync();
			DBG_TIME("sync: finger down\n");
			#endif
		} else {
			#ifdef BYD_FPS_FASYNC
			//抬起唤醒
			DBG_TIME("sync:finger up\n");
			byd_fps_send_fasync();
			#endif
		} 	
	}  else {
		byd_fps_susflag = 1;
	}
	
	
	return 0;
}

// *******************************************************************************
// * Function    :  byd_fps_wakeup_init
// * Description :  malloc/init variable, init work, create work queue, register early_suspend.
// * In          :  dev  :	pointer to device
// * Return      :  0 -- finished,
// *******************************************************************************
int byd_fps_wakeup_init(struct device *dev)
{
	int ret = -1;
	
	byd_fps_wakeup_data = devm_kzalloc(dev, sizeof(*byd_fps_wakeup_data), GFP_KERNEL);
	if (!byd_fps_wakeup_data)	{
		dev_err(dev, "Failed to allocate memory\n");
		return -ENOMEM;
	}

	byd_fps_wakeup_data->byd_fps_wakeup_lock = 2;	//0--默认关闭指纹唤醒系统的功能。
	byd_fps_wakeup_data->byd_fps_suspend_flag = 0;
	byd_fps_wakeup_data->byd_fps_susp_match_cnt = 0;
	byd_fps_wakeup_data->byd_fps_suspend_qual = 0;
	
	INIT_WORK(&byd_fps_wakeup_data->byd_fps_work_susp, byd_fps_worker_susp);
	byd_fps_wakeup_data->byd_fps_workqueue_susp = create_workqueue(BYD_FPS_WORKQUEUE_NAME_SUSP);//创建新的工作队列
	
	//byd_fps_wakeup_data->finger_state_done = 0;
	//byd_fps_wakeup_data->waiting_finger_state = kzalloc(sizeof(*byd_fps_wakeup_data->waiting_finger_state), GFP_KERNEL);
	//init_waitqueue_head(byd_fps_wakeup_data->waiting_finger_state);
	INIT_WORK(&byd_fps_wakeup_data->byd_fps_work_key, byd_fps_worker_key);
	byd_fps_wakeup_data->byd_fps_workqueue_key = create_workqueue(BYD_FPS_WORKQUEUE_NAME_KEY);//创建新的工作队列
	

 	DBG_TIME("%s: register_input_device\n", __func__);
	ret = byd_fps_register_input_device(dev, &byd_input_dev);
	if (ret < 0) {
		byd_fps_wakeup_exit(dev);
		return ret;
	}
	DBG_TIME("%s: Sean Debug -- byd_fps_register_input_device() OK!\n",__func__);

	return 0;
}



// *******************************************************************************
// * Function    :  byd_fps_wakeup_exit
// * Description :  unregister,in byd_fps_remove()
// * In          :  dev - (supposed to be spi device)
// * Return      :  void
// *******************************************************************************
void byd_fps_wakeup_exit(struct device *dev)
{

	if (byd_input_dev) {
		input_unregister_device(byd_input_dev);
		input_free_device(byd_input_dev); //kfree(byd_input_dev);
	}
	destroy_workqueue(byd_fps_wakeup_data->byd_fps_workqueue_susp);
	destroy_workqueue(byd_fps_wakeup_data->byd_fps_workqueue_key);
	//kfree(byd_fps_wakeup_data->waiting_finger_state);
	devm_kfree(dev, byd_fps_wakeup_data);
	
}
