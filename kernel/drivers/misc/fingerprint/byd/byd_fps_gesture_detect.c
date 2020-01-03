#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>      // msleep()
#include <linux/input.h>
#include <linux/interrupt.h>  // IRQ_HANDLED
//#include <linux/init.h>
#include <linux/kthread.h>
//#include <linux/pm.h>

// the following 3 head files must be included in order below
#include "byd_algorithm.h"  // must after kernel head include
#include "byd_fps_bf66xx.h" //#include <linux/input/byd_fps_bf66xx.h>
#include "byd_fps_libbf663x.h"
#ifdef BYD_FPS_GESTURE
#define BYD_FPS_WORKER_THREAD_NAME_GEST  "fps_worker_thread_gest"
#ifdef BYD_FPS_POWER_CTRL
#define BYD_FPS_POWER_OFF_ON 3
#endif

extern unsigned long set_timeout_finger_down;
extern unsigned long set_timeout_finger_up;
extern unsigned char finger_present;
extern const unsigned char FINGER_DOWN;
extern const unsigned char FINGER_UP;
extern struct input_dev *byd_input_dev;
extern struct byd_fps_data  *this_byd_fps;
extern struct mutex byd_fps_mutex_chip;
extern unsigned char byd_fps_gesture_suspend;


struct byd_fps_gest_data_t
{
	struct byd_fps_thread_task *thread_task_gest;
	u8 byd_fps_click_flag ;
	u8 byd_fps_gesture_on_flag ;
	u8 byd_fps_gesture_default_flag;//设置开机后手势功能的默认状态。1：开；0：关
};
static struct byd_fps_gest_data_t *byd_fps_gest_data = NULL;

#ifdef BYD_FPS_FOCUS
extern unsigned char gest_int_number;
unsigned char byd_fps_gest_flag = 0;

void byd_fps_timer_key_stop(void);

void byd_fps_start_timer_up(void);
void byd_fps_udelay(unsigned int usecs);
void byd_fps_timer_up_stop(void);
extern unsigned char byd_fps_finger_up_flag;
#define BYD_FPS_GEST_CACU  2

//#define KEY_UP   103
//#define KEY_LEFT  105
//#define KEY_RIGHT  106
//#define KEY_DOWN  108
#endif
#ifdef PLATFORM_MTK
int byd_fps_init_port(void);
#endif
#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
void byd_fps_set_gest_key_flag(unsigned char flag);
#endif

// *******************************************************************************
// * Function    :  byd_fps_gesture_off
// * Description :  set byd_fps_gesture_on_flag 0.
// * In          :  void.
// * Return      :  void.
// *******************************************************************************
void byd_fps_gesture_off(void)
{
	/*if(byd_fps_gest_data->byd_fps_gesture_default_flag==1)*/
		byd_fps_gest_data->byd_fps_gesture_on_flag  = 0;	
}

// *******************************************************************************
// * Function    :  byd_fps_start_detect_gest
// * Description :  start thread of gesture checking.
// * In          :  @*dev - point to device.
// * Return      :  void.
// *******************************************************************************
void byd_fps_start_detect_gest(void) 
{
	if(byd_fps_gest_data->byd_fps_gesture_default_flag==0){
		byd_fps_gest_data->byd_fps_gesture_on_flag = 1;
		byd_fps_gest_data->thread_task_gest->mode = FPS_THREAD_MODE_ALG_PROC;
		wake_up_interruptible(&byd_fps_gest_data->thread_task_gest->wait_job);
	}
}

int byd_fps_set_gest_state(unsigned char gest_state)
{
	byd_fps_gest_data->byd_fps_gesture_default_flag = gest_state;
	return 0;
}

int byd_fps_get_gest_state(void)
{
	return (byd_fps_gest_data->byd_fps_gesture_default_flag);
}
#ifdef BYD_FPS_FOCUS
void byd_fps_start_gest_cacu(void) 
{
	
	mutex_lock(&byd_fps_mutex_chip);
	printk("%s:gest_int_number=%d\n",__func__,gest_int_number);
	byd_fps_get_fpd_state(this_byd_fps->spi,gest_int_number);
	mutex_unlock(&byd_fps_mutex_chip);
	if(byd_fps_finger_up_flag!=1){//手指没有抬起

		byd_fps_start_timer_up();
	}
	gest_int_number++;
	if(gest_int_number >= 200){//2017.02.22 changed by cgh  old is 100 
		byd_fps_timer_up_stop();
		byd_fps_gest_judge(this_byd_fps,gest_int_number);//1.手指抬起手势判断；2.手指一直不抬起，采集到200帧上报手势
		gest_int_number = 0;
	}
}
//#define BYD_SAVE_GEST
#ifdef BYD_SAVE_GEST
static unsigned char byd_fps_img_cnt_gest = 0;
unsigned char byd_fps_left_right_file[] = "/sdcard/byd_fps/byd_gest_lr00.txt";
unsigned char byd_fps_up_down_file[] =    "/sdcard/byd_fps/byd_gest_ud00.txt";
int byd_fps_write_file(char *p_path, char *buf, int len, int offset);
#endif

