/*************************************************************************
    > Filename: main.c
    >   Author: Wang Qiuwei
    >    Email: qiuwei.wang@ingenic.com / panddio@163.com
    > Datatime: Tue 31 Oct 2017 02:16:19 PM CST
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
#include <network/network_manager.h>

#define LOG_TAG     "test_network"
#define MAC_SIZE    18
#define IP_SIZE     16


int main(int argc, char *argv[])
{
    char *ifname;
    int32_t retval;
    uint32_t ip;
    uint8_t mask[IP_SIZE] = {0};
    uint8_t brd[IP_SIZE] = {0};
    uint8_t mac[MAC_SIZE] = {0};
    enum if_state if_sta;
    struct network_manager *net;

    argc--, argv++;
    if (argc == 0) {
        printf("Please specify the ifname\n");
        return -1;
    }

    ifname = argv[0];

    net = get_network_manager();
    if (net->init(ifname) < 0) {
        LOGE("Failed to init %s\n", ifname);
        return -1;
    }

    retval = net->get_if_state(ifname);
    if (retval < 0) {
        LOGE("Failed to get %s state\n", ifname);
        return retval;
    }
    printf("%s \nSTA: %s\n", ifname, retval == IF_RUNNING ? \
                        "running" : retval == IF_UP? "up" : "down");
    if_sta = retval;
    if (if_sta == IF_RUNNING) {
        retval = net->get_if_ipaddr(ifname, (uint8_t *)&ip, 0);
        if (retval < 0) {
            LOGE("Failed to get %s ip\n", ifname);
            return retval;
        }
        printf("IP: %s\n", inet_ntoa(*(struct in_addr *)&ip));

        retval = net->get_if_netmask(ifname, mask, 1);
        if (retval < 0) {
            LOGE("Failed to get %s mask\n", ifname);
            return retval;
        }
        printf("MASK: %s\n", mask);

        retval = net->get_if_bcast(ifname, brd, 1);
        if (retval < 0) {
            LOGE("Failed to get %s brd\n", ifname);
            return retval;
        }
        printf("BRD: %s\n", brd);
    }

    retval = net->get_if_hwaddr(ifname, mac, 2);
    if (retval < 0) {
        LOGE("Failed to get %s mac\n", ifname);
        return retval;
    }
    printf("MAC: %s\n", mac);

    while(--argc > 0) {
        argv++;
        if (strcmp(argv[0], "up") == 0) {
            retval = net->set_if_state(ifname, IF_UP);
            if (retval < 0) {
                LOGE("Failed to set %s up\n", ifname);
                return retval;
            }
        } else if (strcmp(argv[0], "down") == 0) {
            retval = net->set_if_state(ifname, IF_DOWN);
            if (retval < 0) {
                LOGE("Failed to set %s down\n", ifname);
                return retval;
            }
        }
    }

    net->deinit(ifname);
    return 0;
}
