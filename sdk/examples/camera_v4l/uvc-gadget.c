/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>
#include <pthread.h>

#include <uvc/uvc.h>

#define debug  0


#define UVC_DBG(msg...) \
    do { \
        if (debug) { \
            printf(msg); \
        } \
    } while(0)

#define UVC_ERR(msg...) \
    do { \
         printf(msg); \
    } while(0)

/* Enable debug prints. */
//#define ENABLE_BUFFER_DEBUG
#define ENABLE_TRACE 0

#define TRACE()\
    do {\
        if (ENABLE_TRACE) {\
            UVC_DBG("%s!\n", __func__);\
        }\
    }while(0)

#define ENABLE_USB_REQUEST_DEBUG

#define CLEAR(x)    memset (&(x), 0, sizeof (x))
#define max(a, b)   (((a) > (b)) ? (a) : (b))

#define clamp(val, min, max) ({                 \
        typeof(val) __val = (val);              \
        typeof(min) __min = (min);              \
        typeof(max) __max = (max);              \
        (void) (&__val == &__min);              \
        (void) (&__val == &__max);              \
        __val = __val < __min ? __min: __val;   \
        __val > __max ? __max: __val; })

#define ARRAY_SIZE(a)   ((sizeof(a) / sizeof(a[0])))
#define pixfmtstr(x)    (x) & 0xff, ((x) >> 8) & 0xff, ((x) >> 16) & 0xff, \
    ((x) >> 24) & 0xff

/*
 * The UVC webcam gadget kernel driver (g_webcam.ko) supports changing
 * the Brightness attribute of the Processing Unit (PU). by default. If
 * the underlying video capture device supports changing the Brightness
 * attribute of the image being acquired (like the Virtual Video, VIVI
 * driver), then we should route this UVC request to the respective
 * video capture device.
 *
 * Incase, there is no actual video capture device associated with the
 * UVC gadget and we wish to use this application as the final
 * destination of the UVC specific requests then we should return
 * pre-cooked (static) responses to GET_CUR(BRIGHTNESS) and
 * SET_CUR(BRIGHTNESS) commands to keep command verifier test tools like
 * UVC class specific test suite of USBCV, happy.
 *
 * Note that the values taken below are in sync with the VIVI driver and
 * must be changed for your specific video capture device. These values
 * also work well in case there in no actual video capture device.
 */
#define PU_BRIGHTNESS_MIN_VAL       0
#define PU_BRIGHTNESS_MAX_VAL       255
#define PU_BRIGHTNESS_STEP_SIZE     1
#define PU_BRIGHTNESS_DEFAULT_VAL   127

/* ---------------------------------------------------------------------------
 * Generic stuff
 */


/* Buffer representing one video frame */
struct buffer {
    struct v4l2_buffer buf;
    void *start;
    size_t length;
};

/* ---------------------------------------------------------------------------
 * UVC specific stuff
 */

struct uvc_frame_info {
    unsigned int width;
    unsigned int height;
    unsigned int intervals[8];
};

struct uvc_format_info {
    unsigned int fcc;
    const struct uvc_frame_info *frames;
};
#define rate_to_interval(rate) (10000000 / (rate))

