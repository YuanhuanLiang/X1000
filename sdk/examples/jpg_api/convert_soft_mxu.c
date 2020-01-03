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


#include <jzmedia.h>

#define i_pref(hint,base,offset)                                        \
                ({ __asm__ __volatile__("pref %0,%2(%1)"::"i"(hint),"r"(base),"i"(offset):"memory");})


#if 0
int YUYV_to_RGB565_zoom (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t drtStride )
{
    int line =0;
    int col =0;
    int dst_col =0;
    int src_cache = 1;
    int dst_width = drtStride/32;
    unsigned char *src = (unsigned char *)src_buf;
    unsigned char *dst = (unsigned char *)dst_buf;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src;
    struct bpp16_pixel *pixel = (struct bpp16_pixel *)dst;

    char zoom_flag = 1;
    int zoom_parameter =  dst_width / (src_width - dst_width) ;     // (320 - 240)/240 = 3
    if(zoom_parameter < 0)
    {
       zoom_flag = 0;
    }

    __s8  data_8  = 0;
    __s16 data_16 = 0;
    __u32 data_parameter_128  = 0x80808080;
    __u32 data_parameter_222 = 0xDEDEDEDE; //222
    __u32 data_parameter_175 = 0xAFAFAFAF; //175
    __u32 data_parameter_179 = 0xB3B3B3B3; //179
    __u32 data_parameter_86 = 0x56565656; //86
    __u32 data_parameter_0 = 0x00000000; //0

    __u8 colorB0 = 0;
    __u8 colorB1 = 0;
    __u8 colorB2 = 0;
    __u8 colorB3 = 0;

    __u8 colorR0 = 0;
    __u8 colorR1 = 0;
    __u8 colorR2 = 0;
    __u8 colorR3 = 0;

    __u8 colorG0 = 0;
    __u8 colorG1 = 0;
    __u8 colorG2 = 0;
    __u8 colorG3 = 0;

    i_pref(1, src , 0);
    for(line = 0; line < src_height; line++)
    {
       src_cache = 0;
       col = 0;
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col +8)
       {
                  if( src_cache % 2 )
                  {
                      i_pref(1, src + 32 , 0);
                  }
                  i_pref(30, dst, 0);                                          //xr14  xr11 temporary register
                  src = src + 16;
                  dst = dst + 32 ;
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 0, ptn0);      //y0
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 4, ptn1);
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 8, ptn2);
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 12, ptn3);

                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 2, ptn0);      //y1
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 6, ptn1);
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 10, ptn2);
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 14, ptn3);

                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 1, ptn0);      //u1
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 5, ptn1);      //u2
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 9, ptn2);      //u3
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 13, ptn3);     //u4

                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 3, ptn0);      //v
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 7, ptn1);
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 11, ptn2);
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 15, ptn3);

/*******************************************BLUE**************/
                  S32LDD(xr14,&data_parameter_128,0);   //xr14 = paramter

                                                        //xr13 store utntil  Green end
                  Q8ADD_SS(xr13, xr3, xr14);            //xr13 = u -128


                  S32LDD(xr14,&data_parameter_222,0);
                  Q8MULSU(xr6,xr13,xr14,xr5);    //xr5[16:0] ---x13[8:0] *x13[8:0]         (u1 - 128)*222
                                                 //xr5[32:16] ---x13[16:8] *x13[16:8]      (u2 - 128)*222
                                                 //xr5[16:0] ---x13[24:8] *x13[24:8]       (u3 - 128)*222
                                                 //xr5[32:16] ---x13[32:24] *x13[32:24]    (u4 - 128)*222

                                                    //xr7 xr8 store untile BLUE end
                  Q16SAR(xr8,xr6,xr5,xr7,7);        //XR7[15:0] = XR5[15:00]>>7     (u1 - 128)*222/128
                                                    //xR7[32:16] = XR5[32:16]>>7    (u2 - 128)*222/128
                                                    //XR8[15:0] = XR6[15:00]>>7     (u3 - 128)*222/128
                                                    //XR8[32:16]= xr6[32:16]>>7     (u4 - 128)*222/128

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);       //xr5[16: 0] = xr1[8: 0 ]      y01
                                                       //xr5[32:16] = xr1[16: 0]      y02
                                                       //xr6[16: 0] = xr1[24:16]      y03
                                                       //xr6[32:16] = xr1[32:24]      y04

                                                       //use xr14[]
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr11);  //xr14[16: 0] = xr5[16: 0] + xr7[16: 0]    y01+ (u1 -128)*222/128
                                                       //xr14[32:16] = xr5[32:16] + xr7[32: 16]   y02+ (u1 -128)*222/128

                                                       //user xr15[]
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr11);  //xr15[16: 0] = xr6[16: 0] + xr8[16: 0]    y03+ (u3 -128)*222/128
                                                       //xr15[32:16] = xr6[32:16] + xr8[32: 16]   y04+ (u4 -128)*222/128

                  Q16SAT (xr9, xr15, xr14);            //xr9[ 8: 0]  = xr14[16: 0]  y01+ (u1 -128)*222/128
                                                       //xr9[16: 8]  = xr14[32: 16] y02+ (u1 -128)*222/128
                                                       //xr9[24:16]  = xr15[16: 0 ] y03+ (u3 -128)*222/128
                                                       //xr9[32:24]  = xr16[32: 16 ] y04+ (u4 -128)*222/128


                  S8STD(xr9, &colorB0, 0, ptn0);
                  S8STD(xr9, &colorB1, 0, ptn1);
                  S8STD(xr9, &colorB2, 0, ptn2);
                  S8STD(xr9, &colorB3, 0, ptn3);
                  pixel[line * dst_width + dst_col].blue  = colorB0>>3;
                  pixel[line * dst_width + dst_col + 2].blue  = colorB1>>3;
                  pixel[line * dst_width + dst_col + 4].blue  = colorB2>>3;
                  pixel[line * dst_width + dst_col + 6].blue  = colorB3>>3;


                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr2,xr5,ptn0);
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr14);
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr14);

                  Q16SAT (xr9, xr15, xr14);

                  S8STD(xr9, &colorB0, 0, ptn0);
                  S8STD(xr9, &colorB1, 0, ptn1);
                  S8STD(xr9, &colorB2, 0, ptn2);
                  S8STD(xr9, &colorB3, 0, ptn3);
                  pixel[line * dst_width + dst_col + 1].blue  = colorB0>>3;
                  pixel[line * dst_width + dst_col + 3].blue  = colorB1>>3;
                  pixel[line * dst_width + dst_col + 5].blue  = colorB2>>3;
                  pixel[line * dst_width + dst_col + 7].blue  = colorB3>>3;


