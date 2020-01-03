#include <linux/device.h>//dev_alloc
#include <linux/slab.h>//kzalloc
#include <linux/hrtimer.h>
#include <linux/input.h>
// byd_algorithm.h must be included after linux inclution and before byd_fps_*.h
#include "byd_algorithm.h"  // must after kernel head include
#include "byd_fps_bf66xx.h" //#include <linux/input/byd_fps_bf66xx.h>
#include "byd_fps_libbf663x.h"

//extern struct byd_fps_data  *this_byd_fps;
//extern unsigned char byd_fps_fae_detect[6];

static enum hrtimer_restart byd_fps_timer_func_down(struct hrtimer *timer);
int byd_fps_timer_try_cancel(struct hrtimer *timer);
extern struct byd_fps_data  *this_byd_fps;
const unsigned int byd_timer_down = 500;	//(ms)

#ifdef BYD_FPS_GEST
extern unsigned int byd_fps_keydown_flag;
extern unsigned int byd_fps_keyopen_flag;
const unsigned int byd_timer_key =300;	//(ms)
extern struct input_dev *byd_input_dev;
extern unsigned char byd_fps_finger_up_flag;
extern unsigned char byd_fps_gest_key_flag;
#ifdef BYD_FPS_FOCUS
const unsigned int byd_timer_up =3;	//(ms)
extern unsigned char gest_int_number;
static enum hrtimer_restart byd_fps_timer_func_up(struct hrtimer *timer);
#endif
unsigned char byd_fps_get_fingeup_flag(void);
static enum hrtimer_restart byd_fps_timer_func_key(struct hrtimer *timer);
#endif
extern void byd_fps_mutex_lock(unsigned char lock_flag,unsigned char type_flag);


struct byd_fps_timer_data_t
{
	struct hrtimer byd_fps_timer_down;
	struct hrtimer *byd_this_hrtimer_down;
#ifdef BYD_FPS_FOCUS
	struct hrtimer byd_fps_timer_up;
	struct hrtimer *byd_this_hrtimer_up;
	unsigned char byd_up_timer_flag;
#endif
#ifdef BYD_FPS_GEST
	struct hrtimer byd_fps_timer_key;
	struct hrtimer *byd_this_hrtimer_key;
	unsigned char byd_key_timer_flag;
#endif
	unsigned char byd_fps_finger_det_cnt;

	unsigned char byd_down_timer_flag;

};
struct byd_fps_timer_data_t *byd_fps_timer_data = NULL;

int byd_fps_timer_start(struct hrtimer *timer, unsigned int byd_time);

#ifdef BYD_FPS_POWER_CTRL
void byd_fps_start_powerchip_cfg(void);
#endif

// *******************************************************************************
// * Function    :  byd_fps_timer_init
// * Description :  release timer_data, timer,monitor_workqueue
// * In          :  *dev
// * Return      :  0--succeed, -12--memory allocate fail(-ENOMEM).
// *                
// *******************************************************************************
int byd_fps_timer_init(struct device *dev)
{
	byd_fps_timer_data = devm_kzalloc(dev, sizeof(*byd_fps_timer_data), GFP_KERNEL);
	if (!byd_fps_timer_data) {
		dev_err(dev, "Failed to allocate memory for byd_fps_timer_data\n");
		return -ENOMEM;//-12
	}


	byd_fps_timer_data->byd_this_hrtimer_down = kzalloc(sizeof(*byd_fps_timer_data->byd_this_hrtimer_down), GFP_KERNEL);
	if (!byd_fps_timer_data->byd_this_hrtimer_down) {
		dev_err(dev, "failed to allocate memory for byd_this_hrtimer_down\n");
		return -ENOMEM;
	}
	byd_fps_timer_data->byd_down_timer_flag = 0;
	byd_fps_timer_data->byd_this_hrtimer_down = &byd_fps_timer_data->byd_fps_timer_down;
	hrtimer_init(&byd_fps_timer_data->byd_fps_timer_down, CLOCK_MONOTONIC, HRTIMER_MODE_REL); // Initial the struc hrtimer
	byd_fps_timer_data->byd_fps_timer_down.function = byd_fps_timer_func_down;
#ifdef BYD_FPS_FOCUS
	byd_fps_timer_data->byd_this_hrtimer_up = kzalloc(sizeof(*byd_fps_timer_data->byd_this_hrtimer_up), GFP_KERNEL);
	if (!byd_fps_timer_data->byd_this_hrtimer_up) {
		dev_err(dev, "failed to allocate memory for byd_this_hrtimer_up\n");
		return -ENOMEM;
	}
	byd_fps_timer_data->byd_this_hrtimer_up = &byd_fps_timer_data->byd_fps_timer_up;
	hrtimer_init(&byd_fps_timer_data->byd_fps_timer_up, CLOCK_MONOTONIC, HRTIMER_MODE_REL); 
	byd_fps_timer_data->byd_fps_timer_up.function = byd_fps_timer_func_up;
	byd_fps_timer_data->byd_up_timer_flag = 0;
	
#endif
#ifdef BYD_FPS_GEST
	byd_fps_timer_data->byd_this_hrtimer_key = kzalloc(sizeof(*byd_fps_timer_data->byd_this_hrtimer_key), GFP_KERNEL);
	if (!byd_fps_timer_data->byd_this_hrtimer_key) {
		dev_err(dev, "failed to allocate memory for byd_this_hrtimer_key\n");
		return -ENOMEM;
	}
	byd_fps_timer_data->byd_this_hrtimer_key = &byd_fps_timer_data->byd_fps_timer_key;
	hrtimer_init(&byd_fps_timer_data->byd_fps_timer_key, CLOCK_MONOTONIC, HRTIMER_MODE_REL); 
	byd_fps_timer_data->byd_fps_timer_key.function = byd_fps_timer_func_key;
	byd_fps_timer_data->byd_key_timer_flag = 0;
#endif
	return 0;
}