int byd_fps_gest_judge(struct byd_fps_data *byd_fps,unsigned char gest_int_num)
{
	
	int i;
	 char x_max=0,x_min=40;//,x_mid=0;
	 char y_max=0,y_min=40;//,y_mid=0;
	unsigned char x_max_num=0,x_min_num=0,y_max_num=0,y_min_num=0;
	unsigned char ave_num;
	 int sum_lr_a=0,sum_lr_b=0,sum_lr_c=0;
	 int sum_ud_a=0,sum_ud_b=0,sum_ud_c=0;
	
	//每一份除掉最大值和最小值，减小误差
	unsigned int max_lr_a=0,max_lr_b=0,max_lr_c=0,min_lr_a=byd_fps->left_right_state[0],min_lr_b=byd_fps->left_right_state[(gest_int_num-1)/3],min_lr_c=byd_fps->left_right_state[2*(gest_int_num-1)/3];
	unsigned int max_ud_a=0,max_ud_b=0,max_ud_c=0,min_ud_a=byd_fps->up_down_state[0],min_ud_b=byd_fps->up_down_state[(gest_int_num-1)/3],min_ud_c=byd_fps->up_down_state[2*(gest_int_num-1)/3];
	//
		#ifdef BYD_SAVE_GEST
		#define SAVE_TO_SD
		DBG_TIME("%s: byd write gest data\n", __func__);
		
			if(byd_fps_img_cnt_gest < 100) {
				#ifdef SAVE_TO_SD
				byd_fps_left_right_file[27] = 48+(byd_fps_img_cnt_gest/10);
				byd_fps_left_right_file[28] = 48+(byd_fps_img_cnt_gest%10);
				byd_fps_up_down_file[27] = 48+(byd_fps_img_cnt_gest/10);
				byd_fps_up_down_file[28] = 48+(byd_fps_img_cnt_gest%10);
				
				#else
				byd_fps_up_down_file[59] = 48+(byd_fps_img_cnt_gest/10);
				byd_fps_up_down_file[60] = 48+(byd_fps_img_cnt_gest%10);
				#endif
				byd_fps_img_cnt_gest++;
			} else {
				byd_fps_img_cnt_gest = 0;
				#ifdef SAVE_TO_SD
				byd_fps_up_down_file[27] = 48+0;
				byd_fps_up_down_file[28] = 48+0;
				#else
				byd_fps_up_down_file[59] = 48+0;
				byd_fps_up_down_file[60] = 48+0;
				#endif
				byd_fps_img_cnt_gest++;
			}
			byd_fps_write_file(byd_fps_left_right_file, byd_fps->left_right_state,gest_int_num+1 , 0);
			byd_fps_write_file(byd_fps_up_down_file, byd_fps->up_down_state,gest_int_num+1 , 0);
			
		#endif
		//抛掉最后一帧，算出最大值和最小值
	for(i=0;i<=(gest_int_num-1);i++){
		FP_DBG("%s :byd_fps->left_right_state[%d]:%d\n",__func__,i,byd_fps->left_right_state[i]);
		FP_DBG("%s :byd_fps->up_down_state[%d]:%d\n",__func__,i,byd_fps->up_down_state[i]);
	
		if(byd_fps->left_right_state[i]>x_max){
			x_max = byd_fps->left_right_state[i];
			x_max_num = i;
		}
		if(byd_fps->left_right_state[i]<x_min){
			x_min = byd_fps->left_right_state[i];
			x_min_num = i;
		}
		if(byd_fps->up_down_state[i]>y_max){
			y_max = byd_fps->up_down_state[i];
			y_max_num = i;
		}
		if(byd_fps->up_down_state[i]<y_min){
			y_min = byd_fps->up_down_state[i];
			y_min_num = i;
		}
	}
	DBG_TIME("%s:x_max=%d,x_max_num=%d,x_min=%d,x_min_num=%d,y_max=%d,y_max_num=%d,y_min=%d,y_min_num=%d\n",__func__,x_max,x_max_num,x_min,x_min_num,y_max,y_max_num,y_min,y_min_num);
	//将数据分为三分，判断三份的值的变化
	ave_num = (gest_int_num-1)/3;
	printk("%s:ave_num:%d\n",__func__,ave_num);
	for(i=0;i<ave_num;i++){
		//////////////////////////
		if(ave_num>=3)
			
		{ 
		if(max_lr_a<byd_fps->left_right_state[i])
			max_lr_a=byd_fps->left_right_state[i];
		 if(min_lr_a>byd_fps->left_right_state[i])
			min_lr_a=byd_fps->left_right_state[i];
		//
		 if(max_ud_a<byd_fps->up_down_state[i])
			max_ud_a=byd_fps->up_down_state[i];
		 if(min_ud_a>byd_fps->up_down_state[i])
			min_ud_a=byd_fps->up_down_state[i];
		                                       }
		/////////////////////////////
		sum_lr_a += byd_fps->left_right_state[i];
		sum_ud_a += byd_fps->up_down_state[i];
    }
	////////////////////
	sum_lr_a = sum_lr_a-max_lr_a-min_lr_a;
	sum_ud_a = sum_ud_a-max_ud_a-min_ud_a;
	////////////////////////////////////	
	for(i=ave_num;i<ave_num*2;i++){
		if(ave_num>=3){
	    //////////////////////////
		if(max_lr_b<byd_fps->left_right_state[i])
			max_lr_b=byd_fps->left_right_state[i];
		if(min_lr_b>byd_fps->left_right_state[i])
			min_lr_b=byd_fps->left_right_state[i];
		//
		if(max_ud_b<byd_fps->up_down_state[i])
			max_ud_b=byd_fps->up_down_state[i];
		if(min_ud_b>byd_fps->up_down_state[i])
		min_ud_b=byd_fps->up_down_state[i];}
		/////////////////////////////
		sum_lr_b += byd_fps->left_right_state[i];
		sum_ud_b += byd_fps->up_down_state[i];
	}
	////////////////////
	sum_lr_b = sum_lr_b-max_lr_b-min_lr_b;
	sum_ud_b = sum_ud_b-max_ud_b-min_ud_b;
	///////////////////////////////////	
	for(i=ave_num*2;i<ave_num*3;i++){
		if(ave_num>=3){
		//////////////////////////
		if(max_lr_c<byd_fps->left_right_state[i])
			max_lr_c=byd_fps->left_right_state[i];
		if(min_lr_c>byd_fps->left_right_state[i])
			min_lr_c=byd_fps->left_right_state[i];
		//
		if(max_ud_c<byd_fps->up_down_state[i])
			max_ud_c=byd_fps->up_down_state[i];
		if(min_ud_c>byd_fps->up_down_state[i])
		min_ud_c=byd_fps->up_down_state[i];}
		/////////////////////////////

		sum_lr_c += byd_fps->left_right_state[i];
		sum_ud_c += byd_fps->up_down_state[i];
	}
	////////////////////
	sum_lr_c = sum_lr_c-max_lr_c-min_lr_c;
	sum_ud_c = sum_ud_c-max_ud_c-min_ud_c;
	////////////////////////////////////
	for(i=0;i<100;i++){
		byd_fps->left_right_state[i] = 0;
		byd_fps->up_down_state[i]    = 0;
	}
	printk("%s:sum_lr_a:%d,sum_lr_b:%d,sum_lr_c:%d,sum_ud_a:%d,sum_ud_b:%d,sum_ud_c:%d\n",__func__,sum_lr_a,sum_lr_b,sum_lr_c,sum_ud_a,sum_ud_b,sum_ud_c);
	/*if((x_max>=29)&&(x_min <=11)&&(x_max_num<x_min_num)&&((y_max-y_min)<15))*/
	/*  不同的芯片方案，其晶元的布局不一样导致方向不同，以下是:
	盖板BYD_FPS_128glass175和BYD_FPS_32COATING的差异，用宏 来控制 */
	//if( (gest_int_num>=150/*长按450ms以上才判为长按key_back*/) && (-5<=((x_max-x_min)-(y_max-y_min))<=5) && (-2<=(sum_ud_a-sum_ud_c)<=2) &&
	//		(-2<=(sum_ud_a-sum_ud_b)<=2 ) && (-2<=(sum_lr_a-sum_lr_c)<=2) && (-2<=(sum_lr_a-sum_lr_b)<=2) )	
    #ifdef CONFIG_FPS12
	if( (gest_int_num>=150/*长按450ms以上才判为长按key_back*/) && 
	((((x_max-x_min)-(y_max-y_min))>=-5) && (((x_max-x_min)-(y_max-y_min))<=5)) && 
	(((sum_ud_a-sum_ud_c)>=-2) && ((sum_ud_a-sum_ud_c)<=2)) && 
	(((sum_ud_a-sum_ud_b)>=-2) && ((sum_ud_a-sum_ud_b)<=2)) && 
	(((sum_lr_a-sum_lr_c)>=-2) && ((sum_lr_a-sum_lr_c)<=2)) && 
	(((sum_lr_a-sum_lr_b)>=-2) && ((sum_lr_a-sum_lr_b)<=2)) )
	#else
	if( (gest_int_num>=120/*长按450ms以上才判为长按key_back*/) &&
	(((sum_lr_a-sum_lr_c)>=-20) && ((sum_lr_a-sum_lr_c)<=20)) && 
	(((sum_lr_a-sum_lr_b)>=-20) && ((sum_lr_a-sum_lr_b)<=20)) )	
	#endif
	{
		#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
		byd_fps_set_gest_key_flag(1);
		#endif
			
		input_report_key(byd_input_dev, KEY_BACK, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_BACK, 0);
		input_sync(byd_input_dev);	
		DBG_TIME("%s:report KEY_BACK\n",__func__);	
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);
		#endif
		
		}
	/////////////////////////////////////////////////////////////////////////////
	else if(/*(sum_lr_a>=sum_lr_b)&&(sum_lr_b>=sum_lr_c)&&*/((x_max-x_min)>=(y_max-y_min))&&((sum_lr_a-sum_lr_c)>=12)&&(sum_lr_a>sum_lr_c))
	{
		#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
		byd_fps_set_gest_key_flag(1);
		#endif
		#ifdef BYD_FPS_32COATING
		input_report_key(byd_input_dev, KEY_LEFT, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_LEFT, 0);
		input_sync(byd_input_dev);
		DBG_TIME("%s:report KEY_LEFT \n",__func__);
		#endif
		#ifdef BYD_FPS_128glass175
		input_report_key(byd_input_dev, KEY_UP, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_UP, 0);
		input_sync(byd_input_dev);
		DBG_TIME("%s:report KEY_UP\n",__func__);
		#endif
		/////////////////////////////////////////////////////////////2017.02.21 added by cgh  //
		byd_fps_timer_key_stop();
		DBG_TIME("byd_fps_timer_up_stop \n");
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);
		#endif
		//////////////////////////////////////////////////////////////
	}else /*if((x_max>=29)&&(x_min <=11)&&(x_max_num>x_min_num)&&((y_max-y_min)<15))*/
	if(/*(sum_lr_a<=sum_lr_b)&&(sum_lr_b<=sum_lr_c)&&*/((x_max-x_min)>=(y_max-y_min))&&((sum_lr_c-sum_lr_a)>=12)&&(sum_lr_a<sum_lr_c))
	{
		#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
		byd_fps_set_gest_key_flag(1);
		#endif
		#ifdef BYD_FPS_32COATING
		input_report_key(byd_input_dev, KEY_RIGHT, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_RIGHT, 0);
		input_sync(byd_input_dev);
		DBG_TIME("%s:report KEY_RIGHT\n",__func__);
		#endif
		#ifdef BYD_FPS_128glass175
		input_report_key(byd_input_dev, KEY_DOWN, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_DOWN, 0);
		input_sync(byd_input_dev);
		DBG_TIME("%s:report KEY_DOWN\n",__func__);
		#endif
		//////////////////////////////////////////2017.02.21 added by cgh/////////////////
		byd_fps_timer_key_stop();
		DBG_TIME("byd_fps_timer_up_stop \n");
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);
		#endif
		//////////////////////////////////////////////////////////////
	}else if(/*(sum_ud_a<=sum_ud_b)&&(sum_ud_b<=sum_ud_c)&&*/((x_max-x_min)<(y_max-y_min))&&((sum_ud_c-sum_ud_a)>=12)&&(sum_ud_c>sum_ud_a)){
		#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
		byd_fps_set_gest_key_flag(1);
		#endif
		#ifdef BYD_FPS_32COATING
		input_report_key(byd_input_dev, KEY_DOWN, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_DOWN, 0);
		input_sync(byd_input_dev);	
		DBG_TIME("%s:report KEY_DOWN\n",__func__);
		#endif
		#ifdef BYD_FPS_128glass175
		input_report_key(byd_input_dev, KEY_LEFT, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_LEFT, 0);
		input_sync(byd_input_dev);	
		DBG_TIME("%s:report KEY_LEFT\n",__func__);
		#endif
		//////////////////////////////////////////2017.02.21 added by cgh//////////////////
		byd_fps_timer_key_stop();
		DBG_TIME("byd_fps_timer_up_stop \n");
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);
		#endif
		//////////////////////////////////////////////////////////////
		
	}else if(/*(sum_ud_a>=sum_ud_b)&&(sum_ud_b>=sum_ud_c)&&*/((x_max-x_min)<(y_max-y_min))&&((sum_ud_a-sum_ud_c)>=12)&&(sum_ud_c<sum_ud_a)){
		#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
		byd_fps_set_gest_key_flag(1);
		#endif
		#ifdef BYD_FPS_32COATING
		input_report_key(byd_input_dev, KEY_UP, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_UP, 0);
		input_sync(byd_input_dev);	
		DBG_TIME("%s:report KEY_UP\n",__func__);
		#endif
		#ifdef BYD_FPS_128glass175
		input_report_key(byd_input_dev, KEY_RIGHT, 1);
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_RIGHT, 0);
		input_sync(byd_input_dev);	
		DBG_TIME("%s:report KEY_RIGHT\n",__func__);		
		#endif
		//////////////////////////////////////////2017.02.21 added by cgh//////////////
		byd_fps_timer_key_stop();
		DBG_TIME("byd_fps_timer_up_stop \n");
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);
		#endif
		//////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////2017.02.22 added byd cgh //////////////
	}
	else{
		#if defined(BYD_FPS_REPORT_KEY)&&defined(BYD_FPS_FOCUS)
		byd_fps_set_gest_key_flag(0);
		#endif
	}
	return 0;
}
void byd_fps_start_gest_cacu_test(void) 
{
	{
		byd_fps_gest_data->thread_task_gest->mode = BYD_FPS_GEST_CACU;
		wake_up_interruptible(&byd_fps_gest_data->thread_task_gest->wait_job);
	}
	
}
#endif
#ifdef BYD_FPS_POWER_CTRL
void byd_fps_start_powerchip_cfg(void) 
{
	{
		byd_fps_gest_data->thread_task_gest->mode = BYD_FPS_POWER_OFF_ON;
		wake_up_interruptible(&byd_fps_gest_data->thread_task_gest->wait_job);
	}
	
}
#endif
// *******************************************************************************
// * Function    :  byd_fps_threadfn_gest
// * Description :  function of thread
// * In          :  @*byd_fps_thread - point to the data type defined by BYD.
// * Return      :  0--succeed.
// *******************************************************************************
static int byd_fps_threadfn_gest(void *byd_fps_thread)
{
	int ret = 0;
	struct byd_fps_thread_task *thread_task_gest= byd_fps_thread;

	DBG_TIME("%s: run IN thread_func_gest ...1 !!!\n ", __func__);
	
	while (!kthread_should_stop()) {
		FP_DBG("%s: run thread_gest in while()...2 ,byd_fps_gest_data->thread_task_gest->mode = %d!!!\n ", __func__, byd_fps_gest_data->thread_task_gest->mode);
		up(&thread_task_gest->sem_idle);
		wait_event_interruptible(thread_task_gest->wait_job,byd_fps_gest_data->thread_task_gest->mode != FPS_THREAD_MODE_IDLE);
		
		FP_DBG("%s: run thread_gest in while()...3,break wait_event_interruptible() ,byd_fps_gest_data->thread_task_gest->mode = %d!!!\n ", __func__, byd_fps_gest_data->thread_task_gest->mode);
		down(&thread_task_gest->sem_idle);
		FP_DBG("%s: run switch ()4!!! byd_fps_gest_data->thread_task_gest->mode = %d\n ", __func__, byd_fps_gest_data->thread_task_gest->mode);
		
		switch (byd_fps_gest_data->thread_task_gest->mode){
			case FPS_THREAD_MODE_ALG_PROC:
				FP_DBG("%s: run in switch () 5!!! \n ", __func__);
				#if (defined(BYD_FPS_FOCUS))&&(defined(BYD_FPS_GESTURE))
				if(byd_fps_gest_data->byd_fps_gesture_on_flag == 1){
					
					byd_fps_set_gest_state(1);	//set state of "detect gesture", means starting.
					mutex_lock(&byd_fps_mutex_chip);
					byd_fps_chip_idle(this_byd_fps->spi);
					//ret = byd_fps_area_gest(this_byd_fps);
					mutex_unlock(&byd_fps_mutex_chip);
				} else {
					byd_fps_set_gest_state(0);
				}
				#endif
			break;
			#ifdef BYD_FPS_FOCUS
			case BYD_FPS_GEST_CACU:
			if(byd_fps_finger_up_flag!=1){//
				byd_fps_start_gest_cacu();
			}
			break;
			#endif
#ifdef BYD_FPS_POWER_CTRL
			case BYD_FPS_POWER_OFF_ON:
				byd_fps_power_restart(this_byd_fps->spi,0);
			break;
#endif
			default:
			break;
		}
		if(ret != 0){
			break;
		}
		FP_DBG("%s: out of switch ...7 !!!\n ", __func__);
		if(thread_task_gest->mode != FPS_THREAD_MODE_EXIT){
			DBG_TIME("%s: set  FPS_THREAD_MODE_IDLE ... 8!!!\n ", __func__);
			thread_task_gest->mode = FPS_THREAD_MODE_IDLE;
			schedule_timeout(HZ);
		}
	}
	FP_DBG("%s: exit thread func ... 9!!!\n ", __func__);
	return 0;
}

