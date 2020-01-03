#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>
#include <uart/uart_manager.h>
#include <zigbee/zigbee/zigbee_manager.h>
#include <zigbee/protocol/uart_protocol.h>


#define LOG_TAG                         "zigbee_test"

#define PORT_S1_DEVNAME                 "/dev/ttyS1"
#define PORT_S1_BAUDRATE                115200
#define PORT_S1_DATABITS                8
#define PORT_S1_PRARITY                 0
#define PORT_S1_STOPBITS                1



#define ZIGBEE_ROLE_COOR                0x00
#define ZIGBEE_ROLE_ROUTER              0x01
#define ZIGBEE_ROLE_ENDEV               0x02

#define ZIGBEE_BROADCAST                0x00
#define ZIGBEE_UNICAST                  0x01
#define ZIGBEE_GROUPCAST                0x02

#define ZIGBEE_PAN_ID                   0xFFFF
#define ZIGBEE_ROLE                     ZIGBEE_ROLE_ENDEV
#define ZIGBEE_CHANNEL                  0x19
#define ZIGBEE_POLL_RATE                5000                // 5s  sleep only used end device
#define ZIGBEE_TX_POWER                 3                   // dbm
#define ZIGBEE_JOIN_AGING               255                 // coor alway allow dev join
#define ZIGBEE_CAST_TYPE                ZIGBEE_BROADCAST
#define ZIGBEE_CAST_ADDR                0xFFFF              // broadcast addr





static char* status_str[] = {
    "none",
    "power on",
    "nwk created",
    "nwk join",
    "nwk leave"
};


static struct uart_zigbee_manager* zb_m;

static pthread_mutex_t cfg_mutex  = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cfg_cond    = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t join_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t join_cond   = PTHREAD_COND_INITIALIZER;


static void zb_uart_recv_cb(uint8_t cmd, uint8_t* const pl, uint16_t len);


int main(int argc, char *argv[]) {

    int16_t i,ret = -1;
    uint8_t send[10] = {0,1,2,3,4,5,6,7,8,9};
    uint8_t key[16] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,\
                       0x09,0x10,0x11,0x12,0x13,0x14,0x15,0x16};
    struct zb_uart_cfg_t uart_cfg;

    uart_cfg.devname   = PORT_S1_DEVNAME;
    uart_cfg.baudrate  = PORT_S1_BAUDRATE;
    uart_cfg.date_bits = PORT_S1_DATABITS;
    uart_cfg.parity    = PORT_S1_PRARITY;
    uart_cfg.stop_bits = PORT_S1_STOPBITS;

    zb_m = get_zigbee_manager();

    ret = zb_m->init(zb_uart_recv_cb, uart_cfg);
    if (ret < 0) {
        LOGE("zigbee init fail. err: %d\n", ret);
        return -1;
    }
    sleep(2);

    pthread_mutex_lock(&cfg_mutex);
    LOGI("hardware reset\n");
    ret = zb_m->reset();
    if (ret < 0) {
        LOGE("hardware reset fail, err: %d\r\n",ret);
    }
    pthread_cond_wait(&cfg_cond, &cfg_mutex);
    pthread_mutex_unlock(&cfg_mutex);

    pthread_mutex_lock(&cfg_mutex);
    LOGI("factory setting\n");
    ret = zb_m->factory();
    if (ret < 0) {
        LOGE("factory send fail, err: %d\r\n",ret);
        return -1;
    }
    pthread_cond_wait(&cfg_cond, &cfg_mutex);
    pthread_mutex_unlock(&cfg_mutex);

    LOGI("set tx power: %d dbm\n", ZIGBEE_TX_POWER);
    zb_m->set_tx_power(ZIGBEE_TX_POWER);
    if (ret < 0) {
        LOGE("factory send fail, err: %d\r\n",ret);
    }

    LOGI("set join aging: %d s\n", ZIGBEE_JOIN_AGING);
    ret = zb_m->set_join_aging(ZIGBEE_JOIN_AGING);
    if (ret < 0) {
        LOGE("set join aging send fail, err: %d\r\n",ret);
    }

    LOGI("set cast type: %d addr : 0x%04x \n", ZIGBEE_CAST_TYPE, ZIGBEE_CAST_ADDR);
    ret = zb_m->set_cast_type(ZIGBEE_CAST_TYPE, ZIGBEE_CAST_ADDR);
    if (ret < 0) {
        LOGE("set cast type send fail, err: %d\r\n",ret);
    }

    pthread_mutex_lock(&cfg_mutex);
    LOGI("set panid: 0x%04x\n",ZIGBEE_PAN_ID);
    ret = zb_m->set_panid(ZIGBEE_PAN_ID);
    if (ret < 0) {
        LOGE("set panid send fail, err: %d\r\n",ret);
    }
    pthread_cond_wait(&cfg_cond, &cfg_mutex);
    pthread_mutex_unlock(&cfg_mutex);


    pthread_mutex_lock(&cfg_mutex);
    LOGI("set channel: 0x%02x\n", ZIGBEE_CHANNEL);
    ret = zb_m->set_channel(ZIGBEE_CHANNEL);
    if (ret < 0) {
        LOGE("set channel send fail, err: %d\r\n",ret);
    }
    pthread_cond_wait(&cfg_cond, &cfg_mutex);
    pthread_mutex_unlock(&cfg_mutex);

    pthread_mutex_lock(&cfg_mutex);
    LOGI("set key: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\
 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
    key[0],key[1],key[2],key[3],key[4],key[5],key[6],key[7],key[8],
    key[9],key[10],key[11],key[12],key[13],key[14],key[15]);
    ret = zb_m->set_key(key,sizeof(key));
    if (ret < 0) {
        LOGE("set aes key fail, err: %d\r\n",ret);
    }
    pthread_cond_wait(&cfg_cond, &cfg_mutex);
    pthread_mutex_unlock(&cfg_mutex);


    if (ZIGBEE_ROLE == ZIGBEE_ROLE_ENDEV) {
        pthread_mutex_lock(&cfg_mutex);
        LOGI("set poll rate: %d ms\n",ZIGBEE_POLL_RATE);
        ret = zb_m->set_poll_rate(ZIGBEE_POLL_RATE);
        if (ret < 0) {
            LOGE("set poll rate send fail, err: %d\r\n",ret);
        }
        pthread_cond_wait(&cfg_cond, &cfg_mutex);
        pthread_mutex_unlock(&cfg_mutex);
    }

    LOGI("set rote: %d\n", ZIGBEE_ROLE);
    if (ZIGBEE_ROLE == ZIGBEE_ROLE_COOR) {
        pthread_mutex_lock(&cfg_mutex);
        ret = zb_m->set_role(ZIGBEE_ROLE);
        if (ret < 0) {
            LOGE("set role send fail, err: %d\r\n",ret);
        }
        pthread_cond_wait(&cfg_cond, &cfg_mutex);
        pthread_mutex_unlock(&cfg_mutex);
    } else {
        pthread_mutex_lock(&join_mutex);
        ret = zb_m->set_role(ZIGBEE_ROLE);
        if (ret < 0) {
            LOGE("set role send fail, err: %d\r\n",ret);
        }
        pthread_cond_wait(&join_cond, &join_mutex);
        pthread_mutex_unlock(&join_mutex);
    }


    ret = zb_m->get_info();
    if (ret < 0) {
        LOGE("get info send fail. err: %d\n",ret);
    }

    while(1) {
        ret = zb_m->ctrl(send,sizeof(send));
        if (ret < 0) {
            LOGI("ctrl send fail. err: %d\n",ret);
        } else {
            LOGI("app send: ");
            for (i = 0; i < sizeof(send); ++i) {
                printf(" 0x%x", send[i]);
            }
            printf("\r\n");
        }
        sleep(5);
    }

    zb_m->deinit();
    return 0;
}

