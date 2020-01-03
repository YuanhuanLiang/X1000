/*
 *  Copyright (C) 2017, Monk Su<rongjin.su@ingenic.com, MonkSu@outlook.com>
 *
 *  Ingenic Linux plarform SDK project
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <utils/log.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fb/fb_manager.h>
#include <utils/image_convert.h>
#include <camera_v4l2/camera_v4l2_manager.h>
#include <uvc/uvc.h>


/*
 * Macro
 */
#define LOG_TAG                         "camera_v4l2"


#define DEFAULT_WIDTH                    640
#define DEFAULT_HEIGHT                   480
#define DEFAULT_BPP                      16
#define DEFAULT_NBUF                     1
#define DEFAULT_ACTION                   PREVIEW_PICTURE
#define DEFAULT_DOUBLE_CHANNEL           0
#define DEFAULT_CHANNEL                  CHANNEL_SEQUEUE_COLOR

#define PREVIEW_WIDTH                    640     //resolution of picture cast to screen
#define PREVIEW_HEIGHT                   480
#define PREVIEW_START_PORT_X             0      //picture start point on screen
#define PREVIEW_START_PORT_Y             70
#define PREVIEW_ROTATE_DEGREE            DEGREE_0

#define MK_PIXEL_FMT(x)                                 \
        do{                                             \
            x.rbit_len = fbm->get_redbit_length();      \
            x.rbit_off = fbm->get_redbit_offset();      \
            x.gbit_len = fbm->get_greenbit_length();    \
            x.gbit_off = fbm->get_greenbit_offset();    \
            x.bbit_len = fbm->get_bluebit_length();     \
            x.bbit_off = fbm->get_bluebit_offset();     \
        }while(0)


typedef enum {
    CHANNEL_SEQUEUE_COLOR             = 0x00,
    CHANNEL_SEQUEUE_BLACK_WHITE       = 0x01
} chselect_m;

uint32_t CONFIG_UVC;

typedef enum {
    CAPTURE_PICTURE = 0,
    PREVIEW_PICTURE,
} action_m;

static struct camera_v4l2_manager* cimm;
static struct fb_manager* fbm;
struct uvc_device *udev;
static struct _capt_op{
    action_m action;            // action capture or preview
    chselect_m channel;         // select operate ch for action
    char double_ch;             // channel num
    uint8_t* filename;          // picture save name when action is capture picture
    uint16_t *ppbuf;            // map lcd piexl buf
}capt_op;



static const char short_options[] = "c:phmn:rux:y:ds:f:";

static const struct option long_options[] = {
    { "help",       0,      NULL,           'h' },
    { "mmap",       0,      NULL,           'm' },
    { "nbuf",       1,      NULL,           'n' },
    { "read",       0,      NULL,           'r' },
    { "userp",      0,      NULL,           'u' },
    { "width",      1,      NULL,           'x' },
    { "height",     1,      NULL,           'y' },
    { "capture",    1,      NULL,           'c' },
    { "preview",    0,      NULL,           'p' },
    { "double",     0,      NULL,           'd' },
    { "select",     1,      NULL,           's' },
    { "function",   1,      NULL,           'f' },
    { 0, 0, 0, 0 }
};

/*
 * Functions
 */
static void usage(FILE *fp, int argc, char *argv[])
{
    fprintf(fp,
             "\nUsage: %s [options]\n"
             "Options:\n"
             "-h | --help          Print this message\n"
             "-m | --mmap          Use memory mapped buffers\n"
             "-n | --nbuf          Request buffer numbers\n"
             "-r | --read          Use read() calls\n"
             "-u | --userp         Use application allocated buffers\n"
             "-x | --width         Capture width\n"
             "-y | --height        Capture height\n"
             "-c | --capture       Take picture, +filename\n"
             "-p | --preview       Preview picture to LCD\n"
             "-d | --double        Used double camera sensor\n"
             "-s | --select        Select operate channel, 0(color) or 1(black&white)\n"
             "-f | --function      open uvc or close uvc function\n"
             "\n", argv[0]);
}

