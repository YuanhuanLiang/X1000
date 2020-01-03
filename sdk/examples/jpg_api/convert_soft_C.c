#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <malloc.h>
#include <jpa_api/framebuffer.h>
#include <jpa_api/pixel.h>
int YUYV_to_RGB565 (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t dstStride)
{
    __u8 y0 = 0;
    __u8 u = 0;
    __u8 y1 = 0;
    __u8 v = 0;
    int colorB = 0;
    int colorG = 0;
    int colorR = 0;
    int line =0;
    int col =0;
    int fb_col =0;
    int dst_width = dstStride / 32 ;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src_buf;
    struct bpp16_pixel *pixel = (struct bpp16_pixel *)dst_buf;

    for(line = 0; line < src_height; line++)
    {
       col = 0 ;
       for( fb_col = 0; fb_col < dst_width; fb_col = fb_col + 2)
       {
          y0 = yuv422_samp[line * src_width/2 + col].b1;
          u  = yuv422_samp[line * src_width/2 + col].b2;
          y1 = yuv422_samp[line * src_width/2 + col].b3;
          v  = yuv422_samp[line * src_width/2 + col].b4;

          colorB = y0 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y0 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y0 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

          colorB = colorB >> 3;
          colorG = colorG >> 2;
          colorR = colorR >> 3;

          pixel[line * dst_width + fb_col].red = colorR;
          pixel[line * dst_width + fb_col].green = colorG;
          pixel[line * dst_width + fb_col].blue = colorB;

          colorB = y1 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y1 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y1 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);


          colorB = colorB >> 3;
          colorG = colorG >> 2;
          colorR = colorR >> 3;

          pixel[line * dst_width + fb_col + 1].red = colorR;
          pixel[line * dst_width + fb_col + 1].green = colorG;
          pixel[line * dst_width + fb_col + 1].blue = colorB;

          col = col + 1;

       }
    }
   return 0;
}


int YUYV_to_RGB565_zoom (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t dstStride)
{
    __u8 y0 = 0;
    __u8 u = 0;
    __u8 y1 = 0;
    __u8 v = 0;
    int colorB = 0;
    int colorG = 0;
    int colorR = 0;
    int line =0;
    int col =0;
    int fb_col =0;
    int dst_width = dstStride / 32 ;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src_buf;
    struct bpp16_pixel *pixel = (struct bpp16_pixel *)dst_buf;
    int fb_line = 0;

    char zoom_flag = 1;
    int zoom_parameter =  dst_width / (src_width - dst_width) ;     // (320 - 240)/240 = 3
    if(zoom_parameter < 0)
    {
       zoom_flag = 0;
    }

//    printf("src_height = %d src_width =%d dst_width = %d \n", src_height, src_width, dst_width);
    for(fb_line = 0; fb_line < 240; fb_line ++ )
    {
       col = 0 ;
       for( fb_col = 0; fb_col < dst_width; fb_col = fb_col + 2)
       {
          y0 = yuv422_samp[line * src_width/2 + col].b1;
          u  = yuv422_samp[line * src_width/2 + col].b2;
          y1 = yuv422_samp[line * src_width/2 + col].b3;
          v  = yuv422_samp[line * src_width/2 + col].b4;

          colorB = y0 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y0 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y0 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

          colorB = colorB >> 3;
          colorG = colorG >> 2;
          colorR = colorR >> 3;

          pixel[fb_line * dst_width + fb_col].red = colorR;
          pixel[fb_line * dst_width + fb_col].green = colorG;
          pixel[fb_line * dst_width + fb_col].blue = colorB;

          colorB = y1 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y1 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y1 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);


          colorB = colorB >> 3;
          colorG = colorG >> 2;
          colorR = colorR >> 3;

          pixel[fb_line * dst_width + fb_col + 1].red = colorR;
          pixel[fb_line * dst_width + fb_col + 1].green = colorG;
          pixel[fb_line * dst_width + fb_col + 1].blue = colorB;

          if(zoom_flag)
          {
                if(fb_col%zoom_parameter)
                   col = col + 1;
                else
                   col = col + 2;
          }
       }
       line ++ ;
    }
   return 0;
}

int GREY_to_RGB888 (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t dstStride)
{
    __u8 y0 = 0;
    __u8 u = 0;
    __u8 y1 = 0;
    __u8 v = 0;
    int colorB = 0;
    int colorG = 0;
    int colorR = 0;
    int line =0;
    int dst_col =0;
    int dst_width = dstStride / 32 ;
    __u8 *grey = (__u8 *)src_buf;
    struct bpp32_pixel *pixel = (struct bpp32_pixel *)dst_buf;



    u = 128;
    v = 128;
    for(line = 0; line < src_height; line++)
    {
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col + 2)
       {
          y0 = grey[line * src_width + dst_col];
          y1 = grey[line * src_width + dst_col+1];

          colorB = y0 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y0 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y0 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);


          pixel[line * dst_width + dst_col].red = colorR;
          pixel[line * dst_width + dst_col].green = colorG;
          pixel[line * dst_width + dst_col].blue = colorB;

          colorB = y1 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y1 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y1 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);



          pixel[line * dst_width + dst_col + 1].red = colorR;
          pixel[line * dst_width + dst_col + 1].green = colorG;
          pixel[line * dst_width + dst_col + 1].blue = colorB;
       }
    }
   return 0;
}

