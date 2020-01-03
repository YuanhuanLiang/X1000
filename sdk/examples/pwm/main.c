/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Thu 15 Dec 2016 04:05:47 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <utils/log.h>
#include <utils/assert.h>
#include <pwm/pwm_manager.h>

#define LOG_TAG "test_pwm"

/*
 * Variables
 */
const int duty[] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     9, 10, 11, 13, 15, 17, 19, 21,
    23, 25, 27, 29, 31, 33, 35, 38,
    41, 44, 47, 50, 53, 56, 59, 62,
    66, 70, 74, 79, 84, 89, 94, 100,
};

uint32_t delay_us = 40000;

/*
 * Functions
 */
void print_usage() {
    printf("Usage:\n \
        test_pwm [OPTIONS]\n \
            a pwm test demo\n\n \
        OPTIONS:\n \
            -c   ----> select PWM channel\n \
            -f   ----> set PWM frequency\n \
            -n   ----> set cycle times\n \
            -t   ----> set delay_us value\n \
            -h   ----> get help message\n\n \
        eg:\n \
        test_pwm -c 2   ----> select PWM channel 2\n \
        test_pwm -n 20  ----> set cycle_times = 20\r\n");
}

static void breathing_lamp(struct pwm_manager *pwm, enum pwm id, int cycle_times) {
    int i;

    while(cycle_times--) {
        for(i = 0; i < sizeof(duty)/sizeof(int); i++) {
            pwm->setup_duty(id, duty[i]);
            usleep(delay_us);
        }

        for(i = sizeof(duty)/sizeof(int) - 1; i >= 0; i--) {
            pwm->setup_duty(id, duty[i]);
            usleep(delay_us);
        }
    }
}

int main(int argc, char *argv[])
{
    int oc;
    int cycle_times = 10;
    uint32_t freq = 30000;
    enum pwm pwm_id = 0;
    struct pwm_manager *pwm;

    while(1) {
        oc = getopt(argc, argv, "hc:f:n:t:");
        if(oc == -1)
            break;

        switch(oc) {
        case 'c':
            pwm_id = atoi(optarg);
            break;

        case 'f':
            freq = atoi(optarg);
            break;

        case 'n':
            cycle_times = atoi(optarg);
            break;

        case 't':
            delay_us = atoi(optarg);
            break;

        case 'h':
        default:
            print_usage();
            return 1;
        }
    }

    /* 获取操作PWM的句柄 */
    pwm = get_pwm_manager();

    /* 初始化PWM通道,设置有效电平 */
    pwm->init(pwm_id, ACTIVE_LOW);
    pwm->setup_freq(pwm_id, freq);

    /* 模拟呼吸灯效果 */
    breathing_lamp(pwm, pwm_id, cycle_times);

    pwm->deinit(pwm_id);
    return 0;
}
