#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/wlan_plat.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "board_base.h"

static char *wifi_mac_str               = NULL;
static struct wifi_platform_data        bcmdhd_wlan_pdata;
#define WIFIMAC_ADDR_PATH               "/etc/oeminfo/mac"

/*
 * the MAC file format: 1234567890ab
 */
static int get_wifi_mac_addr(unsigned char* buf)
{
    struct file *fp = NULL;
    mm_segment_t fs;
    unsigned char source_addr[18];
    unsigned char tmp_addr[3];
    loff_t pos = 0;
    int i = 0;
    int ret = 0;
    int num = 0;

    if(wifi_mac_str != NULL) {

        printk("%s:get WiFi MAC from command line\n", __func__);
        tmp_addr[2] = '\0';
        for (num = 0; num <= 10; num = num + 2) {
            memcpy(tmp_addr, wifi_mac_str + num, 2);
            buf[i] = (char)simple_strtol(tmp_addr, NULL, 16);
            i++;
        }
        printk("buf = %02x:%02x:%02x:%02x:%02x:%02x \n",
                buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        return 0;
    } else {

        printk("%s:get WiFi MAC from flash.\n", __func__);
        fp = filp_open(WIFIMAC_ADDR_PATH, O_RDONLY,  0444);
        if (IS_ERR(fp)) {
            printk("Can not access wifi mac file : %s\n",WIFIMAC_ADDR_PATH);
            return -EFAULT;
        }

        fs = get_fs();
        set_fs(KERNEL_DS);
        vfs_read(fp, source_addr, 18, &pos);

        source_addr[17] = '\0';
        for (i = 0; i < strlen(source_addr); i++) {
            source_addr[i] = toupper(source_addr[i]);
        }

        tmp_addr[2] = '\0';
        for (i=0; i<6; i++) {
            memcpy(&tmp_addr[0], &source_addr[i*2], 2);
            buf[i] = simple_strtoul(tmp_addr, NULL, 16);
            printk("wifi mac %02x\n", buf[i]);
        } //end of for(...

        set_fs(fs);
        filp_close(fp, NULL);
    }


    /*
     * check if invalid or not:
     * FF:FF:FF:FF:FF:FF
     * 00:00:00:00:00:00
     */
    if (buf[0] == 0x00 || buf[0] == 0xFF) {
        for (i=1; i<6; i++) {
            if(buf[0] != buf[i])
                break;
        }/* end of for */

        if (i == 6) {
            printk("wifi MAC file is invalid, use default.\n");
            ret = EFAULT;
        }
    } /* end of if( */

	return ret;
}

static struct resource wlan_resources[] = {
    [0] = {
        .start = GPIO_WIFI_WAKE,
        .end = GPIO_WIFI_WAKE,
        .name = "bcmdhd_wlan_irq",
        .flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE,
    },
};
static struct platform_device wlan_device = {
    .name   = "bcmdhd_wlan",
    .id     = 1,
    .dev    = {
        .platform_data = &bcmdhd_wlan_pdata,
    },
    .resource	= wlan_resources,
    .num_resources	= ARRAY_SIZE(wlan_resources),
};

static int __init get_wifi_mac_addr_from_cmdline(char *str)
{
    wifi_mac_str = str;
    return 1;
}

__setup("wifi_mac=", get_wifi_mac_addr_from_cmdline);

static int __init wlan_device_init(void)
{
    int ret;
    memset(&bcmdhd_wlan_pdata, 0, sizeof(bcmdhd_wlan_pdata));

    bcmdhd_wlan_pdata.get_mac_addr = get_wifi_mac_addr;

    if (wifi_mac_str == NULL) {
        wlan_device.dev.platform_data = &bcmdhd_wlan_pdata;
    }
    ret = platform_device_register(&wlan_device);

    return ret;
}

late_initcall(wlan_device_init);