int YUYV_to_RGB888 (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t dstStride)
{
    __u8 y0 = 0;
    __u8 u = 0;
    __u8 y1 = 0;
    __u8 v = 0;
    int colorB = 0;
    int colorG = 0;
    int colorR = 0;
    int line =0;
    int col =0;
    int dst_col =0;
    int dst_width = dstStride / 32 ;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src_buf;
    struct bpp32_pixel *pixel = (struct bpp32_pixel *)dst_buf;

    for(line = 0; line < src_height; line++)
    {
       col = 0 ;
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col + 2)
       {
          y0 = yuv422_samp[line * src_width/2 + col].b1;
          u  = yuv422_samp[line * src_width/2 + col].b2;
          y1 = yuv422_samp[line * src_width/2 + col].b3;
          v  = yuv422_samp[line * src_width/2 + col].b4;

          colorB = y0 + ((443 * (v - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y0 - ((179 * (u - 128) + 86 * (v - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y0 + ((351 * (u - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

          pixel[line * dst_width + dst_col].red = colorR;
          pixel[line * dst_width + dst_col].green = colorG;
          pixel[line * dst_width + dst_col].blue = colorB;

          colorB = y1 + ((443 * (v - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y1 - ((179 * (u - 128) + 86 * (v - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y1 + ((351 * (u - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

          pixel[line * dst_width + dst_col + 1].red = colorR;
          pixel[line * dst_width + dst_col + 1].green = colorG;
          pixel[line * dst_width + dst_col + 1].blue = colorB;

           col = col + 1;
       }
    }
   return 0;
}

int YUYV_to_RGB888_FLIP_Y (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t dstStride)
{
    __u8 y0 = 0;
    __u8 u = 0;
    __u8 y1 = 0;
    __u8 v = 0;
    int colorB = 0;
    int colorG = 0;
    int colorR = 0;
    int line =0;
    int col =0;
    int dst_col =0;
    int dst_width = dstStride / 32 ;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src_buf;
    struct bpp32_pixel *pixel = (struct bpp32_pixel *)dst_buf;

    for(line = 0; line < src_height; line++)
    {
       col = 0 ;
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col + 2)
       {
          y0 = yuv422_samp[line * src_width/2 + col].b1;
          u  = yuv422_samp[line * src_width/2 + col].b2;
          y1 = yuv422_samp[line * src_width/2 + col].b3;
          v  = yuv422_samp[line * src_width/2 + col].b4;

          colorB = y0 + ((443 * (v - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y0 - ((179 * (u - 128) + 86 * (v - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y0 + ((351 * (u - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);


          pixel[line * dst_width + src_width - (dst_col)].red = colorR;
          pixel[line * dst_width + src_width - (dst_col)].green = colorG;
          pixel[line * dst_width + src_width - (dst_col)].blue = colorB;

          colorB = y1 + ((443 * (v - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y1 - ((179 * (u - 128) + 86 * (v - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y1 + ((351 * (u - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);

          pixel[line * dst_width + src_width - (dst_col + 1)].red = colorR;
          pixel[line * dst_width + src_width - (dst_col + 1)].green = colorG;
          pixel[line * dst_width + src_width - (dst_col + 1)].blue = colorB;

           col = col + 1;
       }
    }
   return 0;
}

int UYVY_to_RGB888 (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t dstStride)
{
    __u8 y0 = 0;
    __u8 u = 0;
    __u8 y1 = 0;
    __u8 v = 0;
    int colorB = 0;
    int colorG = 0;
    int colorR = 0;
    int line =0;
    int col =0;
    int dst_col =0;
    int dst_width = dstStride / 32 ;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src_buf;
    struct bpp32_pixel *pixel = (struct bpp32_pixel *)dst_buf;



    for(line = 0; line < src_height; line++)
    {
       col = 0 ;
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col + 2)
       {

          v  = yuv422_samp[line * src_width/2 + col].b1;
          y0 = yuv422_samp[line * src_width/2 + col].b2;
          u  = yuv422_samp[line * src_width/2 + col].b3;
          y1 = yuv422_samp[line * src_width/2 + col].b4;

          colorB = y0 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y0 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y0 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);


          pixel[line * dst_width + dst_col].red = colorR;
          pixel[line * dst_width + dst_col].green = colorG;
          pixel[line * dst_width + dst_col].blue = colorB;

          colorB = y1 + ((443 * (u - 128)) >> 8);
          colorB = (colorB < 0) ? 0 : ((colorB > 255 ) ? 255 : colorB);
          colorG = y1 - ((179 * (v - 128) + 86 * (u - 128)) >> 8);
          colorG = (colorG < 0) ? 0 : ((colorG > 255 ) ? 255 : colorG);
          colorR = y1 + ((351 * (v - 128)) >> 8);
          colorR = (colorR < 0) ? 0 : ((colorR > 255 ) ? 255 : colorR);



          pixel[line * dst_width + dst_col + 1].red = colorR;
          pixel[line * dst_width + dst_col + 1].green = colorG;
          pixel[line * dst_width + dst_col + 1].blue = colorB;

          col = col + 1;

       }
    }
   return 0;
}
