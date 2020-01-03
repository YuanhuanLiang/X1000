/*
 * Copyright (C) MicroArray
 * MicroArray Fprint Driver Code
 * ioctl_cmd.h
 * Date: 2017-3-9
 * Version: v4.0.05
 * Author: guq
 * Contact: guq@microarray.com.cn
 */

#ifndef __MICROARRAY_IOCTL_CMD_H__
#define __MICROARRAY_IOCTL_CMD_H__

/**
 *  the old ioctl command, compatible for the old version
 */
#ifdef COMPATIBLE_VERSION3
#define IOCTL_DEBUG                         0x100   //调试信息      //debug message
#define IOCTL_IRQ_ENABLE                    0x101   //中断使能      //enable interrupt
#define IOCTL_SPI_SPEED                     0x102   //SPI速度      //spi speed
#define IOCTL_READ_FLEN                     0x103   //读帧长度(保留) the length of one frame
#define IOCTL_LINK_DEV                      0x104   //连接设备(保留) connect the device
#define IOCTL_COVER_NUM                     0x105   //材料编号      //the index of the material
#define IOCTL_GET_VDATE                     0x106   //版本日期      //the date fo the version

#define IOCTL_CLR_INTF                      0x110   //清除中断标志
#define IOCTL_GET_INTF                      0x111   //获取中断标志
#define IOCTL_REPORT_FLAG                   0x112   //上报标志
#define IOCTL_REPORT_KEY                    0x113   //上报键值
#define IOCTL_SET_WORK                      0x114   //设置工作
#define IOCTL_GET_WORK                      0x115   //获取工作
#define IOCTL_SET_VALUE                     0x116   //设值
#define IOCTL_GET_VALUE                     0x117   //取值
#define IOCTL_TRIGGER                       0x118   //自触发
#define IOCTL_WAKE_LOCK                     0x119   //唤醒上锁
#define IOCTL_WAKE_UNLOCK                   0x120   //唤醒解锁

#define IOCTL_SCREEN_ON                     0x121

#define IOCTL_KEY_DOWN                      0x121   //按下
#define IOCTL_KEY_UP                        0x122   //抬起
#define IOCTL_SET_X                         0x123   //偏移X
#define IOCTL_SET_Y                         0x124   //偏移Y
#define IOCTL_KEY_TAP                       0x125   //单击
#define IOCTL_KEY_DTAP                      0x126   //双击
#define IOCTL_KEY_LTAP                      0x127   //长按

#define IOCTL_ENABLE_CLK                    0x128
#define TRUE                                (1)
#define FALSE                               (0)
#endif

/*****************************************************************************/
#define MA_DRV_VERSION	                    (0x00004005)

#define MA_IOC_MAGIC                        'M'
#define MA_IOC_INIT                         _IOR(MA_IOC_MAGIC, 0, unsigned char)
#define TIMEOUT_WAKELOCK                    _IO(MA_IOC_MAGIC, 1)
#define SLEEP                               _IO(MA_IOC_MAGIC, 2)    //陷入内核
#define WAKEUP                              _IO(MA_IOC_MAGIC, 3)    //唤醒
#define ENABLE_CLK                          _IO(MA_IOC_MAGIC, 4)    //打开spi时钟
#define DISABLE_CLK                         _IO(MA_IOC_MAGIC, 5)    //关闭spi时钟
#define ENABLE_INTERRUPT                    _IO(MA_IOC_MAGIC, 6)    //开启中断上报
#define DISABLE_INTERRUPT                   _IO(MA_IOC_MAGIC, 7)    //关闭中断上报
#define TAP_DOWN                            _IO(MA_IOC_MAGIC, 8)
#define TAP_UP                              _IO(MA_IOC_MAGIC, 9)
#define SINGLE_TAP                          _IO(MA_IOC_MAGIC, 11)
#define DOUBLE_TAP                          _IO(MA_IOC_MAGIC, 12)
#define LONG_TAP                            _IO(MA_IOC_MAGIC, 13)

//version time
#define MA_IOC_VTIM                         _IOR(MA_IOC_MAGIC, 14, unsigned char)
//cover num
#define MA_IOC_CNUM                         _IOR(MA_IOC_MAGIC, 15, unsigned char)
//sensor type
#define MA_IOC_SNUM                         _IOR(MA_IOC_MAGIC, 16, unsigned char)
//user define the report key
#define MA_IOC_UKRP                         _IOW(MA_IOC_MAGIC, 17, unsigned char)

#define MA_KEY_UP                           _IO(MA_IOC_MAGIC,  18)  //nav up
#define MA_KEY_LEFT                         _IO(MA_IOC_MAGIC,  19)  //nav left
#define MA_KEY_DOWN                         _IO(MA_IOC_MAGIC,  20)  //nav down
#define MA_KEY_RIGHT                        _IO(MA_IOC_MAGIC,  21)  //nav right

#define MA_KEY_F14                          _IO(MA_IOC_MAGIC,  23)
#define SET_MODE                            _IOW(MA_IOC_MAGIC, 33, unsigned int)
#define GET_MODE                            _IOR(MA_IOC_MAGIC, 34, unsigned int)


#define ENABLE_IRQ                          _IO(MA_IOC_MAGIC,  31)
#define DISABLE_IRQ                         _IO(MA_IOC_MAGIC,  32)

/*
 * get the driver version,the version mapping in the u32 is the final  4+4+8,
 * ****(major verson number)  ****
 * ****(minor version number) *****
 * ****(revised version number)****
 * the front 16 byte is reserved.
 */
#define SCREEN_OFF                          _IO(MA_IOC_MAGIC,  36)
#define MA_IOC_GVER                         _IOR(MA_IOC_MAGIC, 35, unsigned int)
#define SCREEN_ON                           _IO(MA_IOC_MAGIC,  37)
#define SET_SPI_SPEED                       _IOW(MA_IOC_MAGIC, 38, unsigned int)

//for fingerprintd
#define WAIT_FACTORY_CMD                    _IO(MA_IOC_MAGIC,  39)
//for factory test
#define WAKEUP_FINGERPRINTD                 _IO(MA_IOC_MAGIC,  40)
//for factory test
#define WAIT_FINGERPRINTD_RESPONSE          _IOR(MA_IOC_MAGIC, 41, unsigned int)
//for fingerprintd
#define WAKEUP_FACTORY_TEST_SEND_FINGERPRINTD_RESPONSE         \
                                            _IOW(MA_IOC_MAGIC, 42, unsigned int)

#define WAIT_SCREEN_STATUS_CHANGE           _IOR(MA_IOC_MAGIC, 43, unsigned int)
#define GET_INTERRUPT_STATUS                _IOR(MA_IOC_MAGIC, 44, unsigned int)
#define SYNC                                _IO(MA_IOC_MAGIC,  45)
#define SYNC2                               _IO(MA_IOC_MAGIC,  46)
#define GET_SCREEN_STATUS                   _IOR(MA_IOC_MAGIC, 47, unsigned int)

#endif /* __IOCTL_CMD_H__ */

