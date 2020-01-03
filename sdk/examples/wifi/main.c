/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Fri 10 Nov 2017 04:53:26 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>
#include <wpa_cli/wpa_cli.h>
#include <wifi/wifi_manager.h>

#define LOG_TAG    "test_wifi"
#define MAC_SIZE   18
#define IP_SIZE    16

#if 1
static const char short_options[] = "afhc:";
static const struct option long_options[] = {
    { "all-info",      0,    NULL,    'a' },
    { "foreground",    0,    NULL,    'f' },
    { "help",          0,    NULL,    'h' },
    { "connect",       1,    NULL,    'c' },
    { 0, 0, 0, 0 }
};

static void usage(FILE *fp, char *argv[])
{
    fprintf(fp,
            "\nUsage: %s [options]\n"
            "Options:\n"
            "-a | --all-info      Similar to the ifconfig command\n"
            "-f | --foreground    Don't quit after processing the command line\n"
            "-h | --help          Show the usage\n"
            "-c | --connect       Connect to new ssid\n"
            "eg:\n"
            "%s -f -a\n"
            "%s -c \"ssid=xxx psk=xxx\"\n"
            "\n",
            argv[0], argv[0], argv[0]);
}

static void dump_ssid_info(struct ssid_info *info)
{
    uint8_t i;
    enum encryption encryption;

    printf("======== AP: %s info ========\n", info->ssid);
        printf("device\t  signal_level\t  bssid\t\t\tflags\n");
    for(i = 0; i < info->count; i++) {
        encryption = info->ap[i].encryption;
        printf("%d\t  %d\t\t  %s\t%s\n",
               i, info->ap[i].signal_level,
               info->ap[i].bssid,
               encryption == OPEN ? "OPEN": encryption == WPA_PSK ? "WPA_PSK" : \
               encryption == WPA2_PSK ? "WPA2_PSK" : "WPA_WPA2_PSK");
    }
}

int main(int argc, char *argv[])
{
    int32_t retval = 0;
    uint8_t ip[IP_SIZE] = {0};
    uint8_t mask[IP_SIZE] = {0};
    uint8_t bcast[IP_SIZE] = {0};
    uint8_t mac[MAC_SIZE] = {0};
    int8_t foreground = 0;
    char cmd[256] = {0};
    char *pos1 = NULL, *pos2 = NULL;
    char *ssid = NULL, *psk = NULL;
    char *msg = NULL;
    size_t msg_len = 0;
    struct ssid_info info;
    struct wifi_manager *wifi = NULL;

    if (argc < 2) {
        usage(stdout, argv);
        return 0;
    }

    wifi = get_wifi_manager();

    if (wifi->init() < 0) {
        LOGE("Failed to init wifi manager\n");
        return -1;
    }

    if (wifi->set_if_state(IF_UP) < 0) {
        LOGE("Failed to set if_state UP\n");
        goto err_exit;
    }
    wpa_cli_cmd_request(cmd, NULL, NULL);
    usleep(200*1000);

    while(1) {
        int oc;
        oc = getopt_long(argc, argv, \
                         short_options, long_options, \
                         NULL);
        if (-1 == oc)
            break;

        switch(oc) {
        case 'a':
            retval = wifi->get_hwaddr(mac, 1);
            if (retval < 0) {
                LOGE("Failed to get hwaddr\n");
                goto err_exit;
            }
            retval = wifi->get_ipaddr(ip, 1);
            if (retval < 0) {
                LOGE("Failed to get ipaddr\n");
                goto err_exit;
            }
            retval = wifi->get_netmask(mask, 1);
            if (retval < 0) {
                LOGE("Failed to get netmask\n");
                goto err_exit;
            }
            retval = wifi->get_bcast(bcast, 1);
            if (retval < 0) {
                LOGE("Failed to get bcast\n");
                goto err_exit;
            }
            fprintf(stdout,
                    "----------------------------\n"
                    "硬件地址: %s\n"
                    "inet地址: %s\n"
                    "广播地址: %s\n"
                    "子网掩码: %s\n"
                    "----------------------------\n",
                    mac, ip, bcast, mask);
            break;
        case 'f':
            foreground = 1;
            break;
        case 'c':
            pos1 = strstr(optarg, "ssid=");
            if (pos1 == NULL) {
                LOGE("The paramters of -c is invalid. L%d\n", __LINE__);
                retval = -1;
                goto err_exit;
            }
            ssid = pos1 + 5;

            pos2 = strstr(optarg, "psk=");
            if (pos2 == NULL) {
                LOGE("The paramters of -c is invalid. L%d\n", __LINE__);
                retval = -1;
                goto err_exit;
            }
            psk = pos2 + 4;

            if (pos2 > pos1)
                *(pos2 - 1) = 0;
            else
                *(pos1 - 1) = 0;

            retval = wifi->connect_to_new_ssid(ssid, psk);
            if (retval < 0) {
                LOGE("Failed to connect, new ssid=%s, psk=%s\n", ssid, psk);
                goto err_exit;
            }
            break;
        case 'h':
        default:
            usage(stderr, argv);
            exit(EXIT_SUCCESS);
        }
    }

    /**
     * 示例：获取当前环境名为"test"的AP信息
     */
    bzero(&info, sizeof(struct ssid_info));
    strcpy(info.ssid, "test");
    retval = wifi->get_ssid_info(&info);
    if (retval > 0)
        dump_ssid_info(&info);

    if (!foreground) {
        wifi->deinit();
        exit(EXIT_SUCCESS);
    }

    while (1) {
        bzero(cmd, sizeof(cmd));
        printf("> ");
        fflush(stdout);
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strlen(cmd) - 1] = '\0';

        if (strcmp("q", cmd) == 0 ||
            strcmp("quit", cmd) == 0) {
            break;
        } else if (strcmp("sleep", cmd) == 0) {
            wifi->disconnect_and_suspend();
            system("echo mem > /sys/power/state");
            /**
             * Restore from sleep
             */
            retval = wifi->resume_and_reconnect();
            if (retval < 0) {
                LOGE("Failed to reconnect wifi after wakeup\n");
                break;
            }
        } else {
            if (wpa_cli_cmd_request(cmd, &msg, &msg_len) == 0) {
                /**
                 * CMD request successful, print the reply message
                 */
                printf("--------------------------------\n");
                printf("Reply len = %d, msg:\n\n", msg_len);
                printf("%s\n", msg);
                printf("--------------------------------\n");
            }
        }
    }