// *******************************************************************************
// * Function    :  byd_fps_gest_init
// * Description :  initiate resource used by gesture checking.
// * In          :  @*dev - point to device.
// * Return      :  0--succeed, -12--memory allocated fail.
// *******************************************************************************
int byd_fps_gest_init(struct device *dev)
{
	byd_fps_gest_data = devm_kzalloc(dev, sizeof(*byd_fps_gest_data), GFP_KERNEL);
	if (!byd_fps_gest_data) {
		dev_err(dev, "Failed to allocate memory for byd_fps_gest_data\n");
		return -ENOMEM;//-12
	}
	byd_fps_gest_data->thread_task_gest = kzalloc(sizeof(*byd_fps_gest_data->thread_task_gest), GFP_KERNEL);
	if (!byd_fps_gest_data->thread_task_gest) {
		dev_err(dev, "failed to allocate memory for thread_task\n");
		return -ENOMEM;
	}
	byd_fps_gest_data->thread_task_gest->should_stop = 0;
	byd_fps_gest_data->byd_fps_gesture_on_flag  = 0;
	byd_fps_gest_data->byd_fps_gesture_default_flag = 0;
	byd_fps_gest_data->byd_fps_click_flag = 0;//上报按下为1，上报抬起为2.
	init_waitqueue_head(&byd_fps_gest_data->thread_task_gest->wait_job);
	sema_init(&byd_fps_gest_data->thread_task_gest->sem_idle, 0);
	byd_fps_gest_data->thread_task_gest->mode = FPS_THREAD_MODE_IDLE;
	printk("%s:init2\n",__func__);
	byd_fps_gest_data->thread_task_gest->thread = kthread_run(byd_fps_threadfn_gest, byd_fps_gest_data->thread_task_gest, "%s", BYD_FPS_WORKER_THREAD_NAME_GEST);
	if (IS_ERR(byd_fps_gest_data->thread_task_gest->thread)) {
		dev_err(dev, "kthread_run failed.\n");
		kthread_stop(byd_fps_gest_data->thread_task_gest->thread);
	}
	return 0 ;
		
}

// *******************************************************************************
// * Function    :  byd_fps_gest_exit
// * Description :  release resource used by gesture checking.
// * In          :  @*dev - point to device.
// * Return      :  void.
// *******************************************************************************
void byd_fps_gest_exit(struct device *dev)
{
	if (!IS_ERR(byd_fps_gest_data->thread_task_gest->thread)){
		//if(kthread_could_stop())
			byd_fps_gest_data->thread_task_gest->mode = FPS_THREAD_MODE_EXIT;
			kthread_stop(byd_fps_gest_data->thread_task_gest->thread);
	}
	//kthread_stop(byd_fps_gest_data->thread_task_gest->thread);
	devm_kfree(dev, byd_fps_gest_data);
}

#endif