/*******************************************RED**************/
                  S32LDD(xr14,&data_parameter_128,0);   //xr14 = paramter
                                                        //xr12 store  until Green end
                  Q8ADD_SS(xr12, xr4, xr14);            //xr12 = v -128


                  S32LDD(xr14,&data_parameter_175,0);
                  Q8MULSU(xr6,xr12,xr14,xr5);    //xr5[16:0] ---x12[8:0] *x14[8:0]          (v1 - 128)*175
                                                 //xr5[32:16] ---x12[16:8] *x14[16:8]       (v2 - 128)*175
                                                 //xr5[16:0] ---x12[24:8] *x14[24:8]        (v3 - 128)*175
                                                 //xr5[32:16] ---x12[32:24] *x14[32:24]     (v4 - 128)*175


                  Q16SAR(xr8,xr6,xr5,xr7,7);        //XR7[15:0] = XR5[15:00]>>7             (v1 - 128)*175 /128
                                                    //xR7[32:16] = XR5[32:16]>>7            (v2 - 128)*175 /128
                                                    //XR8[15:0] = XR6[15:00]>>7             (v3 - 128)*175 /128
                                                    //XR8[32:16]= xr6[32:16]>>7             (v4 - 128)*175 /128

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);                  //xr5[16: 0]  = xr1[8: 0 ]      y01
                                                                  //xr5[32:16]  = xr1[16:8 ]      y02
                                                                  //xr6[16: 0]  = xr1[24:16]      y03
                                                                  //xr6[32:16]  = xr1[32:24]      y04

                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr11);             //xr14[16: 0] = xr5[16: 0] + xr7[16: 0]    y01+ (v1 -128)*175/128
                                                                  //xr14[32:16] = xr5[32:16] + xr7[32:16]    y02+ (v2 -128)*175/128

                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr11);             //xr15[16: 0] = xr6[16: 0] + xr8[16: 0]    y03+ (v3 -128)*175/128
                                                                  //xr15[32:16] = xr6[32:16] + xr8[32:16]    y04+ (v4 -128)*175/128

                  Q16SAT (xr9, xr15, xr14);                       //xr9[ 8: 0]  = xr14[16: 0]  y01+ (v1 -128)*175/128
                                                                  //xr9[16: 8]  = xr14[32:16]  y02+ (v2 -128)*175/128
                                                                  //xr9[24:16]  = xr15[16: 0]  y03+ (v3 -128)*175/128
                                                                  //xr9[32:24]  = xr15[32:16]  y04+ (v4 -128)*175/128



                  S8STD(xr9, &colorR0, 0, ptn0);
                  S8STD(xr9, &colorR1, 0, ptn1);
                  S8STD(xr9, &colorR2, 0, ptn2);
                  S8STD(xr9, &colorR3, 0, ptn3);
                  pixel[line * dst_width + dst_col].red  = colorR0>>3;
                  pixel[line * dst_width + dst_col + 2].red  = colorR1>>3;
                  pixel[line * dst_width + dst_col + 4].red  = colorR2>>3;
                  pixel[line * dst_width + dst_col + 6].red  = colorR3>>3;

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr2,xr5,ptn0);
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr14);
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr14);

                  Q16SAT (xr9, xr15, xr14);

                  S8STD(xr9, &colorR0, 0, ptn0);
                  S8STD(xr9, &colorR1, 0, ptn1);
                  S8STD(xr9, &colorR2, 0, ptn2);
                  S8STD(xr9, &colorR3, 0, ptn3);
                  pixel[line * dst_width + dst_col + 1].red  = colorR0>>3;
                  pixel[line * dst_width + dst_col + 3].red  = colorR1>>3;
                  pixel[line * dst_width + dst_col + 5].red  = colorR2>>3;
                  pixel[line * dst_width + dst_col + 7].red  = colorR3>>3;