static int8_t info_rsp_handle(uint8_t* info, uint16_t len)
{
#define K(x)  zb_info.key[x]

    struct zb_info_t zb_info = {0};

    if (info == NULL) {
        return -1;
    }

    GET_ZIGBEE_INFO(zb_info,info);

    LOGI("----------------------------------------------------\r\n");
    LOGI("role: 0x%x\r\n",zb_info.role);
    LOGI("panid: 0x%x\r\n",zb_info.panid);
    LOGI("short addr: 0x%x\r\n",zb_info.short_addr);
    LOGI("channel: 0x%02x%02x%02x%02x \r\n" ,zb_info.channel[3],zb_info.channel[2]
                                            ,zb_info.channel[1],zb_info.channel[0]);
    LOGI("secure key: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\
        0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\r\n",
        K(0),K(1),K(2),K(3),K(4),K(5),K(6),K(7),K(8),K(9),K(10),K(11),K(12),K(13),K(14),K(15));

    LOGI("join aging: %d %s\r\n",zb_info.join_aging, zb_info.join_aging == 0xFF? "s always" : "s");
    LOGI("group_id: 0x%x\r\n",zb_info.group_id);
    LOGI("send: type 0x%x\r\n",zb_info.send_type);
    LOGI("send_addr: 0x%x\r\n",zb_info.send_addr);
    LOGI("send_group_id: 0x%x\r\n",zb_info.send_group_id);
    LOGI("tx power: %d dbm\r\n",(int8_t)zb_info.tx_power);
    LOGI("poll rate: %d ms \n", zb_info.poll_rate);
    LOGI("----------------------------------------------------\r\n");

    return 0;
}

static void zb_uart_recv_cb(uint8_t cmd, uint8_t* const pl, uint16_t len)
{
    uint8_t nwk_status = DEV_STATUS_NONE;
    int16_t i;
    uint16_t id;

    LOGI("receive cmd : 0x%x\n", cmd);
    switch (cmd)
    {
        case UART_CMD_DATA_REPORT:
            LOGI("app recv: ");
            for (i = 0; i < len; ++i) {
                printf(" 0x%x", pl[i]);
            }
            printf("\r\n");
        break;

        case UART_CMD_STATUS_REPORT:
            GET_NWK_STATUS(id, nwk_status, pl);
            LOGI("   dev id: 0x%04x   status: %s\n", id,status_str[nwk_status]);
            if (nwk_status == DEV_STATUS_POWER_ON) {
                pthread_mutex_lock(&cfg_mutex);
                pthread_cond_signal(&cfg_cond);
                pthread_mutex_unlock(&cfg_mutex);
            } else if (nwk_status == DEV_STATUS_NWK_JOIN) {
                pthread_mutex_lock(&join_mutex);
                pthread_cond_signal(&join_cond);
                pthread_mutex_unlock(&join_mutex);
            }

        break;

        case UART_CMD_INFO_RSP:
            info_rsp_handle(pl, len);
        break;

        default:
        break;
    }
    return;
}