err_exit:
    wifi->deinit();
    return retval;
}
#else
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
struct wifi_manager *wifi = NULL;
unsigned char wifi_connect_ok = 0;
unsigned char wifi_connect_timeout = 0;
unsigned char wifi_resume_from_sleep = 0;

void *wifi_connect_thread(void *arg)
{
    int32_t retval;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    sleep(1);
    retval = wifi->connect_to_defaut_ssid();
    if (retval < 0) {
        LOGE("Connect to default ssid failed\n");
        return NULL;
    }
    pthread_mutex_lock(&mutex);
    wifi_connect_ok = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    while(1) {
        pthread_mutex_lock(&mutex);
        while(!wifi_resume_from_sleep)
            pthread_cond_wait(&cond, &mutex);

        retval = wifi->resume_and_reconnect();
        if (retval < 0) {
            LOGE("WIFI resume from sleep, but reconnect timeout\n");
            LOGE("WIFI resume from sleep, but reconnect timeout\n");
            LOGE("WIFI resume from sleep, but reconnect timeout\n");
            wifi_connect_timeout = 1;
        } else if (retval == 1) {
            LOGE("WIFI resume from sleep, but reconnect be cancelled\n");
            LOGE("WIFI resume from sleep, but reconnect be cancelled\n");
            LOGE("WIFI resume from sleep, but reconnect be cancelled\n");
        } else {
            wifi_connect_ok = 1;
            wifi_resume_from_sleep = 0;
            LOGD("WIFI resume from sleep and reconnect success\n");
            LOGD("WIFI resume from sleep and reconnect success\n");
            LOGD("WIFI resume from sleep and reconnect success\n");
        }

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    uint8_t ip[IP_SIZE] = {0};
    uint8_t mask[IP_SIZE] = {0};
    uint8_t bcast[IP_SIZE] = {0};
    uint8_t mac[MAC_SIZE] = {0};
    int32_t retval;
    pthread_t thread;

    wifi = get_wifi_manager();
    if (wifi->init() < 0) {
        LOGE("Failed to init wifi manager\n");
        return -1;
    }

    pthread_create(&thread, NULL, (void *)wifi_connect_thread, NULL);
    pthread_detach(thread);

    while(1) {
        pthread_mutex_lock(&mutex);
        while(!wifi_connect_ok) {
            if (!wifi_connect_timeout)
                pthread_cond_wait(&cond, &mutex);
            else {
                #if 0
                pthread_cancel(thread);
                pthread_cond_signal(&cond);
                pthread_mutex_unlock(&mutex);
                return -1;
                #else
                goto entry_sleep;
                #endif
            }
        }

        /**
         * Do something here
         */
        retval = wifi->get_hwaddr(mac, 1);
        if (retval < 0) {
            LOGE("Failed to get hwaddr\n");
            goto entry_sleep;
        }
        retval = wifi->get_ipaddr(ip, 1);
        if (retval < 0) {
            LOGE("Failed to get ipaddr\n");
            goto entry_sleep;
        }
        retval = wifi->get_netmask(mask, 1);
        if (retval < 0) {
            LOGE("Failed to get netmask\n");
            goto entry_sleep;
        }
        retval = wifi->get_bcast(bcast, 1);
        if (retval < 0) {
            LOGE("Failed to get bcast\n");
            goto entry_sleep;
        }
        fprintf(stdout,
                "----------------------------\n"
                "硬件地址: %s\n"
                "inet地址: %s\n"
                "广播地址: %s\n"
                "子网掩码: %s\n"
                "----------------------------\n",
                mac, ip, bcast, mask);

        sleep(3);

        /**
         * Entry sleep
         */
entry_sleep:
        wifi->dhcpc_request_ip_cancel();
        wifi->disconnect_and_suspend();
        system("echo mem > /sys/power/state");

        /**
         * Resume from sleep
         */
        wifi_connect_ok = 0;
        wifi_connect_timeout = 0;
        wifi_resume_from_sleep = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}
#endif