/*****************************************GREEN****************/


                  S32LDD(xr14,&data_parameter_86,0);
                  Q8MULSU(xr6,xr13,xr14,xr5);    //xr5[16:0] ---x13[8:0] *x14[8:0]       (u1 - 128 )*86
                                                 //xr5[32:16] ---x13[16:8] *x14[16:8]    (u2 - 128 )*86
                                                 //xr6[16:0] ---x13[24:8] *x14[24:8]     (u3 - 128 )*86
                                                 //xr6[32:16] ---x13[32:24] *x14[32:24]  (u4 - 128 )*86



                  S32LDD(xr14,&data_parameter_179,0);
                  Q8MULSU(xr8,xr12,xr14,xr7);    //xr7[16: 0] ---x12[8: 0]  *x14[8 : 0]        (v1 - 128 )*179
                                                 //xr7[32:16] ---x12[16:8]  *x14[16: 8]        (v2 - 128 )*179
                                                 //xr8[16: 0] ---x12[24:16] *x14[32: 16]       (v3 - 128 )*179
                                                 //xr8[32:16] ---x12[32:24] *x14[32: 24]       (v4 - 128 )*179

                  Q16ADD_AA_WW(xr9, xr5, xr7 ,xr11);    //xr9[16: 0] =  xr5[16: 0]  + xr7[16: 0]     (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr9[32: 16] = xr5[32: 16] + xr7[32: 16]    (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_AA_WW(xr10, xr6, xr8 ,xr11);   //xr10[16: 0] =  xr6[16: 0]  + xr8[16: 0]     (u3 - 128 )*86 + (v3 -128)*179
                                                        //xr10[32: 16] = xr6[32: 16] + xr8[32: 16]    (u4 - 128 )*86 + (v4 -128)*179


                                                    //at the time  xr12 xr13 able
                  Q16SAR(xr12,xr10,xr9,xr11,8);     //XR11[15 :0] = XR9[15: 0]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //xr11[32:16] = XR9[32:16]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //XR12[15: 0] = XR10[15:0]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //XR12[32:15] = XR10[32:16]>>8      [(u1 - 128 )*86 + (v1 -128)*179] /256

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);                  //xr5[16: 0]  = xr1[8: 0 ]      y01
                                                                  //xr5[32:16]  = xr1[16:8 ]      y02
                                                                  //xr6[16: 0]  = xr1[24:16]      y03
                                                                  //xr6[32:16]  = xr1[32:24]      y04



                  Q16ADD_SS_WW(xr13, xr5, xr11 ,xr15);  //xr13[16: 0] =  xr5[16: 0]  -  xr7[16: 0]   y01 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr13[32: 16] = xr5[32: 16] - xr7[32: 16]   y02 - (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_SS_WW(xr14, xr6, xr12 ,xr15);  //xr14[16: 0] =  xr6[16: 0]  - xr8[16: 0]    y03 -(u3 - 128 )*86 + (v3 -128)*179
                                                        //xr14[32: 16] = xr6[32: 16] - xr8[32: 16]   y04 -(u4 - 128 )*86 + (v4 -128)*179


                  Q16SAT (xr15, xr14, xr13);            //xr15[ 8: 0]  = xr13[16: 0]  y01 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[16: 8]  = xr13[32:16]  y02 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[24:16]  = xr14[16: 0]  y03 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[32:24]  = xr14[32:16]  y04 - (u1 - 128 )*86 + (v1 -128)*179


                  S8STD(xr15, &colorG0, 0, ptn0);
                  S8STD(xr15, &colorG1, 0, ptn1);
                  S8STD(xr15, &colorG2, 0, ptn2);
                  S8STD(xr15, &colorG3, 0, ptn3);
                  pixel[line * dst_width + dst_col].green  = colorG0>>2;
                  pixel[line * dst_width + dst_col + 2].green  = colorG1>>2;
                  pixel[line * dst_width + dst_col + 4].green  = colorG2>>2;
                  pixel[line * dst_width + dst_col + 6].green  = colorG3>>2;



                  S32LDD(xr14,&data_parameter_0,0);
                  S32SFL(xr6,xr14,xr2,xr5,ptn0);                  //xr5[16: 0]  = xr2[8: 0 ]      y11
                                                                  //xr5[32:16]  = xr2[16:8 ]      y12
                                                                  //xr6[16: 0]  = xr2[24:16]      y13
                                                                  //xr6[32:16]  = xr2[32:24]      y14

                  Q16ADD_SS_WW(xr13, xr5, xr11 ,xr15);  //xr13[16: 0] =  xr5[16: 0]  -  xr7[16: 0]   y11 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr13[32: 16] = xr5[32: 16] - xr7[32: 16]   y12 - (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_SS_WW(xr14, xr6, xr12 ,xr15);  //xr14[16: 0] =  xr6[16: 0]  - xr8[16: 0]    y13 -(u3 - 128 )*86 + (v3 -128)*179
                                                        //xr14[32: 16] = xr6[32: 16] - xr8[32: 16]   y14 -(u4 - 128 )*86 + (v4 -128)*179


                  Q16SAT (xr15, xr14, xr13);            //xr15[ 8: 0]  = xr13[16: 0]  y11 - (u1 - 128 )*86 + (v1 -128)*179


                  S8STD(xr15, &colorG0, 0, ptn0);
                  S8STD(xr15, &colorG1, 0, ptn1);
                  S8STD(xr15, &colorG2, 0, ptn2);
                  S8STD(xr15, &colorG3, 0, ptn3);
                  pixel[line * dst_width + dst_col + 1].green  = colorG0>>2;
                  pixel[line * dst_width + dst_col + 3].green  = colorG1>>2;
                  pixel[line * dst_width + dst_col + 5].green  = colorG2>>2;
                  pixel[line * dst_width + dst_col + 7].green  = colorG3>>2;



                  if(zoom_flag)
                  {
                        if(dst_col%zoom_parameter)
                           col = col + 4;
                        else
                           col = col + 8;
                  }

                  src_cache = src_cache + 1;
       }
    }
    return 0;
}






int YUYV_to_RGB565 (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t drtStride ,size_t dst_height)
{
    int line =0;
    int col =0;
    int dst_col =0;
    int src_cache = 1;
    int dst_width = drtStride/32;
    unsigned char *src = (unsigned char *)src_buf;
    unsigned char *dst = (unsigned char *)dst_buf;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src;
    struct bpp16_pixel *pixel = (struct bpp16_pixel *)dst;


    __s8  data_8  = 0;
    __s16 data_16 = 0;
    __u32 data_parameter_128  = 0x80808080;
    __u32 data_parameter_222 = 0xDEDEDEDE; //222
    __u32 data_parameter_175 = 0xAFAFAFAF; //175
    __u32 data_parameter_179 = 0xB3B3B3B3; //179
    __u32 data_parameter_86 = 0x56565656; //86
    __u32 data_parameter_0 = 0x00000000; //0

    __u8 colorB0 = 0;
    __u8 colorB1 = 0;
    __u8 colorB2 = 0;
    __u8 colorB3 = 0;

    __u8 colorR0 = 0;
    __u8 colorR1 = 0;
    __u8 colorR2 = 0;
    __u8 colorR3 = 0;

    __u8 colorG0 = 0;
    __u8 colorG1 = 0;
    __u8 colorG2 = 0;
    __u8 colorG3 = 0;

    i_pref(1, src , 0);
    for(line = 0; line < dst_height; line++)
    {
       src_cache = 0;
       col = 0;
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col +8)
       {
                  if( src_cache % 2 )
                  {
                      i_pref(1, src + 32 , 0);
                  }
                  i_pref(30, dst, 0);                                          //xr14  xr11 temporary register
                  src = src + 16;
                  dst = dst + 32 ;
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 0, ptn0);      //y0
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 4, ptn1);
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 8, ptn2);
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 12, ptn3);

                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 2, ptn0);      //y1
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 6, ptn1);
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 10, ptn2);
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 14, ptn3);

                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 1, ptn0);      //u1
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 5, ptn1);      //u2
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 9, ptn2);      //u3
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 13, ptn3);     //u4

                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 3, ptn0);      //v
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 7, ptn1);
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 11, ptn2);
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 15, ptn3);

