/*
 *  测试方法:
 *  在linux终端下执行以下命令
 *      启动测试程序
 *          /root/export/test_usb_switch
 *
 */
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <usb/usb_manager.h>
#define  DEV_HID                                   "/dev/hidg0"
#define  DEV_CDC_ACM                        "/dev/ttyGS0"
#define  METHOD_1                       0
#define  METHOD_2                       1
#define METHOD                  METHOD_2
int main(int argc, const char *argv[])
{
#if METHOD == METHOD_1
    struct usb_device_manager *hid= NULL;
    struct usb_device_manager *cdc_acm = NULL;
    int retval;

    printf("testing method 1\n");
    hid = get_usb_device_manager();
    cdc_acm = get_usb_device_manager();
    /* 切换测试 hid -> cdc */
    printf("switch from hid to cdc...\n");
    retval = hid->init(DEV_HID);
    if (retval < 0) {
        printf("USB hid init failed \n");
        return -1;
    }
    hid->deinit(DEV_HID);

    retval = cdc_acm->init(DEV_CDC_ACM);
    if (retval < 0) {
        printf("USB cdc acm init failed \n");
        return -1;
    }
    cdc_acm->deinit(DEV_CDC_ACM);

     /* 切换测试 cdc -> hid */
    printf("switch from cdc to hid...\n");
    retval = cdc_acm->init(DEV_CDC_ACM);
    if (retval < 0) {
        printf("USB cdc acm init failed \n");
        return -1;
    }
    cdc_acm->deinit(DEV_CDC_ACM);

    retval = hid->init(DEV_HID);
    if (retval < 0) {
        printf("USB hid init failed \n");
        return -1;
    }
    hid->deinit(DEV_HID);
    return 0;
#elif METHOD == METHOD_2
    struct usb_device_manager *device= NULL;
    int retval;
    device = get_usb_device_manager();

    printf("testing method 2\n");
    retval = device->init(DEV_HID);
    if (retval < 0) {
        printf("USB device init failed on %s\n", DEV_HID);
        return -1;
    }
    printf("switch from hid to cdc...\n");
    /* 切换测试 hid -> cdc */
    retval = device->switch_func(DEV_CDC_ACM, DEV_HID);
    if (retval < 0) {
        printf("USB switch failed \n");
        return -1;
    }
    printf("switch from cdc to hid...\n");
    /* 切换测试 cdc ->  hid*/
    retval = device->switch_func(DEV_HID, DEV_CDC_ACM);
    if (retval < 0) {
        printf("USB switch failed \n");
        return -1;
    }

    device->deinit(DEV_HID);
    return 0;
#endif
}
