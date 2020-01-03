/*************************************************************************
	> Filename: main.c
	>   Author: Wang Qiuwei
	>    Email: qiuwei.wang@ingenic.com / panddio@163.com
	> Datatime: Fri 29 Sep 2017 11:45:18 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>

#include <types.h>
#include <utils/log.h>
#include <utils/assert.h>
#include <lock_cylinder/lock_cylinder_manager.h>

#define LOG_TAG "lock_cylinder_testunit"

enum action_t {
    REGISTER_KEY,
    AUTHENTICATE_KEY,
};

struct lock_cylinder_data {
    int8_t is_key_inserted;
    uint8_t is_finished;
    uint8_t romid[8];
    pthread_mutex_t lock;
};

static struct lock_cylinder_manager *lock_cylinder;

static const char short_options[] = "arc:h";

static const struct option long_options[] = {
    { "authenticate",  0,      NULL,           'a' },
    { "register",      0,      NULL,           'r' },
    { "count",         1,      NULL,           'c' },
    { "help",          0,      NULL,           'h' },
    { 0, 0, 0, 0 }
};

static void usage(FILE *fp, int argc, char *argv[])
{
    fprintf(fp,
              "\nUsage: %s [options]\n"
              "Options:\n"
              "-a | --authenticate    Authenticate key\n"
              "-r | --register        Register key\n"
              "-c | --count           Number of cycles\n"
              "-h | --help            Print this message\n"
              "\n", argv[0]);
}
/**
 * Functions
 */
static void key_detected_handler(bool is_key_inserted, void *arg)
{
    struct lock_cylinder_data *data = (struct lock_cylinder_data *)arg;

    pthread_mutex_lock(&data->lock);
    data->is_key_inserted = is_key_inserted;
    if (is_key_inserted)
        data->is_finished = false;
    printf("is_key_inserted=%d\n", data->is_key_inserted);
    pthread_mutex_unlock(&data->lock);
}

int main(int argc, char *argv[])
{
    struct lock_cylinder_data *data;
    enum action_t action = REGISTER_KEY;
    int32_t count = 1;

    while(1) {
        int oc;

        oc = getopt_long(argc, argv, \
                        short_options, long_options, \
                        NULL);
        if(-1 == oc)
            break;

        switch(oc) {
        case 'a':
            action = AUTHENTICATE_KEY;
            break;
        case 'r':
            action = REGISTER_KEY;
            break;
        case 'c':
            count = atoi(optarg);
            break;
        case 'h':
            usage(stdout, argc, argv);
            break;
        default:
            usage(stderr, argc, argv);
            exit(EXIT_FAILURE);
        }
    }

    data = (struct lock_cylinder_data *)malloc(sizeof(struct lock_cylinder_data));
    if (!data) {
        LOGE("Failed to alloc memory\n");
        return -1;
    }

    bzero(data, sizeof(struct lock_cylinder_data));
    pthread_mutex_init(&data->lock, NULL);
    data->is_key_inserted = false;
    data->is_finished = false;

    lock_cylinder = get_lock_cylinder_manager();
    if (lock_cylinder->init(key_detected_handler, (void *)data) < 0) {
        return -1;
    }

    printf("\nBefore main loop\n");

    while(count) {
        pthread_mutex_lock(&data->lock);
        //printf("key statue=%d\n", lock_cylinder->get_keystatue());

        if (data->is_key_inserted && !data->is_finished) {
            if (lock_cylinder->get_romid(data->romid) == 0) {
                printf("------------------------\n");
                for (int i = 0; i < 8; i++) {
                    printf("romid[%d]=0x%02x\n", i, data->romid[i]);
                }
                printf("------------------------\n");

            }

            switch(action) {
            case REGISTER_KEY:
                if (lock_cylinder->register_key() == 0) {
                    printf("Register key OK\n");
                }
                break;
            case AUTHENTICATE_KEY:
                if (lock_cylinder->authenticate_key() == 0) {
                    printf("Authenticate key OK\n");
                    lock_cylinder->power_ctrl(false); /* 允许转动锁心 */
                }
                break;
            default:
                LOGE("Unknown action\n");
            }

            count--;
            data->is_finished = true;
        }

        pthread_mutex_unlock(&data->lock);
        usleep(10 * 1000);
    }

    pthread_mutex_destroy(&data->lock);
    free(data);
    lock_cylinder->deinit();
    return 0;
}