/*******************************************BLUE**************/
                  S32LDD(xr14,&data_parameter_128,0);   //xr14 = paramter

                                                        //xr13 store utntil  Green end
                  Q8ADD_SS(xr13, xr3, xr14);            //xr13 = u -128


                  S32LDD(xr14,&data_parameter_222,0);
                  Q8MULSU(xr6,xr13,xr14,xr5);    //xr5[16:0] ---x13[8:0] *x13[8:0]         (u1 - 128)*222
                                                 //xr5[32:16] ---x13[16:8] *x13[16:8]      (u2 - 128)*222
                                                 //xr5[16:0] ---x13[24:8] *x13[24:8]       (u3 - 128)*222
                                                 //xr5[32:16] ---x13[32:24] *x13[32:24]    (u4 - 128)*222

                                                    //xr7 xr8 store untile BLUE end
                  Q16SAR(xr8,xr6,xr5,xr7,7);        //XR7[15:0] = XR5[15:00]>>7     (u1 - 128)*222/128
                                                    //xR7[32:16] = XR5[32:16]>>7    (u2 - 128)*222/128
                                                    //XR8[15:0] = XR6[15:00]>>7     (u3 - 128)*222/128
                                                    //XR8[32:16]= xr6[32:16]>>7     (u4 - 128)*222/128

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);       //xr5[16: 0] = xr1[8: 0 ]      y01
                                                       //xr5[32:16] = xr1[16: 0]      y02
                                                       //xr6[16: 0] = xr1[24:16]      y03
                                                       //xr6[32:16] = xr1[32:24]      y04

                                                       //use xr14[]
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr11);  //xr14[16: 0] = xr5[16: 0] + xr7[16: 0]    y01+ (u1 -128)*222/128
                                                       //xr14[32:16] = xr5[32:16] + xr7[32: 16]   y02+ (u1 -128)*222/128

                                                       //user xr15[]
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr11);  //xr15[16: 0] = xr6[16: 0] + xr8[16: 0]    y03+ (u3 -128)*222/128
                                                       //xr15[32:16] = xr6[32:16] + xr8[32: 16]   y04+ (u4 -128)*222/128

                  Q16SAT (xr9, xr15, xr14);            //xr9[ 8: 0]  = xr14[16: 0]  y01+ (u1 -128)*222/128
                                                       //xr9[16: 8]  = xr14[32: 16] y02+ (u1 -128)*222/128
                                                       //xr9[24:16]  = xr15[16: 0 ] y03+ (u3 -128)*222/128
                                                       //xr9[32:24]  = xr16[32: 16 ] y04+ (u4 -128)*222/128


                  S8STD(xr9, &colorB0, 0, ptn0);
                  S8STD(xr9, &colorB1, 0, ptn1);
                  S8STD(xr9, &colorB2, 0, ptn2);
                  S8STD(xr9, &colorB3, 0, ptn3);
                  pixel[line * dst_width + dst_col].blue  = colorB0>>3;
                  pixel[line * dst_width + dst_col + 2].blue  = colorB1>>3;
                  pixel[line * dst_width + dst_col + 4].blue  = colorB2>>3;
                  pixel[line * dst_width + dst_col + 6].blue  = colorB3>>3;


                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr2,xr5,ptn0);
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr14);
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr14);

                  Q16SAT (xr9, xr15, xr14);

                  S8STD(xr9, &colorB0, 0, ptn0);
                  S8STD(xr9, &colorB1, 0, ptn1);
                  S8STD(xr9, &colorB2, 0, ptn2);
                  S8STD(xr9, &colorB3, 0, ptn3);
                  pixel[line * dst_width + dst_col + 1].blue  = colorB0>>3;
                  pixel[line * dst_width + dst_col + 3].blue  = colorB1>>3;
                  pixel[line * dst_width + dst_col + 5].blue  = colorB2>>3;
                  pixel[line * dst_width + dst_col + 7].blue  = colorB3>>3;


