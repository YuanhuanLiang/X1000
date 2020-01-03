/*
 * doouble Camera types
 *
 * Copyright (C) 2017, Ingenic Semiconductor Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _DOUBLE_CAMERA_H_
#define _DOUBLE_CAMERA_H_


struct double_camera_op
{
    struct v4l2_subdev subdev;
    unsigned char cur_channel;
    void (*switch_work_channel) (struct double_camera_op* dc);
};



#endif