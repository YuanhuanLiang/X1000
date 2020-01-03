/*
 * gpio_ir.h
 *
 *  Created on: 2015-5-10
 *      Author: robinson
 */

#ifndef _CHD_GPIO_DRIVER_H_
#define _CHD_GPIO_DRIVER_H_

#define DEBUG 0

#define GPIO_MAJOR 				255		// device major number
#define GPIO_DEVNAME			"gpio"
#define GPIO_NUMBER				3		// C150 chird used gpio number


#define GPIO_SET_DIR_IN			0xD001	
#define GPIO_SET_DIR_OUT		0xD002
#define GPIO_READ_PORT			0xD012
#define GPIO_WRITE_PORT			0xD013
#define GPIO_LED_SET			0xD024

typedef struct{
	int port;
	unsigned int value;
}CHD_GPIO_PORT_INFO;


#define GPIO_LED_INFINITY	4000
typedef struct {
	int gpio;				//gpio number 
	unsigned int on;		//interval of led on
	unsigned int off;		//interval of led off
	unsigned int blinks;	//number of blinking cycles
	unsigned int rests;		//number of break cycles
	unsigned int times;		//blinking times
}CHD_GPIO_LED_INFO;


#endif /* GPIO_IR_H_ */