/*******************************************RED**************/
                  S32LDD(xr14,&data_parameter_128,0);   //xr14 = paramter
                                                        //xr12 store  until Green end
                  Q8ADD_SS(xr12, xr4, xr14);            //xr12 = v -128


                  S32LDD(xr14,&data_parameter_175,0);
                  Q8MULSU(xr6,xr12,xr14,xr5);    //xr5[16:0] ---x12[8:0] *x14[8:0]          (v1 - 128)*175
                                                 //xr5[32:16] ---x12[16:8] *x14[16:8]       (v2 - 128)*175
                                                 //xr5[16:0] ---x12[24:8] *x14[24:8]        (v3 - 128)*175
                                                 //xr5[32:16] ---x12[32:24] *x14[32:24]     (v4 - 128)*175


                  Q16SAR(xr8,xr6,xr5,xr7,7);        //XR7[15:0] = XR5[15:00]>>7             (v1 - 128)*175 /128
                                                    //xR7[32:16] = XR5[32:16]>>7            (v2 - 128)*175 /128
                                                    //XR8[15:0] = XR6[15:00]>>7             (v3 - 128)*175 /128
                                                    //XR8[32:16]= xr6[32:16]>>7             (v4 - 128)*175 /128

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);                  //xr5[16: 0]  = xr1[8: 0 ]      y01
                                                                  //xr5[32:16]  = xr1[16:8 ]      y02
                                                                  //xr6[16: 0]  = xr1[24:16]      y03
                                                                  //xr6[32:16]  = xr1[32:24]      y04

                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr11);             //xr14[16: 0] = xr5[16: 0] + xr7[16: 0]    y01+ (v1 -128)*175/128
                                                                  //xr14[32:16] = xr5[32:16] + xr7[32:16]    y02+ (v2 -128)*175/128

                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr11);             //xr15[16: 0] = xr6[16: 0] + xr8[16: 0]    y03+ (v3 -128)*175/128
                                                                  //xr15[32:16] = xr6[32:16] + xr8[32:16]    y04+ (v4 -128)*175/128

                  Q16SAT (xr9, xr15, xr14);                       //xr9[ 8: 0]  = xr14[16: 0]  y01+ (v1 -128)*175/128
                                                                  //xr9[16: 8]  = xr14[32:16]  y02+ (v2 -128)*175/128
                                                                  //xr9[24:16]  = xr15[16: 0]  y03+ (v3 -128)*175/128
                                                                  //xr9[32:24]  = xr15[32:16]  y04+ (v4 -128)*175/128



                  S8STD(xr9, &colorR0, 0, ptn0);
                  S8STD(xr9, &colorR1, 0, ptn1);
                  S8STD(xr9, &colorR2, 0, ptn2);
                  S8STD(xr9, &colorR3, 0, ptn3);
                  pixel[line * dst_width + dst_col].red  = colorR0>>3;
                  pixel[line * dst_width + dst_col + 2].red  = colorR1>>3;
                  pixel[line * dst_width + dst_col + 4].red  = colorR2>>3;
                  pixel[line * dst_width + dst_col + 6].red  = colorR3>>3;

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr2,xr5,ptn0);
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr14);
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr14);

                  Q16SAT (xr9, xr15, xr14);

                  S8STD(xr9, &colorR0, 0, ptn0);
                  S8STD(xr9, &colorR1, 0, ptn1);
                  S8STD(xr9, &colorR2, 0, ptn2);
                  S8STD(xr9, &colorR3, 0, ptn3);
                  pixel[line * dst_width + dst_col + 1].red  = colorR0>>3;
                  pixel[line * dst_width + dst_col + 3].red  = colorR1>>3;
                  pixel[line * dst_width + dst_col + 5].red  = colorR2>>3;
                  pixel[line * dst_width + dst_col + 7].red  = colorR3>>3;

