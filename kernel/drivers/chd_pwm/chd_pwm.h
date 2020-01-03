/*
 * gpio_ir.h
 *
 *  Created on: 2015-5-10
 *      Author: robinson
 */

#ifndef _CHD_PWM_DRIVER_H_
#define _CHD_PWM_DRIVER_H_


#define PWM_MAJOR 		250		// device major number
#define PWM_DEVNAME		"chird_pwm"
#define PWM_NUMBER 		5

#define PWM_SET			0xD101
#define PWM_GET			0xD102

typedef struct pwn_ctrl{
	int pwmidx;		// 索引pwm
	int frequency;	// 频率
	int duty;       // 占空比
}CHD_PWM_INFO;

#endif /* PWM_IR_H_ */