// *******************************************************************************
// * Function    :  byd_fps_timer_exit
// * Description :  release timer_data, timer,monitor_workqueue
// * In          :  *dev
// * Return      :  
// *                
// *******************************************************************************
void byd_fps_timer_exit(struct device *dev)
{
	devm_kfree(dev, byd_fps_timer_data);
	hrtimer_cancel(byd_fps_timer_data->byd_this_hrtimer_down);
	#ifdef BYD_FPS_FOCUS
	hrtimer_cancel(byd_fps_timer_data->byd_this_hrtimer_up);
	#endif
	#ifdef BYD_FPS_REPORT_KEY
	hrtimer_cancel(byd_fps_timer_data->byd_this_hrtimer_key);
	#endif
}

// *******************************************************************************
//* Function    :  byd_fps_timer_func_down
//* Description :  ISR of timer, response when time out.Start count,when finger down
//* In          :  timer associated with this function
//* Return      :  working mode of timer, HRTIMER_NORESTART returned indicates
//*                there is no need for timer to restart
// *******************************************************************************
static enum hrtimer_restart byd_fps_timer_func_down(struct hrtimer *timer)
{
	DBG_TIME("%s: IN\n", __func__);
	if(byd_fps_timer_data->byd_down_timer_flag == 1) {
		byd_fps_timer_data->byd_down_timer_flag = 2;	//test_timer
	}
	DBG_TIME("%s: timeout,flag=%d\n", __func__, byd_fps_timer_data->byd_down_timer_flag);
	return HRTIMER_NORESTART;
}

#ifdef BYD_FPS_GEST
void byd_fps_start_timer_key(void)
{
	DBG_TIME("%s: IN\n", __func__);
	DBG_TIME("%s:timer key start,byd_key_timer_flag =%d\n", __func__,byd_fps_timer_data->byd_key_timer_flag);

	if((byd_fps_timer_data->byd_key_timer_flag == 0)||(byd_fps_timer_data->byd_key_timer_flag == 2)){
		DBG_TIME("%s:timer start key\n", __func__);
		
		byd_fps_timer_start(byd_fps_timer_data->byd_this_hrtimer_key, byd_timer_key);
		byd_fps_timer_data->byd_key_timer_flag = 1;
		byd_fps_finger_up_flag = 2;//2
	}
}

//////////////////////////////////////////////////////////////////////////
/*2017.02.21 added by cgh 
*/
void byd_fps_timer_key_stop(void)
{
	DBG_TIME("%s:timer key stop,byd_key_timer_flag =%d\n", __func__,byd_fps_timer_data->byd_key_timer_flag);
	byd_fps_timer_data->byd_key_timer_flag = 2;
	if((byd_fps_timer_data->byd_key_timer_flag == 1)||(byd_fps_timer_data->byd_key_timer_flag == 2)){
		byd_fps_timer_try_cancel(byd_fps_timer_data->byd_this_hrtimer_key);
	}
}
///////////////////////////////////////////////////////////////////////////
static enum hrtimer_restart byd_fps_timer_func_key(struct hrtimer *timer)
{	
	
	byd_fps_timer_data->byd_key_timer_flag = 2;
	#ifdef BYD_FPS_REPORT_KEY
	
	if((byd_fps_get_fingeup_flag()==2)&&((byd_fps_keyopen_flag == BYD_FPS_ONLY_KEY_ON)||(byd_fps_keyopen_flag == BYD_FPS_GEST_ON))&&(byd_fps_keydown_flag == 0)&&(byd_fps_gest_key_flag == 0)){//表示在一定时间内没有手指再次抬起，上报单击事件
		input_report_key(byd_input_dev, KEY_FINGERPRINT, 1);//KEY_BACK
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_FINGERPRINT, 0);//KEY_BACK
		input_sync(byd_input_dev);
		DBG_TIME("%s:report KEY_FINGERPRINT \n",__func__);//KEY_BACK
		