/*****************************************GREEN****************/


                  S32LDD(xr14,&data_parameter_86,0);
                  Q8MULSU(xr6,xr13,xr14,xr5);    //xr5[16:0] ---x13[8:0] *x14[8:0]       (u1 - 128 )*86
                                                 //xr5[32:16] ---x13[16:8] *x14[16:8]    (u2 - 128 )*86
                                                 //xr6[16:0] ---x13[24:8] *x14[24:8]     (u3 - 128 )*86
                                                 //xr6[32:16] ---x13[32:24] *x14[32:24]  (u4 - 128 )*86



                  S32LDD(xr14,&data_parameter_179,0);
                  Q8MULSU(xr8,xr12,xr14,xr7);    //xr7[16: 0] ---x12[8: 0]  *x14[8 : 0]        (v1 - 128 )*179
                                                 //xr7[32:16] ---x12[16:8]  *x14[16: 8]        (v2 - 128 )*179
                                                 //xr8[16: 0] ---x12[24:16] *x14[32: 16]       (v3 - 128 )*179
                                                 //xr8[32:16] ---x12[32:24] *x14[32: 24]       (v4 - 128 )*179

                  Q16ADD_AA_WW(xr9, xr5, xr7 ,xr11);    //xr9[16: 0] =  xr5[16: 0]  + xr7[16: 0]     (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr9[32: 16] = xr5[32: 16] + xr7[32: 16]    (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_AA_WW(xr10, xr6, xr8 ,xr11);   //xr10[16: 0] =  xr6[16: 0]  + xr8[16: 0]     (u3 - 128 )*86 + (v3 -128)*179
                                                        //xr10[32: 16] = xr6[32: 16] + xr8[32: 16]    (u4 - 128 )*86 + (v4 -128)*179


                                                    //at the time  xr12 xr13 able
                  Q16SAR(xr12,xr10,xr9,xr11,8);     //XR11[15 :0] = XR9[15: 0]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //xr11[32:16] = XR9[32:16]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //XR12[15: 0] = XR10[15:0]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //XR12[32:15] = XR10[32:16]>>8      [(u1 - 128 )*86 + (v1 -128)*179] /256

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);                  //xr5[16: 0]  = xr1[8: 0 ]      y01
                                                                  //xr5[32:16]  = xr1[16:8 ]      y02
                                                                  //xr6[16: 0]  = xr1[24:16]      y03
                                                                  //xr6[32:16]  = xr1[32:24]      y04



                  Q16ADD_SS_WW(xr13, xr5, xr11 ,xr15);  //xr13[16: 0] =  xr5[16: 0]  -  xr7[16: 0]   y01 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr13[32: 16] = xr5[32: 16] - xr7[32: 16]   y02 - (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_SS_WW(xr14, xr6, xr12 ,xr15);  //xr14[16: 0] =  xr6[16: 0]  - xr8[16: 0]    y03 -(u3 - 128 )*86 + (v3 -128)*179
                                                        //xr14[32: 16] = xr6[32: 16] - xr8[32: 16]   y04 -(u4 - 128 )*86 + (v4 -128)*179


                  Q16SAT (xr15, xr14, xr13);            //xr15[ 8: 0]  = xr13[16: 0]  y01 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[16: 8]  = xr13[32:16]  y02 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[24:16]  = xr14[16: 0]  y03 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[32:24]  = xr14[32:16]  y04 - (u1 - 128 )*86 + (v1 -128)*179


                  S8STD(xr15, &colorG0, 0, ptn0);
                  S8STD(xr15, &colorG1, 0, ptn1);
                  S8STD(xr15, &colorG2, 0, ptn2);
                  S8STD(xr15, &colorG3, 0, ptn3);
                  pixel[line * dst_width + dst_col].green  = colorG0>>2;
                  pixel[line * dst_width + dst_col + 2].green  = colorG1>>2;
                  pixel[line * dst_width + dst_col + 4].green  = colorG2>>2;
                  pixel[line * dst_width + dst_col + 6].green  = colorG3>>2;



                  S32LDD(xr14,&data_parameter_0,0);
                  S32SFL(xr6,xr14,xr2,xr5,ptn0);                  //xr5[16: 0]  = xr2[8: 0 ]      y11
                                                                  //xr5[32:16]  = xr2[16:8 ]      y12
                                                                  //xr6[16: 0]  = xr2[24:16]      y13
                                                                  //xr6[32:16]  = xr2[32:24]      y14

                  Q16ADD_SS_WW(xr13, xr5, xr11 ,xr15);  //xr13[16: 0] =  xr5[16: 0]  -  xr7[16: 0]   y11 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr13[32: 16] = xr5[32: 16] - xr7[32: 16]   y12 - (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_SS_WW(xr14, xr6, xr12 ,xr15);  //xr14[16: 0] =  xr6[16: 0]  - xr8[16: 0]    y13 -(u3 - 128 )*86 + (v3 -128)*179
                                                        //xr14[32: 16] = xr6[32: 16] - xr8[32: 16]   y14 -(u4 - 128 )*86 + (v4 -128)*179


                  Q16SAT (xr15, xr14, xr13);            //xr15[ 8: 0]  = xr13[16: 0]  y11 - (u1 - 128 )*86 + (v1 -128)*179


                  S8STD(xr15, &colorG0, 0, ptn0);
                  S8STD(xr15, &colorG1, 0, ptn1);
                  S8STD(xr15, &colorG2, 0, ptn2);
                  S8STD(xr15, &colorG3, 0, ptn3);
                  pixel[line * dst_width + dst_col + 1].green  = colorG0>>2;
                  pixel[line * dst_width + dst_col + 3].green  = colorG1>>2;
                  pixel[line * dst_width + dst_col + 5].green  = colorG2>>2;
                  pixel[line * dst_width + dst_col + 7].green  = colorG3>>2;

                  col = col + 4;
                  src_cache = src_cache + 1;

       }
    }
    return 0;
}



int  YUYV_to_RGB888 (void *src_buf, void *dst_buf, size_t src_height ,size_t src_width, size_t drtStride )
{
    int line =0;
    int col =0;
    int dst_col =0;
    int src_cache = 1;
    int dst_width = drtStride/32;
    unsigned char *src = (unsigned char *)src_buf;
    unsigned char *dst = (unsigned char *)dst_buf;
    struct yuv422_sample *yuv422_samp = (struct yuv422_sample*)src;
    struct bpp32_pixel *pixel = (struct bpp32_pixel *)dst;


    __s8  data_8  = 0;
    __s16 data_16 = 0;
    __u32 data_parameter_128  = 0x80808080;
    __u32 data_parameter_222 = 0xDEDEDEDE; //222
    __u32 data_parameter_175 = 0xAFAFAFAF; //175
    __u32 data_parameter_179 = 0xB3B3B3B3; //179
    __u32 data_parameter_86 = 0x56565656; //86
    __u32 data_parameter_0 = 0x00000000; //0

    i_pref(1, src , 0);
    for(line = 0; line < src_height; line++)
    {
       src_cache = 0;
       col = 0;
       for( dst_col = 0; dst_col < dst_width; dst_col = dst_col +8)
       {
                  if( src_cache % 2 )
                  {
                      i_pref(1, src + 32 , 0);
                  }
                  i_pref(30, dst, 0);                                          //xr14  xr11 temporary register
                  src = src + 16;
                  dst = dst + 32 ;

                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 0, ptn0);      //y0
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 4, ptn1);
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 8, ptn2);
                  S8LDD(xr1, &yuv422_samp[line * src_width/2 + col], 12, ptn3);

                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 2, ptn0);      //y1
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 6, ptn1);
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 10, ptn2);
                  S8LDD(xr2, &yuv422_samp[line * src_width/2 + col], 14, ptn3);

                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 1, ptn0);      //u1
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 5, ptn1);      //u2
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 9, ptn2);      //u3
                  S8LDD(xr3, &yuv422_samp[line * src_width/2 + col], 13, ptn3);     //u4

                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 3, ptn0);      //v
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 7, ptn1);
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 11, ptn2);
                  S8LDD(xr4, &yuv422_samp[line * src_width/2 + col], 15, ptn3);