static void frame_receive_cb(uint8_t* buf, uint32_t pixelformat, uint32_t width, uint32_t height, uint32_t seq)
{
    int ret;
    uint8_t *yuvbuf     = buf;
    uint8_t *rgbbuf     = NULL;
    uint8_t *convertbuf = NULL;
    struct rgb_pixel_fmt pixel_fmt;
    int rgb_w = width,rgb_h = height;

    if (buf == NULL) {
        LOGE("Buf is NULL\n");
        return;
    }
    if(CONFIG_UVC == 1) {

    image_load(udev, buf, udev->width, udev->height);
    fd_set  fdsu;
    FD_ZERO(&fdsu);

    /* We want both setup and data events on UVC interface.. */
    FD_SET(udev->uvc_fd, &fdsu);
    fd_set dfds = fdsu;
    fd_set efds = fdsu;
    ret = select(udev->uvc_fd + 1, NULL, &dfds, &efds, NULL);

    if (FD_ISSET(udev->uvc_fd, &efds))
    uvc_events_process(udev);
    if (FD_ISSET(udev->uvc_fd, &dfds))
    uvc_video_process(udev);
    }
    else {
    if (capt_op.double_ch != 1) {
        seq = capt_op.channel;
    }

    if (seq == capt_op.channel) {
        if (capt_op.action == PREVIEW_PICTURE &&
           (PREVIEW_HEIGHT != height || PREVIEW_WIDTH != width)){
            convertbuf = (uint8_t*)malloc(PREVIEW_WIDTH*PREVIEW_HEIGHT*2);
            if (convertbuf == NULL) {
                LOGE("Failed to malloc yuvbuf.\n");
                goto out;
            }
            ret = convert_yuv422p_resolution(buf,width,height,convertbuf,
                                             PREVIEW_WIDTH,PREVIEW_HEIGHT);
            if (ret < 0) {
                LOGE("Failed to transfor yuv422p resolution. \n");
                goto out;
            }
            yuvbuf = convertbuf;
            rgb_w = PREVIEW_WIDTH;
            rgb_h = PREVIEW_HEIGHT;
        }

        rgbbuf = (uint8_t *)malloc(rgb_w * rgb_h * 3);
        if (!rgbbuf) {
            LOGE("malloc rgbbuf failed!!\n");
            goto out;
        }

        ret = convert_yuv2rgb(yuvbuf, rgbbuf, rgb_w, rgb_h);
        if (ret < 0){
            LOGE("yuv 2 rgb fail, errno: %d\n", ret);
            free(rgbbuf);
            goto out;
        }

        if (capt_op.action == CAPTURE_PICTURE){
            char filename[128];
            sprintf(filename, "%s%s",capt_op.filename,".bmp");
            ret = convert_rgb2bmp(filename, rgb_w, rgb_h, 24, rgbbuf);
            if (ret < 0){
                LOGE("make bmp picutre fail, errno: %d\n",ret);
            }
        } else if (capt_op.action == PREVIEW_PICTURE) {
            MK_PIXEL_FMT(pixel_fmt);
            convert_rgb2pixel(rgbbuf, capt_op.ppbuf,rgb_w,rgb_h,
                             fbm->get_screen_width(),
                             fbm->get_screen_height(),
                             PREVIEW_START_PORT_X,PREVIEW_START_PORT_Y,
                             pixel_fmt,PREVIEW_ROTATE_DEGREE);
            fbm->display();
        }
    }


out:
    if (convertbuf != NULL)
        free(convertbuf);
    if (rgbbuf != NULL)
        free(rgbbuf);
    }
}