		#ifdef BYD_FPS_POWER_CTRL
		//byd_fps_power_restart(this_byd_fps->spi,0);
		byd_fps_start_powerchip_cfg();
		#endif
		
	}
	#endif
	
	
	#ifdef BYD_FPS_FOCUS
	if((byd_fps_get_fingeup_flag()==1)&&((byd_fps_keyopen_flag == BYD_FPS_ONLY_FOCUS_ON)||(byd_fps_keyopen_flag == BYD_FPS_GEST_ON))&&(byd_fps_keydown_flag == 0)&&(byd_fps_gest_key_flag == 0)){//上报双击事件
		input_report_key(byd_input_dev, KEY_ENTER, 1);//KEY_ENTER
		input_sync(byd_input_dev);
		input_report_key(byd_input_dev, KEY_ENTER, 0);//KEY_ENTER
		input_sync(byd_input_dev);
		DBG_TIME("%s:report KEY_ENTER \n",__func__);//KEY_ENTER	
		#ifdef BYD_FPS_POWER_CTRL
		byd_fps_power_restart(this_byd_fps->spi,0);
		#endif
	}
	#endif
	return HRTIMER_NORESTART;
}
#endif
#ifdef BYD_FPS_FOCUS
void byd_fps_start_gest_cacu_test(void);
static enum hrtimer_restart byd_fps_timer_func_up(struct hrtimer *timer)
{	
	
	DBG_TIME("%s: IN\n", __func__);
	
	byd_fps_timer_data->byd_up_timer_flag = 2;
	byd_fps_start_gest_cacu_test();//不能直接进行SPI通信，开启线程读取fpd_state
	return HRTIMER_NORESTART;
}
void byd_fps_start_timer_up(void)
{
			
			DBG_TIME("%s:IN \n", __func__);
			
	if((byd_fps_timer_data->byd_up_timer_flag == 0)||(byd_fps_timer_data->byd_up_timer_flag == 2)){
			
			DBG_TIME("%s:timer start up\n", __func__);
			
			byd_fps_timer_start(byd_fps_timer_data->byd_this_hrtimer_up, byd_timer_up);
			byd_fps_timer_data->byd_up_timer_flag = 1;
	}
}
void byd_fps_timer_up_stop(void)
{
	
	DBG_TIME("%s:timer up stop,byd_up_timer_flag =%d\n", __func__,byd_fps_timer_data->byd_up_timer_flag);
	
	byd_fps_timer_data->byd_up_timer_flag = 2;
	if((byd_fps_timer_data->byd_up_timer_flag == 1)||(byd_fps_timer_data->byd_up_timer_flag == 2)){
		byd_fps_timer_try_cancel(byd_fps_timer_data->byd_this_hrtimer_up);
	}
}
#endif
unsigned char byd_fps_get_timer_flag(void)
{
	return byd_fps_timer_data->byd_down_timer_flag;
}
// *******************************************************************************
// * Function    :  byd_fps_timer_start
// * Description :  the worker  in "byd_fps_workqueue",Bottom of ISR .
// * In          :  *work
// * Return      :  0--succeed.
// *******************************************************************************
int byd_fps_timer_start(struct hrtimer *timer, unsigned int byd_time)
{
	FP_DBG("byd_hrTimer:s=%d, ms=%d\n", byd_time/1000, byd_time%1000);
	hrtimer_start(timer, ktime_set((byd_time/1000), (byd_time%1000)*1000*1000), HRTIMER_MODE_REL);
	FP_DBG("byd_hrTimer:%d ms\n", byd_time);

	return 0;
}
int byd_fps_timer_try_cancel(struct hrtimer *timer)
{
	hrtimer_try_to_cancel(timer);
	FP_DBG("%s: byd Cancel hrTimer\n", __FUNCTION__);

	return 0;
}
// *******************************************************************************
// * Function    :  byd_fps_downif
// * Description :  check status of timer_flag to start or cancel timer.
// * In          :  void
// * Return      :  void
// *******************************************************************************
void byd_fps_downif(void)
{
	if((byd_fps_timer_data->byd_down_timer_flag == 0)||(byd_fps_timer_data->byd_down_timer_flag == 2)){
			
			DBG_TIME("%s:timer first start DOWN\n", __func__);
			
			byd_fps_timer_start(byd_fps_timer_data->byd_this_hrtimer_down, byd_timer_down);
			byd_fps_timer_data->byd_down_timer_flag = 1;
	}
	else {
		if(byd_fps_timer_data->byd_down_timer_flag == 1) {
			
			DBG_TIME("%s:timer stop\n", __func__);
			
			byd_fps_timer_try_cancel(byd_fps_timer_data->byd_this_hrtimer_down);
			
			DBG_TIME("%s:timer restart\n", __func__);
			
			byd_fps_timer_start(byd_fps_timer_data->byd_this_hrtimer_down, byd_timer_down);
			
			DBG_TIME("%s:timer restart end\n", __func__);
			
		}
	}
}