/*******************************************BLUE**************/
                  S32LDD(xr14,&data_parameter_128,0);   //xr14 = paramter

                                                        //xr13 store utntil  Green end
                  Q8ADD_SS(xr13, xr3, xr14);            //xr13 = u -128


                  S32LDD(xr14,&data_parameter_222,0);
                  Q8MULSU(xr6,xr13,xr14,xr5);    //xr5[16:0] ---x13[8:0] *x13[8:0]         (u1 - 128)*222
                                                 //xr5[32:16] ---x13[16:8] *x13[16:8]      (u2 - 128)*222
                                                 //xr5[16:0] ---x13[24:8] *x13[24:8]       (u3 - 128)*222
                                                 //xr5[32:16] ---x13[32:24] *x13[32:24]    (u4 - 128)*222

                                                    //xr7 xr8 store untile BLUE end
                  Q16SAR(xr8,xr6,xr5,xr7,7);        //XR7[15:0] = XR5[15:00]>>7     (u1 - 128)*222/128
                                                    //xR7[32:16] = XR5[32:16]>>7    (u2 - 128)*222/128
                                                    //XR8[15:0] = XR6[15:00]>>7     (u3 - 128)*222/128
                                                    //XR8[32:16]= xr6[32:16]>>7     (u4 - 128)*222/128

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);       //xr5[16: 0] = xr1[8: 0 ]      y01
                                                       //xr5[32:16] = xr1[16: 0]      y02
                                                       //xr6[16: 0] = xr1[24:16]      y03
                                                       //xr6[32:16] = xr1[32:24]      y04

                                                       //use xr14[]
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr11);  //xr14[16: 0] = xr5[16: 0] + xr7[16: 0]    y01+ (u1 -128)*222/128
                                                       //xr14[32:16] = xr5[32:16] + xr7[32: 16]   y02+ (u1 -128)*222/128

                                                       //user xr15[]
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr11);  //xr15[16: 0] = xr6[16: 0] + xr8[16: 0]    y03+ (u3 -128)*222/128
                                                       //xr15[32:16] = xr6[32:16] + xr8[32: 16]   y04+ (u4 -128)*222/128

                  Q16SAT (xr9, xr15, xr14);            //xr9[ 8: 0]  = xr14[16: 0]  y01+ (u1 -128)*222/128
                                                       //xr9[16: 8]  = xr14[32: 16] y02+ (u1 -128)*222/128
                                                       //xr9[24:16]  = xr15[16: 0 ] y03+ (u3 -128)*222/128
                                                       //xr9[32:24]  = xr16[32: 16 ] y04+ (u4 -128)*222/128

                  S8STD(xr9, &pixel[line * dst_width + dst_col].blue, 0, ptn0);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 2].blue, 0, ptn1);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 4].blue, 0, ptn2);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 6].blue, 0, ptn3);


                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr2,xr5,ptn0);
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr14);
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr14);

                  Q16SAT (xr9, xr15, xr14);

                  S8STD(xr9, &pixel[line * dst_width + dst_col + 1].blue, 0, ptn0);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 3].blue, 0, ptn1);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 5].blue, 0, ptn2);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 7].blue, 0, ptn3);


/*******************************************RED**************/
                  S32LDD(xr14,&data_parameter_128,0);   //xr14 = paramter
                                                        //xr12 store  until Green end
                  Q8ADD_SS(xr12, xr4, xr14);            //xr12 = v -128


                  S32LDD(xr14,&data_parameter_175,0);
                  Q8MULSU(xr6,xr12,xr14,xr5);    //xr5[16:0] ---x12[8:0] *x14[8:0]          (v1 - 128)*175
                                                 //xr5[32:16] ---x12[16:8] *x14[16:8]       (v2 - 128)*175
                                                 //xr5[16:0] ---x12[24:8] *x14[24:8]        (v3 - 128)*175
                                                 //xr5[32:16] ---x12[32:24] *x14[32:24]     (v4 - 128)*175


                  Q16SAR(xr8,xr6,xr5,xr7,7);        //XR7[15:0] = XR5[15:00]>>7             (v1 - 128)*175 /128
                                                    //xR7[32:16] = XR5[32:16]>>7            (v2 - 128)*175 /128
                                                    //XR8[15:0] = XR6[15:00]>>7             (v3 - 128)*175 /128
                                                    //XR8[32:16]= xr6[32:16]>>7             (v4 - 128)*175 /128

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);                  //xr5[16: 0]  = xr1[8: 0 ]      y01
                                                                  //xr5[32:16]  = xr1[16:8 ]      y02
                                                                  //xr6[16: 0]  = xr1[24:16]      y03
                                                                  //xr6[32:16]  = xr1[32:24]      y04

                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr11);             //xr14[16: 0] = xr5[16: 0] + xr7[16: 0]    y01+ (v1 -128)*175/128
                                                                  //xr14[32:16] = xr5[32:16] + xr7[32:16]    y02+ (v2 -128)*175/128

                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr11);             //xr15[16: 0] = xr6[16: 0] + xr8[16: 0]    y03+ (v3 -128)*175/128
                                                                  //xr15[32:16] = xr6[32:16] + xr8[32:16]    y04+ (v4 -128)*175/128

                  Q16SAT (xr9, xr15, xr14);                       //xr9[ 8: 0]  = xr14[16: 0]  y01+ (v1 -128)*175/128
                                                                  //xr9[16: 8]  = xr14[32:16]  y02+ (v2 -128)*175/128
                                                                  //xr9[24:16]  = xr15[16: 0]  y03+ (v3 -128)*175/128
                                                                  //xr9[32:24]  = xr15[32:16]  y04+ (v4 -128)*175/128


                  S8STD(xr9, &pixel[line * dst_width + dst_col].red, 0, ptn0);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 2 ].red, 0, ptn1);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 4 ].red, 0, ptn2);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 6 ].red, 0, ptn3);


                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr2,xr5,ptn0);
                  Q16ADD_AA_WW(xr14, xr5, xr7 ,xr14);
                  Q16ADD_AA_WW(xr15, xr6, xr8 ,xr14);

                  Q16SAT (xr9, xr15, xr14);

                  S8STD(xr9, &pixel[line * dst_width + dst_col + 1 ].red, 0, ptn0);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 3 ].red, 0, ptn1);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 5 ].red, 0, ptn2);
                  S8STD(xr9, &pixel[line * dst_width + dst_col + 7 ].red, 0, ptn3);