int main(int argc, char *argv[])
{
    struct capt_param_t capt_param;
    int ret;
    capt_param.width  = DEFAULT_WIDTH;
    capt_param.height = DEFAULT_HEIGHT;
    capt_param.bpp    = DEFAULT_BPP;
    capt_param.nbuf   = DEFAULT_NBUF;
    capt_param.io     = V4L2_METHOD_MMAP;
    capt_param.fr_cb  = (frame_receive)frame_receive_cb;

    capt_op.double_ch = DEFAULT_DOUBLE_CHANNEL;
    capt_op.channel   = DEFAULT_CHANNEL;
    capt_op.action    = DEFAULT_ACTION;

    while(1) {
        int oc;

        oc = getopt_long(argc, argv, \
                         short_options, long_options, \
                         NULL);
        if(-1 == oc)
            break;

        switch(oc) {
        case 0:
            break;

        case 'h':
            usage(stdout, argc, argv);
            return 0;

        case 'm':
            capt_param.io = V4L2_METHOD_MMAP;
            break;

        case 'r':
            capt_param.io = V4L2_METHOD_READ;
            break;

        case 'u':
            capt_param.io = V4L2_METHOD_USERPTR;
            break;

        case 'n':
            capt_param.nbuf = atoi(optarg);
            break;

        case 'x':
            capt_param.width = atoi(optarg);
            break;

        case 'y':
            capt_param.height = atoi(optarg);
            break;

        case 'c':
            capt_op.action = CAPTURE_PICTURE;
            capt_op.filename = (uint8_t*)optarg;
            break;

        case 'p':
            capt_op.action = PREVIEW_PICTURE;
            break;

        case 'd':
            capt_op.double_ch = 1;
            break;

        case 's':
            capt_op.channel = atoi(optarg);
            break;
        case 'f':
            CONFIG_UVC = atoi(optarg);
            break;
        default:
            usage(stderr, argc, argv);
            LOGE("Invalid parameter %c.\n",oc);
            return -1;
        }
    }
if(CONFIG_UVC == 1) {
    char *uvc_devname = "/dev/video1";
    int bulk_mode = 0;
    int dummy_data_gen_mode = 1;
    /* Frame format/resolution related params. */
    int nbufs = 3;			/* Ping-Pong buffers */
    /* USB speed related params */
    int mult = 0;
    int burst = 0;
    enum usb_device_speed speed = 2;	/* High-Speed */
    //enum io_method uvc_io_method = IO_METHOD_USERPTR;
    enum io_method uvc_io_method = IO_METHOD_MMAP;


    ret = uvc_open(&udev, uvc_devname,capt_param.width, capt_param.height);
    if (udev == NULL || ret < 0)
        return 1;
    udev->uvc_devname = uvc_devname;
    /* Set parameters as passed by user. */
    udev->width = DEFAULT_WIDTH;
    udev->height = DEFAULT_HEIGHT;
    udev->imgsize = (udev->width * udev->height * 2);
    udev->fcc = V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_YUV420;//
    udev->io = uvc_io_method;
    udev->bulk = bulk_mode;
    udev->nbufs = nbufs;
    udev->mult = mult;
    udev->burst = burst;
    udev->speed = speed;
    switch (speed) {
        case USB_SPEED_FULL:
        udev->maxpkt = 1023;
        break;
        default:
        case USB_SPEED_HIGH:
        udev->maxpkt = 1024;
    }


    if (dummy_data_gen_mode)
        /* UVC standalone setup. */
        udev->run_standalone = 1;
    uvc_video_set_format(udev);
}
#if 0
    fbm = get_fb_manager();

    if (fbm->init() < 0) {
        LOGE("Failed to init fb_manager\n");
        return -1;
    }

    capt_op.ppbuf = (uint16_t *)fbm->get_fbmem();
    if (capt_op.ppbuf == NULL) {
        LOGE("Failed to get fbmem\n");
        return -1;
    }
#endif
    cimm = get_camera_v4l2_manager();

    ret = cimm->init(&capt_param);
    if (ret < 0) {
        LOGE("Failed to init camera manager\n");
        return -1;
    }

    ret = cimm->start();
    if (ret < 0) {
        LOGE("Failed to start.\n");
        return -1;
    }

    if (capt_op.action == PREVIEW_PICTURE){
        while(getchar() != 'Q')
            LOGI("Enter 'Q' to exit\n");
    } else if (capt_op.action == CAPTURE_PICTURE) {
        sleep(1);
    }
    cimm->stop();
    cimm->deinit();
    if(CONFIG_UVC == 1) {
        if (udev->is_streaming) {
            /* ... and now UVC streaming.. */
            uvc_video_stream(udev, 0);
            uvc_uninit_device(udev);
            uvc_video_reqbufs(udev, 0);
            udev->is_streaming = 0;
        }
        uvc_close(udev);
    }
    return 0;
}