static const struct uvc_frame_info uvc_frames_yuyv[] = {
    {  640, 360, { 666666, 10000000, 50000000, 0 }, },
    {  640, 480, {rate_to_interval(30), rate_to_interval(25), rate_to_interval(15),  0 }, },
    { 1280, 720, { 50000000, 0 }, },
    { 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_mjpeg[] = {
    {  640, 360, { 666666, 10000000, 50000000, 0 }, },
    { 1280, 720, { 50000000, 0 }, },
    { 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_h264[] = {
    {  640, 360, { 666666, 10000000, 50000000, 0 }, },
    { 1280, 720, { 50000000, 0 }, },
    { 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_grey[] = {
    {  640, 480, { 666666, 1000000, 5000000, 0 }, },
    { 1280, 720, { 50000000, 0 }, },
    { 0, 0, { 0, }, },
};

static const struct uvc_frame_info uvc_frames_yuv420[] = {
    {  640, 480, { 333333, 666666, 10000000, 0 }, },
    {1280, 720, { 50000000, 0 }, },
    { 0, 0, { 0, }, },
};

static const struct uvc_format_info uvc_formats[] = {
    { V4L2_PIX_FMT_YUYV, uvc_frames_yuyv },
    { V4L2_PIX_FMT_YUV420, uvc_frames_yuv420},
    {V4L2_PIX_FMT_GREY, uvc_frames_grey},
    //{ V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg },
    //{ V4L2_PIX_FMT_H264, uvc_frames_h264 },
};



/* forward declarations */
int uvc_video_stream(struct uvc_device *dev, int enable);

/* ---------------------------------------------------------------------------
 * UVC generic stuff
 */




/*
 * FIXME -- Need to see what this was intended for on the UVC gadget side.  It's
 * 			here, but there's nothing TIED to it right now on the gadget server
 * 			end of things.  Commenting it out to remove warnings.
 */

int uvc_video_set_format(struct uvc_device *dev)
{
    struct v4l2_format fmt;
    int ret;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (0 != ioctl(dev->uvc_fd, VIDIOC_G_FMT, &fmt)) {
         UVC_ERR("%x Failed to ioctl:VIDIOC_G_FMT %d(%s)\n", dev->uvc_fd, errno, strerror(errno));
    return -1;
    }
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = dev->width;
    fmt.fmt.pix.height = dev->height;
    fmt.fmt.pix.pixelformat = dev->fcc;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (dev->fcc == V4L2_PIX_FMT_MJPEG)
        fmt.fmt.pix.sizeimage = dev->imgsize * 1.5;

    ret = ioctl(dev->uvc_fd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        UVC_ERR("UVC: Unable to set format %s (%d).\n",
                strerror(errno), errno);
        return ret;
    }

    UVC_DBG("UVC: Setting format to: %c%c%c%c %ux%u\n",
            pixfmtstr(dev->fcc), dev->width, dev->height);

    return 0;
}

int uvc_video_stream(struct uvc_device *dev, int enable)
{
    int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    int ret;

    if (!enable) {
        ret = ioctl(dev->uvc_fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0) {
            UVC_ERR("UVC: VIDIOC_STREAMOFF failed: %s (%d).\n",
                    strerror(errno), errno);
            return ret;
        }

        UVC_DBG("UVC: Stopping video stream.\n");

        return 0;
    }

    ret = ioctl(dev->uvc_fd, VIDIOC_STREAMON, &type);
    if (ret < 0) {
        UVC_ERR("UVC: Unable to start streaming %s (%d).\n",
                strerror(errno), errno);
        return ret;
    }

    //UVC_DBG("UVC: Starting video stream.\n");

    dev->uvc_shutdown_requested = 0;

    return 0;
}

int uvc_uninit_device(struct uvc_device *dev)
{
    unsigned int i;
    int ret;

    switch (dev->io) {
    case IO_METHOD_MMAP:
        if (!dev->mem)
            break;
        for (i = 0; i < dev->nbufs; ++i) {
            if (NULL == dev->mem[i].start)
            continue;
            ret = munmap(dev->mem[i].start, dev->mem[i].length);
            if (ret < 0) {
                UVC_DBG("UVC: munmap failed\n");
                return ret;
            }
        }
        free(dev->mem);
        dev->mem = NULL;
        break;

    case IO_METHOD_USERPTR:
    default:
        if (!dev->mem)
            break;

        for (i = 0; i < dev->nbufs; ++i) {
            if (NULL == dev->mem[i].start)
                continue;
            free(dev->mem[i].start);
            dev->mem[i].start = NULL;
        }
        free(dev->mem);
        dev->mem = NULL;
        break;
    }

    return 0;
}
void uvc_fill_streaming_control(struct uvc_device *dev,
        struct uvc_streaming_control *ctrl,
        int iframe, int iformat);

int uvc_open(struct uvc_device **uvc_device, char *devname, int width, int height)
{
    struct uvc_device *dev;
    struct v4l2_capability cap;
    struct v4l2_event_subscription sub;
    unsigned int payload_size = 0;
    struct uvc_format_info const *format;
    struct uvc_frame_info const *frame;
    int fd;
    int ret = -EINVAL;
    int i, j;
    memset(&sub, 0, sizeof sub);
    do {
    fd = open(devname, O_RDWR | O_NONBLOCK);
    if (fd < 0)
        sleep(10);
    } while ( fd < 0 );
    ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0) {
        UVC_ERR("UVC: unable to query uvc device: %s (%d) fd %d\n",
                strerror(errno), errno, fd);
        goto err;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {
        UVC_ERR("UVC: %s is no video output device\n", devname);
        goto err;
    }

    sub.type = UVC_EVENT_SETUP;
    ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DATA;
    ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMON;
    ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_STREAMOFF;
    ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_CONNECT;
    ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
    sub.type = UVC_EVENT_DISCONNECT;
    ioctl(fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

    dev = calloc(1, sizeof *dev);
    if (dev == NULL) {
        ret = -ENOMEM;
        goto err;
    }

    UVC_DBG("uvc device is %s on bus %s\n", cap.card, cap.bus_info);
    UVC_DBG("uvc open succeeded, file descriptor = %d\n", fd);

    dev->uvc_fd = fd;
    *uvc_device = dev;

    switch (dev->fcc) {
    case V4L2_PIX_FMT_GREY:
        payload_size = dev->width * dev->height;
        break;
    case V4L2_PIX_FMT_YUV420:
        payload_size = dev->width * dev->height * 3 / 2;
        break;
    case V4L2_PIX_FMT_YUYV:
        payload_size = dev->width * dev->height * 2;
        break;
    }
    for(i=0; i<ARRAY_SIZE(uvc_formats); i++){
        format = &uvc_formats[i];
        if(format->fcc == V4L2_PIX_FMT_YUYV)
        break;
    }
    for(j=0; j<ARRAY_SIZE(uvc_frames_yuyv); j++){
    frame = &uvc_frames_yuyv[j];
    if(frame->width == width && frame->height == height)
        break;
    }
    uvc_fill_streaming_control(dev, &dev->probe, j, i);
    uvc_fill_streaming_control(dev, &dev->commit, j, i);

    if (dev->bulk)
        /* FIXME Crude hack, must be negotiated with the driver. */
        dev->probe.dwMaxPayloadTransferSize =
            dev->commit.dwMaxPayloadTransferSize = payload_size;
    dev->is_streaming = 0;

    return 0;

err:
    close(fd);
    return ret;
}

void uvc_close(struct uvc_device *dev)
{
    close(dev->uvc_fd);
    free(dev->imgdata);
    dev->imgdata = NULL;
    free(dev);
    dev = NULL;
}

/* ---------------------------------------------------------------------------
 * UVC streaming related
 */

void uvc_video_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
    unsigned int bpl;
    unsigned int i;


    switch (dev->fcc) {
        case V4L2_PIX_FMT_GREY:
            /* Fill the buffer with video data. */
            bpl = dev->width;
            for (i = 0; i < dev->height; ++i) {
               memset(dev->mem[buf->index].start + i*bpl,
                dev->color++, bpl);
            }

            buf->bytesused = bpl * dev->height;
            break;
        case V4L2_PIX_FMT_YUV420:
            if (dev->imgdata) {
            memcpy(dev->mem[buf->index].start, dev->imgdata, dev->mem[buf->index].length);
            }
            buf->bytesused = dev->width * dev->height * 3 / 2;
            if (buf->bytesused !=  dev->mem[buf->index].length)
            UVC_ERR("%s buf->bytesused %d != dev->mem[buf->index].length %d\n", __func__,
            buf->bytesused,  dev->mem[buf->index].length);
            break;
        case V4L2_PIX_FMT_YUYV:
            if (dev->imgdata) {
            memcpy(dev->mem[buf->index].start, dev->imgdata, dev->mem[buf->index].length);
            }
            buf->bytesused = dev->width * dev->height * 2;
            if (buf->bytesused !=  dev->mem[buf->index].length)
            UVC_ERR("%s buf->bytesused %d != dev->mem[buf->index].length %d\n", __func__,
            buf->bytesused,  dev->mem[buf->index].length);
            break;
    }
    //UVC_DBG("%s:%d!\n", __func__, __LINE__);
}

    int
uvc_video_process(struct uvc_device *dev)
{
    struct v4l2_buffer ubuf;
    int ret;
    UVC_DBG("Enter %s:%d,  dev->is_streaming = %d,dev->run_standalone = %d\n", __func__, __LINE__, dev->is_streaming, dev->run_standalone);
    /*
     * Return immediately if UVC video output device has not started
     * streaming yet.
     */
    //UVC_DBG("2\n");
    if (!dev->is_streaming)
        return 0;
    /* Prepare a v4l2 buffer to be dequeued from UVC domain. */
    CLEAR(ubuf);

    ubuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    switch (dev->io) {
        case IO_METHOD_MMAP:
            ubuf.memory = V4L2_MEMORY_MMAP;
            break;

        case IO_METHOD_USERPTR:
        default:
            ubuf.memory = V4L2_MEMORY_USERPTR;
            break;
    }

    if (dev->run_standalone) {
        /* UVC stanalone setup. */
    ret = ioctl(dev->uvc_fd, VIDIOC_DQBUF, &ubuf);
    if (ret < 0)
        return ret;
        dev->dqbuf_count++;
        uvc_video_fill_buffer(dev, &ubuf);

        ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &ubuf);
        if (ret < 0) {
            UVC_DBG("UVC: Unable to queue buffer: %s (%d).\n",
                    strerror(errno), errno);
            return ret;
        }
        UVC_DBG("d%d!\n", ubuf.index);

        dev->qbuf_count++;
    }
    //UVC_DBG("Leave %s:%d\n", __func__, __LINE__);
    return 0;
}
int uvc_video_qbuf_mmap(struct uvc_device *dev)
{
    unsigned int i;
    int ret;

    for (i = 0; i < dev->nbufs; ++i) {
    memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

    dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
    dev->mem[i].buf.index = i;

    /* UVC standalone setup. */
    if (dev->run_standalone)
        uvc_video_fill_buffer(dev, &(dev->mem[i].buf));

    //UVC_DBG("%s:%d!\n", __func__, __LINE__);
    ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
    if (ret < 0) {
        UVC_ERR("UVC: VIDIOC_QBUF failed : %s (%d).\n",
            strerror(errno), errno);
        return ret;
    }

    dev->qbuf_count++;
    }

    return 0;
}
int uvc_video_qbuf_userptr(struct uvc_device *dev)
{
    unsigned int i;
    int ret;

    /* UVC standalone setup. */
    for (i = 0; i < dev->nbufs; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_USERPTR;
        dev->mem[i].buf.index = i;
        dev->mem[i].buf.m.userptr = (unsigned long)dev->mem[i].start;
        dev->mem[i].buf.length = dev->mem[i].length;

        if (dev->run_standalone)
            uvc_video_fill_buffer(dev, &(dev->mem[i].buf));

        ret = ioctl(dev->uvc_fd, VIDIOC_QBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            UVC_ERR("UVC: %d VIDIOC_QBUF failed : %s (%d).\n", __LINE__,
                strerror(errno), errno);
            return ret;
        }
        dev->qbuf_count++;
    }

    return 0;
}

int uvc_video_qbuf(struct uvc_device *dev)
{
    int ret = 0;

    //UVC_DBG("%s:%d!\n", __func__, __LINE__);
    switch (dev->io) {
        case IO_METHOD_MMAP:
            ret = uvc_video_qbuf_mmap(dev);
            break;

        case IO_METHOD_USERPTR:
            ret = uvc_video_qbuf_userptr(dev);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

int uvc_video_reqbufs_mmap(struct uvc_device *dev, int nbufs)
{
    struct v4l2_requestbuffers rb;
    unsigned int i;
    int ret;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_MMAP;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
        if (ret == -EINVAL)
            UVC_ERR("UVC: does not support memory mapping\n");
        else
            UVC_ERR("UVC: Unable to allocate buffers: %s (%d).\n",
                    strerror(errno), errno);
        goto err;
    }

    if (!rb.count)
        return 0;

    if (rb.count < 2) {
        UVC_DBG("UVC: Insufficient buffer memory.\n");
        ret = -EINVAL;
        goto err;
    }

    /* Map the buffers. */
    dev->mem = calloc(rb.count, sizeof dev->mem[0]);
    if (!dev->mem) {
        UVC_DBG("UVC: Out of memory\n");
        ret = -ENOMEM;
        goto err;
    }
    for (i = 0; i < rb.count; ++i) {
        memset(&dev->mem[i].buf, 0, sizeof(dev->mem[i].buf));

        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_MMAP;
        dev->mem[i].buf.index = i;
        dev->mem[i].buf.length = dev->imgsize;

        ret = ioctl(dev->uvc_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            printf("UVC: VIDIOC_QUERYBUF failed for buf %d: "
                    "%s (%d).\n", i, strerror(errno), errno);
            ret = -EINVAL;
            goto err_free;
        }
        dev->mem[i].start = mmap(NULL /* start anywhere */,
                dev->mem[i].buf.length,
                PROT_READ | PROT_WRITE /* required */,
                MAP_SHARED /* recommended */,
                dev->uvc_fd, dev->mem[i].buf.m.offset);
        if (MAP_FAILED == dev->mem[i].start) {
            printf("UVC: Unable to map buffer %u: %s (%d).\n", i,
                    strerror(errno), errno);
            dev->mem[i].length = 0;
            ret = -EINVAL;
            goto err_free;
        }
        dev->mem[i].length = dev->mem[i].buf.length;
    }

    dev->nbufs = rb.count;
    //UVC_DBG("UVC: %u buffers allocated.\n", rb.count);

    return 0;

err_free:
    free(dev->mem);
    dev->mem = NULL;
err:
    return ret;
}

int uvc_video_reqbufs_userptr(struct uvc_device *dev, int nbufs)
{
    unsigned int i, payload_size = 0;
    int ret;
    struct v4l2_requestbuffers rb;

    CLEAR(rb);

    rb.count = nbufs;
    rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    rb.memory = V4L2_MEMORY_USERPTR;

    ret = ioctl(dev->uvc_fd, VIDIOC_REQBUFS, &rb);
    if (ret < 0) {
        if (ret == -EINVAL)
            UVC_ERR("UVC: does not support user pointer i/o\n");
        else
            UVC_ERR("UVC: VIDIOC_REQBUFS error %s (%d).\n",
                    strerror(errno), errno);
        goto err;
    }

    if (!rb.count)
           return 0;

    dev->nbufs = rb.count;

    /* Allocate buffers to hold dummy data pattern. */
    dev->mem = calloc(rb.count, sizeof (dev->mem[0]) + 1024);
    if (!dev->mem) {
    UVC_DBG("UVC: Out of memory\n");
    ret = -ENOMEM;
    goto err;
    }

    switch (dev->fcc) {
    case V4L2_PIX_FMT_GREY:
        payload_size = dev->width * dev->height;
    break;
    case V4L2_PIX_FMT_YUV420:
        payload_size = dev->width * dev->height * 3 / 2;
        break;
    case V4L2_PIX_FMT_YUYV:
        payload_size = dev->width * dev->height * 2;
        break;
    }

    for (i = 0; i < rb.count; ++i) {
        dev->mem[i].length = payload_size;
        dev->mem[i].start = malloc(payload_size);
        dev->mem[i].buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        dev->mem[i].buf.memory = V4L2_MEMORY_USERPTR;
        dev->mem[i].buf.index = i;
        dev->mem[i].buf.m.userptr = (unsigned long)dev->mem[i].start;
        dev->mem[i].buf.length = payload_size;
        ret = ioctl(dev->uvc_fd, VIDIOC_QUERYBUF, &(dev->mem[i].buf));
        if (ret < 0) {
            UVC_ERR("UVC: VIDIOC_QUERYBUF failed for buf %d: "
            "%s (%d).\n", i, strerror(errno), errno);
            ret = -EINVAL;
            goto err;
        }
    }

    return 0;

err:
    return ret;

}

    int
uvc_video_reqbufs(struct uvc_device *dev, int nbufs)
{
    int ret = 0;

    switch (dev->io) {
        case IO_METHOD_MMAP:
            ret = uvc_video_reqbufs_mmap(dev, nbufs);
            break;

        case IO_METHOD_USERPTR:
            ret = uvc_video_reqbufs_userptr(dev, nbufs);
            break;

        default:
            ret = -EINVAL;
            break;
    }

    return ret;
}

/*
 * This function is called in response to either:
 * 	- A SET_ALT(interface 1, alt setting 1) command from USB host,
 * 	  if the UVC gadget supports an ISOCHRONOUS video streaming endpoint
 * 	  or,
 *
 *	- A UVC_VS_COMMIT_CONTROL command from USB host, if the UVC gadget
 *	  supports a BULK type video streaming endpoint.
 */
int uvc_handle_streamon_event(struct uvc_device *dev)
{
    TRACE();
    int ret;

    ret = uvc_video_reqbufs(dev, dev->nbufs);
    if (ret < 0)
        goto err;

    ret = uvc_video_qbuf(dev);
    if (ret < 0)
       goto err;

    UVC_DBG("start streaming\n");
    uvc_video_stream(dev, 1);
    UVC_DBG("started streaming\n");

    if(dev->run_standalone) {
        dev->first_buffer_queued = 1;
        dev->is_streaming = 1;
    }

    return 0;

err:
    return ret;
}

void dump_stream_control(struct uvc_streaming_control *ctrl)
{
    UVC_DBG("mHit : 0x%04x\n", ctrl->bmHint);
    UVC_DBG("FormatIndex: 0x%02x\n", ctrl->bFormatIndex);
    UVC_DBG("FrameIndex: 0x%02x\n", ctrl->bFrameIndex);
    UVC_DBG("FrameInterval: 0x%08x\n", ctrl->dwFrameInterval);
    UVC_DBG("KeyFrameRate: 0x%04x\n", ctrl->wKeyFrameRate);
    UVC_DBG("PFrameRate: 0x%04x\n", ctrl->wPFrameRate);
    UVC_DBG("compQuality: 0x%04x\n", ctrl->wCompQuality);
    UVC_DBG("CompWindowSize: 0x%04x\n", ctrl->wCompWindowSize);
    UVC_DBG("Delay: 0x%04x\n", ctrl->wDelay);
    UVC_DBG("MaxVideoFrameSize: 0x%08x\n", ctrl->dwMaxVideoFrameSize);
    UVC_DBG("MaxPayloadTransferSize: 0x%08x\n", ctrl->dwMaxPayloadTransferSize);
    UVC_DBG("ClockFrequency: 0x%08x\n", ctrl->dwClockFrequency);
    UVC_DBG("FrameInfo: 0x%02x\n", ctrl->bmFramingInfo);
    UVC_DBG("PreferVersion: 0x%02x\n", ctrl->bPreferedVersion);
    UVC_DBG("MinVersion: 0x%02x\n", ctrl->bMinVersion);
    UVC_DBG("MaxVersion: 0x%02x\n", ctrl->bMaxVersion);
}
/* ---------------------------------------------------mHit ------------------------
 * UVC Request processing
 */

void uvc_fill_streaming_control(struct uvc_device *dev,
        struct uvc_streaming_control *ctrl,
        int iframe, int iformat)
{
    const struct uvc_format_info *format;
    const struct uvc_frame_info *frame;
    unsigned int nframes;
    UVC_DBG("format:%d, frame:%d!\n", iformat, iframe);

    if (iformat < 0)
        iformat = ARRAY_SIZE(uvc_formats) + iformat;
    if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats))
        return;
    format = &uvc_formats[iformat];

    nframes = 0;
    while (format->frames[nframes].width != 0)
        ++nframes;

    if (iframe < 0)
        iframe = nframes + iframe;
    if (iframe < 0 || iframe >= (int)nframes)
        return;
    frame = &format->frames[iframe];

    memset(ctrl, 0, sizeof *ctrl);
    ctrl->bmHint = 1;
    ctrl->bFormatIndex = iformat + 1;
    ctrl->bFrameIndex = iframe + 1;
    ctrl->dwFrameInterval = frame->intervals[0];
    switch (format->fcc) {
        case V4L2_PIX_FMT_GREY:
            ctrl->dwMaxVideoFrameSize = frame->width * frame->height;
            break;
        case V4L2_PIX_FMT_YUV420:
            ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 3 / 2;
            break;
        case V4L2_PIX_FMT_YUYV:
            ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 2;
            break;
    }

    /* TODO: the UVC maxpayload transfer size should be filled
     * by the driver.
     */
    if (!dev->bulk)
        ctrl->dwMaxPayloadTransferSize = (dev->maxpkt) * (dev->mult + 1) * (dev->burst + 1);
    else
        ctrl->dwMaxPayloadTransferSize = ctrl->dwMaxVideoFrameSize;

    ctrl->bmFramingInfo = 3;
    ctrl->bPreferedVersion = 1;
    ctrl->bMaxVersion = 1;
    dump_stream_control(ctrl);
}

void uvc_events_process_standard(struct uvc_device *dev,
        struct usb_ctrlrequest *ctrl,
        struct uvc_request_data *resp)
{
    TRACE();
    UVC_DBG("standard request\n");
    (void)dev;
    (void)ctrl;
    (void)resp;
    }

void uvc_events_process_control(struct uvc_device *dev, uint8_t req,
        uint8_t cs, uint8_t entity_id,
        uint8_t len, struct uvc_request_data *resp)
{
    TRACE();
    UVC_DBG("id:%d, req:%d, cs:%d, len:%d!\n", entity_id, req, cs, len);
    switch (entity_id) {
        case 0:
            switch (cs) {
                case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
                    /* Send the request error code last prepared. */
                    resp->data[0] = dev->request_error_code.data[0];
                    resp->length = dev->request_error_code.length;
                    break;

                default:
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare an error code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }
            break;

            /* Camera terminal unit 'UVC_VC_INPUT_TERMINAL'. */
        case 1:
            switch (cs) {
                /*
                 * We support only 'UVC_CT_AE_MODE_CONTROL' for CAMERA
                 * terminal, as our bmControls[0] = 2 for CT. Also we
                 * support only auto exposure.
                 */
                case UVC_CT_AE_MODE_CONTROL:
                    switch (req) {
                        case UVC_SET_CUR:
                            /* Incase of auto exposure, attempts to
                             * programmatically set the auto-adjusted
                             * controls are ignored.
                             */
                            resp->data[0] = 0x01;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;

                        case UVC_GET_INFO:
                            /*
                             * TODO: We support Set and Get requests, but
                             * don't support async updates on an video
                             * status (interrupt) endpoint as of
                             * now.
                             */
                            resp->data[0] = 0x03;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;

                        case UVC_GET_CUR:
                        case UVC_GET_DEF:
                        case UVC_GET_RES:
                            /* Auto Mode â?? auto Exposure Time, auto Iris. */
                            resp->data[0] = 0x02;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        default:
                            /*
                             * We don't support this control, so STALL the
                             * control ep.
                             */
            printf("=====> %s %d\n", __func__, __LINE__);

                            resp->length = -EL2HLT;
                            /*
                             * For every unsupported control request
                             * set the request error code to appropriate
                             * value.
                             */
                            dev->request_error_code.data[0] = 0x07;
                            dev->request_error_code.length = 1;
                            break;
                    }
                    break;

                default:
                    /*
                     * We don't support this control, so STALL the control
                     * ep.
                     */
            printf("=====> %s %d\n", __func__, __LINE__);

                    resp->length = -EL2HLT;
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare a Request Error Code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }
            break;

            /* processing unit 'UVC_VC_PROCESSING_UNIT' */
        case 2:
            switch (cs) {
                /*
                 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
                 * Unit, as our bmControls[0] = 1 for PU.
                 */
                case UVC_PU_BRIGHTNESS_CONTROL:
                    switch (req) {
                        case UVC_SET_CUR:
                            resp->data[0] = 0x0;
                            resp->length = len;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_MIN:
                            resp->data[0] = PU_BRIGHTNESS_MIN_VAL;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_MAX:
                            resp->data[0] = PU_BRIGHTNESS_MAX_VAL;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_CUR:
                            resp->length = 2;
                            memcpy(&resp->data[0], &dev->brightness_val,
                                    resp->length);
                            /*
                             * For every successfully handled control
                             * request set the request error code to no
                             * error
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_INFO:
                            /*
                             * We support Set and Get requests and don't
                             * support async updates on an interrupt endpt
                             */
                            resp->data[0] = 0x03;
                            resp->length = 1;
                            /*
                             * For every successfully handled control
                             * request, set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_DEF:
                            resp->data[0] = PU_BRIGHTNESS_DEFAULT_VAL;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request, set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        case UVC_GET_RES:
                            resp->data[0] = PU_BRIGHTNESS_STEP_SIZE;
                            resp->length = 2;
                            /*
                             * For every successfully handled control
                             * request, set the request error code to no
                             * error.
                             */
                            dev->request_error_code.data[0] = 0x00;
                            dev->request_error_code.length = 1;
                            break;
                        default:
                            /*
                             * We don't support this control, so STALL the
                             * default control ep.
                             */
                            resp->length = -EL2HLT;
                            /*
                             * For every unsupported control request
                             * set the request error code to appropriate
                             * code.
                             */
                            dev->request_error_code.data[0] = 0x07;
                            dev->request_error_code.length = 1;
                            break;
                    }
                    break;

                default:
                    /*
                     * We don't support this control, so STALL the control
                     * ep.
                     */
                    UVC_DBG("Don't support processing cs %d!\n", cs);
                    resp->length = -EL2HLT;
                    /*
                     * If we were not supposed to handle this
                     * 'cs', prepare a Request Error Code response.
                     */
                    dev->request_error_code.data[0] = 0x06;
                    dev->request_error_code.length = 1;
                    break;
            }

            break;

        default:
            /*
             * If we were not supposed to handle this
             * 'cs', prepare a Request Error Code response.
             */
            UVC_DBG("Don't support control cs %d!\n", cs);
            dev->request_error_code.data[0] = 0x06;
            dev->request_error_code.length = 1;
            break;

    }

    //UVC_DBG("control request (req %02x cs %02x)\n", req, cs);
}

void uvc_events_process_streaming(struct uvc_device *dev, uint8_t req, uint8_t cs,
    struct uvc_request_data *resp)
{
    TRACE();
    struct uvc_streaming_control *ctrl;

    //UVC_DBG("streaming request (req %02x cs %02x)\n", req, cs);
    UVC_DBG("req:%d, cs:%d\n",req, cs);
    UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
    if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
        return;

    ctrl = (struct uvc_streaming_control *)&resp->data;
    resp->length = sizeof *ctrl;
    UVC_DBG("Enter %s:%d\n", __func__, __LINE__);

    switch (req) {
    case UVC_SET_CUR:
    UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
    dev->control = cs;
    case UVC_GET_CUR:
    UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
    if (cs == UVC_VS_PROBE_CONTROL)
        memcpy(ctrl, &dev->probe, sizeof *ctrl);
    else
        memcpy(ctrl, &dev->commit, sizeof *ctrl);
    resp->length = 34;
    break;

    case UVC_GET_MIN:
        UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
    case UVC_GET_MAX:
        UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
    case UVC_GET_DEF:
        UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
        uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0,
            req == UVC_GET_MAX ? -1 : 0);
        resp->length = 26;
    break;

    case UVC_GET_RES:
        UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
        CLEAR(ctrl);
    break;

    case UVC_GET_LEN:
        UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
        resp->data[0] = 0x00;
        resp->data[1] = 0x22;
        resp->length = 2;
    break;

    case UVC_GET_INFO:
        UVC_DBG("Enter %s:%d\n", __func__, __LINE__);
        resp->data[0] = 0x03;
        resp->length = 1;
    break;
    }
}

void uvc_events_process_class(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
        struct uvc_request_data *resp)
{
    TRACE();
    if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
        return;

    switch (ctrl->wIndex & 0xff) {
        case UVC_INTF_CONTROL:
            uvc_events_process_control(dev, ctrl->bRequest,
                    ctrl->wValue >> 8,
                    ctrl->wIndex >> 8,
                    ctrl->wLength, resp);
            break;

        case UVC_INTF_STREAMING:
            uvc_events_process_streaming(dev, ctrl->bRequest,
                    ctrl->wValue >> 8, resp);
            break;

        default:
            break;
    }
}

void uvc_events_process_setup(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
        struct uvc_request_data *resp)
{
    TRACE();
    dev->control = 0;

#ifdef ENABLE_USB_REQUEST_DEBUG
    UVC_DBG("\nbRequestType %02x bRequest %02x wValue %04x wIndex %04x "
            "wLength %04x\n", ctrl->bRequestType, ctrl->bRequest,
            ctrl->wValue, ctrl->wIndex, ctrl->wLength);
#endif

    switch (ctrl->bRequestType & USB_TYPE_MASK) {
    case USB_TYPE_STANDARD:
        uvc_events_process_standard(dev, ctrl, resp);
        break;

    case USB_TYPE_CLASS:
        uvc_events_process_class(dev, ctrl, resp);
        break;

    default:
        break;
    }
}

int uvc_events_process_control_data(struct uvc_device *dev,
        uint8_t cs, uint8_t entity_id,
        struct uvc_request_data *data)
{
    TRACE();
    switch (entity_id) {
        /* Processing unit 'UVC_VC_PROCESSING_UNIT'. */
        case 2:
            switch (cs) {
                /*
                 * We support only 'UVC_PU_BRIGHTNESS_CONTROL' for Processing
                 * Unit, as our bmControls[0] = 1 for PU.
                 */
                case UVC_PU_BRIGHTNESS_CONTROL:
                    memcpy(&dev->brightness_val, data->data, data->length);
                    /* UVC - V4L2 integrated path. */
                    if (!dev->run_standalone)
                        /*
                         * Try to change the Brightness attribute on
                         * Video capture device. Note that this try may
                         * succeed or end up with some error on the
                         * video capture side. By default to keep tools
                         * like USBCV's UVC test suite happy, we are
                         * maintaining a local copy of the current
                         * brightness value in 'dev->brightness_val'
                         * variable and we return the same value to the
                         * Host on receiving a GET_CUR(BRIGHTNESS)
                         * control request.
                         *
                         * FIXME: Keeping in view the point discussed
                         * above, notice that we ignore the return value
                         * from the function call below. To be strictly
                         * compliant, we should return the same value
                         * accordingly.
                         */
                        //v4l2_set_ctrl(dev->vdev, dev->brightness_val,
                        //        V4L2_CID_BRIGHTNESS);
                    break;

                default:
                    break;
            }

            break;

        default:
            break;
    }

    UVC_DBG("Control Request data phase (cs %02x entity %02x)\n",
            cs, entity_id);

    return 0;
}

int uvc_events_process_data(struct uvc_device *dev, struct uvc_request_data *data)
{
    TRACE();
    struct uvc_streaming_control *target;
    struct uvc_streaming_control *ctrl;
    //struct v4l2_format fmt;				// FIXME -- Unused right now.
    const struct uvc_format_info *format;
    const struct uvc_frame_info *frame;
    const unsigned int *interval;
    unsigned int iformat, iframe;
    unsigned int nframes;
    unsigned int *val = (unsigned int *)data->data;
    int ret;
    switch (dev->control) {
    case UVC_VS_PROBE_CONTROL:
            //UVC_DBG("setting probe control, length = %d\n", data->length);
           target = &dev->probe;
            break;

    case UVC_VS_COMMIT_CONTROL:
        //UVC_DBG("setting commit control, length = %d\n", data->length);
        target = &dev->commit;
        break;

    default:
        //UVC_DBG("setting unknown control, length = %d\n", data->length);

        /*
         * As we support only BRIGHTNESS control, this request is
         * for setting BRIGHTNESS control.
         * Check for any invalid SET_CUR(BRIGHTNESS) requests
         * from Host. Note that we support Brightness levels
         * from 0x0 to 0x10 in a step of 0x1. So, any request
         * with value greater than 0x10 is invalid.
         */
        if (*val > PU_BRIGHTNESS_MAX_VAL) {
            UVC_DBG("%s:%d!\n", __func__, __LINE__);
            return -EINVAL;
        } else {
            ret = uvc_events_process_control_data(dev,
            UVC_PU_BRIGHTNESS_CONTROL,
            2, data);
            if (ret < 0)
            goto err;

            return 0;
        }
    }

    ctrl = (struct uvc_streaming_control *)&data->data;
    dump_stream_control(ctrl);
    iformat = clamp((unsigned int)ctrl->bFormatIndex, 1U,
            (unsigned int)ARRAY_SIZE(uvc_formats));
    format = &uvc_formats[iformat-1];
    nframes = 0;
    while (format->frames[nframes].width != 0)
        ++nframes;

    iframe = clamp((unsigned int)ctrl->bFrameIndex, 1U, nframes);
    frame = &format->frames[iframe-1];
    interval = frame->intervals;

    while (interval[0] < ctrl->dwFrameInterval && interval[1])
        ++interval;

    target->bFormatIndex = iformat;
    target->bFrameIndex = iframe;
    switch (format->fcc) {
        case V4L2_PIX_FMT_GREY:
            target->dwMaxVideoFrameSize = frame->width * frame->height;
            break;
        case V4L2_PIX_FMT_YUV420:
            target->dwMaxVideoFrameSize = frame->width * frame->height * 3 / 2;
        case V4L2_PIX_FMT_YUYV:
            target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
    }
    target->dwFrameInterval = *interval;

    if (dev->control == UVC_VS_COMMIT_CONTROL) {
        dev->fcc = format->fcc;
        dev->width = frame->width;
        dev->height = frame->height;
        UVC_ERR("commit ctl %08x, %dX%d!\n", dev->fcc, dev->width, dev->height);

        if (dev->bulk) {
            ret = uvc_handle_streamon_event(dev);
            if (ret < 0)
                goto err;
        }
    }
    dump_stream_control(target);
    return 0;

err:
    return ret;
}

int uvc_events_process(struct uvc_device *dev)
{
    struct v4l2_event v4l2_event;
    struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
    struct uvc_request_data resp;
    int ret;
    TRACE();
    UVC_DBG("Enter %s:%d, dev->is_streaming = %d, v4l2_event.type = 0x%x\n", __func__, __LINE__, dev->is_streaming , v4l2_event.type);

    ret = ioctl(dev->uvc_fd, VIDIOC_DQEVENT, &v4l2_event);
    if (ret < 0) {
        UVC_ERR("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno),
                errno);
        return 0;
    }

    memset(&resp, 0, sizeof resp);
    resp.length = -EL2HLT;

    switch (v4l2_event.type) {
        case UVC_EVENT_CONNECT:
            printf("UVC: connected!\n");
            return 0;

        case UVC_EVENT_DISCONNECT:
            dev->uvc_shutdown_requested = 1;
            printf("UVC: Possible USB shutdown requested from "
                    "Host, seen via UVC_EVENT_DISCONNECT\n");
    case UVC_EVENT_STREAMOFF:
        if (dev->is_streaming) {
            uvc_video_stream(dev, 0);
            uvc_uninit_device(dev);
            uvc_video_reqbufs(dev, 0);
            dev->is_streaming = 0;
            dev->first_buffer_queued = 0;
        }
        dev->uvc_shutdown_requested = 0;
        return 0;

    case UVC_EVENT_SETUP:
        uvc_events_process_setup(dev, &uvc_event->req, &resp);
        break;

    case UVC_EVENT_DATA:
        uvc_events_process_data(dev, &uvc_event->data);
        return 0;

    case UVC_EVENT_STREAMON:
        if (!dev->bulk)
        uvc_handle_streamon_event(dev);
        break;
    }

    ret = ioctl(dev->uvc_fd, UVCIOC_SEND_RESPONSE, &resp);
    if (ret < 0) {
        UVC_ERR("UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno),
                errno);
        return 0;
    }
    return 0;
    //UVC_DBG("Leave %s:%d\n", __func__, __LINE__);
}

void uvc_events_init(struct uvc_device *dev)
{
   UVC_DBG("Leave %s:%d\n", __func__, __LINE__);
}

/* ---------------------------------------------------------------------------
 * main
 */
void image_load(struct uvc_device *dev,  uint8_t *img, unsigned int width, unsigned int height)
{
    if (img == NULL)
        return;

    static int i =0;
    dev->imgsize = width * height * 2;

    dev->imgwidth = width;
    dev->imgdata=img;
    if(i%1000==0){
    i++;
    printf("image_load%x %x %x %x\n", *(unsigned int *)dev->imgdata,
        *((unsigned int *)dev->imgdata + 1),
        *((unsigned int *)dev->imgdata + 2),
        *((unsigned int *)dev->imgdata + 3));
    }
}