/*****************************************GREEN****************/


                  S32LDD(xr14,&data_parameter_86,0);
                  Q8MULSU(xr6,xr13,xr14,xr5);    //xr5[16:0] ---x13[8:0] *x14[8:0]       (u1 - 128 )*86
                                                 //xr5[32:16] ---x13[16:8] *x14[16:8]    (u2 - 128 )*86
                                                 //xr6[16:0] ---x13[24:8] *x14[24:8]     (u3 - 128 )*86
                                                 //xr6[32:16] ---x13[32:24] *x14[32:24]  (u4 - 128 )*86



                  S32LDD(xr14,&data_parameter_179,0);
                  Q8MULSU(xr8,xr12,xr14,xr7);    //xr7[16: 0] ---x12[8: 0]  *x14[8 : 0]        (v1 - 128 )*179
                                                 //xr7[32:16] ---x12[16:8]  *x14[16: 8]        (v2 - 128 )*179
                                                 //xr8[16: 0] ---x12[24:16] *x14[32: 16]       (v3 - 128 )*179
                                                 //xr8[32:16] ---x12[32:24] *x14[32: 24]       (v4 - 128 )*179

                  Q16ADD_AA_WW(xr9, xr5, xr7 ,xr11);    //xr9[16: 0] =  xr5[16: 0]  + xr7[16: 0]     (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr9[32: 16] = xr5[32: 16] + xr7[32: 16]    (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_AA_WW(xr10, xr6, xr8 ,xr11);   //xr10[16: 0] =  xr6[16: 0]  + xr8[16: 0]     (u3 - 128 )*86 + (v3 -128)*179
                                                        //xr10[32: 16] = xr6[32: 16] + xr8[32: 16]    (u4 - 128 )*86 + (v4 -128)*179


                                                    //at the time  xr12 xr13 able
                  Q16SAR(xr12,xr10,xr9,xr11,8);     //XR11[15 :0] = XR9[15: 0]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //xr11[32:16] = XR9[32:16]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //XR12[15: 0] = XR10[15:0]>>8       [(u1 - 128 )*86 + (v1 -128)*179] /256
                                                    //XR12[32:15] = XR10[32:16]>>8      [(u1 - 128 )*86 + (v1 -128)*179] /256

                  S32LDD(xr14,&data_parameter_0,0);

                  S32SFL(xr6,xr14,xr1,xr5,ptn0);                  //xr5[16: 0]  = xr1[8: 0 ]      y01
                                                                  //xr5[32:16]  = xr1[16:8 ]      y02
                                                                  //xr6[16: 0]  = xr1[24:16]      y03
                                                                  //xr6[32:16]  = xr1[32:24]      y04



                  Q16ADD_SS_WW(xr13, xr5, xr11 ,xr15);  //xr13[16: 0] =  xr5[16: 0]  -  xr7[16: 0]   y01 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr13[32: 16] = xr5[32: 16] - xr7[32: 16]   y02 - (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_SS_WW(xr14, xr6, xr12 ,xr15);  //xr14[16: 0] =  xr6[16: 0]  - xr8[16: 0]    y03 -(u3 - 128 )*86 + (v3 -128)*179
                                                        //xr14[32: 16] = xr6[32: 16] - xr8[32: 16]   y04 -(u4 - 128 )*86 + (v4 -128)*179


                  Q16SAT (xr15, xr14, xr13);            //xr15[ 8: 0]  = xr13[16: 0]  y01 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[16: 8]  = xr13[32:16]  y02 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[24:16]  = xr14[16: 0]  y03 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr15[32:24]  = xr14[32:16]  y04 - (u1 - 128 )*86 + (v1 -128)*179




                  S8STD(xr15, &pixel[line * dst_width + dst_col].green, 0, ptn0);
                  S8STD(xr15, &pixel[line * dst_width + dst_col + 2 ].green, 0, ptn1);
                  S8STD(xr15, &pixel[line * dst_width + dst_col + 4 ].green, 0, ptn2);
                  S8STD(xr15, &pixel[line * dst_width + dst_col + 6 ].green, 0, ptn3);

                  S32LDD(xr14,&data_parameter_0,0);
                  S32SFL(xr6,xr14,xr2,xr5,ptn0);                  //xr5[16: 0]  = xr2[8: 0 ]      y11
                                                                  //xr5[32:16]  = xr2[16:8 ]      y12
                                                                  //xr6[16: 0]  = xr2[24:16]      y13
                                                                  //xr6[32:16]  = xr2[32:24]      y14

                  Q16ADD_SS_WW(xr13, xr5, xr11 ,xr15);  //xr13[16: 0] =  xr5[16: 0]  -  xr7[16: 0]   y11 - (u1 - 128 )*86 + (v1 -128)*179
                                                        //xr13[32: 16] = xr5[32: 16] - xr7[32: 16]   y12 - (u2 - 128 )*86 + (v2 -128)*179

                  Q16ADD_SS_WW(xr14, xr6, xr12 ,xr15);  //xr14[16: 0] =  xr6[16: 0]  - xr8[16: 0]    y13 -(u3 - 128 )*86 + (v3 -128)*179
                                                        //xr14[32: 16] = xr6[32: 16] - xr8[32: 16]   y14 -(u4 - 128 )*86 + (v4 -128)*179


                  Q16SAT (xr15, xr14, xr13);            //xr15[ 8: 0]  = xr13[16: 0]  y11 - (u1 - 128 )*86 + (v1 -128)*179

                  S8STD(xr15, &pixel[line * dst_width + dst_col + 1].green, 0, ptn0);
                  S8STD(xr15, &pixel[line * dst_width + dst_col + 3].green, 0, ptn1);
                  S8STD(xr15, &pixel[line * dst_width + dst_col + 5].green, 0, ptn2);
                  S8STD(xr15, &pixel[line * dst_width + dst_col + 7].green, 0, ptn3);

                  col = col + 4;
                  src_cache = src_cache + 1;

       }
    }
    return 0;
}
#endif
