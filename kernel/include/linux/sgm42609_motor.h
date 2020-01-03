#ifndef __SGM42609_MOTOR_H__
#define __SGM42609_MOTOR_H__

struct sgm42609_motor_platform_data {
	int power_gpio;
	int power_active_level;
	int in1_gpio;
	int in2_gpio;
	int fault_gpio;
};

#endif

