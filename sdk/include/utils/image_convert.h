/*************************************************************************
	> Filename: yuv2bmp.h
	>   Author: Qiuwei.wang
	>    Email: qiuwei.wang@ingenic.com
	> Datatime: Tue 29 Nov 2016 11:28:48 AM CST
 ************************************************************************/

#ifndef _YUV2BMP_H
#define _YUV2BMP_H
#include <types.h>
#include <lib/ffmpeg/libswscale/swscale.h>


#define FAR
#define DWORD uint32_t
#define WORD  uint16_t
#ifndef LONG
#define LONG  uint32_t
#endif
#define BYTE  uint8_t

#ifndef PACKED
#define PACKED
#endif
#ifndef GCC_PACKED
#define GCC_PACKED __attribute__((packed))
#endif

#define DEGREE_0                        0
#define DEGREE_90                       1
#define DEGREE_180                      2
#define DEGREE_270                      3

struct rgb_pixel_fmt {
    uint32_t rbit_len;
    uint32_t rbit_off;
    uint32_t gbit_len;
    uint32_t gbit_off;
    uint32_t bbit_len;
    uint32_t bbit_off;
};

/* pixel to lcd */
static inline uint32_t make_pixel(uint32_t r, uint32_t g,
                                  uint32_t b, struct rgb_pixel_fmt fmt)
{
    uint32_t pixel = (uint32_t)(((r >> (8 - fmt.rbit_len)) << fmt.rbit_off)
                              | ((g >> (8 - fmt.gbit_len)) << fmt.gbit_off)
                              | ((b >> (8 - fmt.bbit_len)) << fmt.bbit_off));
    return pixel;
}

/* structures for defining DIBs */
typedef PACKED struct tagBITMAPCOREHEADER
{
	DWORD   bcSize;                 /* used to get to color table */
	WORD    bcWidth;
	WORD    bcHeight;
	WORD    bcPlanes;
	WORD    bcBitCount;
} GCC_PACKED BITMAPCOREHEADER, FAR *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

typedef PACKED struct tagBITMAPINFOHEADER
{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} GCC_PACKED BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef PACKED struct tagRGBQUAD
{
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} GCC_PACKED RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;

typedef PACKED struct tagRGBTRIPLE
{
	BYTE    rgbtBlue;
	BYTE    rgbtGreen;
	BYTE    rgbtRed;
} GCC_PACKED RGBTRIPLE;


typedef PACKED struct tagBITMAPINFO
{
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} GCC_PACKED BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;

typedef PACKED struct tagBITMAPCOREINFO
{
	BITMAPCOREHEADER    bmciHeader;
	RGBTRIPLE           bmciColors[1];
} GCC_PACKED BITMAPCOREINFO, FAR *LPBITMAPCOREINFO, *PBITMAPCOREINFO;

typedef PACKED struct tagBITMAPFILEHEADER
{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
}GCC_PACKED BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

/*
 * Extern functions
 */
/**
* @fn int convert_yuv422p2rgb565(uint8_t *yuv, uint8_t *rgb, uint32_t width, uint32_t height);
*
* @brief  图像yuv422p 转换为 rgb
*
* @param yuv     yuv 数据
*        rgb     rgb 数据
*        width   图片宽度
*        height  图片高度
*
* @retval 0 成功
* @retval <0 失败
*
*/
int convert_yuv2rgb(uint8_t *yuv, uint8_t *rgb, uint32_t width, uint32_t height);

/**
* @fn int convert_rgb2bmp(char *filename, uint32_t width, uint32_t height,
*                         int iBitCount, uint8_t *rgbbuf);
*
* @brief  rgb图像 保存为bmp格式
*
* @param filename 输出文件名, 不带后缀
*        width   图片宽度
*        height  图片高度
*        iBitCount 位数, 只支持 24bits
*        rgbbuf  rgb 数据
*
* @retval 0 成功
* @retval <0 失败
*
*/
int convert_rgb2bmp(char *filename, uint32_t width, uint32_t height,
                    int iBitCount, uint8_t *rgbbuf);
/**
* @fn int convert_rgb2pixel(uint8_t* rgb,  uint16_t* pbuf, uint32_t width,uint32_t height,
*                           uint32_t fb_w, uint32_t fb_h, uint32_t sp_w, uint32_t sp_h,
*                           struct rgb_pixel_fmt fmt, uint8_t degree);
*
* @brief  rgb数据转换为像素点, 主要用于投屏
*
* @param rgb     rgb 数据
*        pbuf    输出buf, 一般直接传fb buffer
*        width   图片宽度
*        height  图片高度
*        fb_w    屏的宽度
*        fb_h    屏的高度
*        sp_w    投屏的起始位置
*        sp_h    投屏的起始位置
*        fmt     像素点格式
*        degree  旋转角度
*
* @retval 0 成功
* @retval <0 失败
*
* @note  fb buffer 在像素点转换及旋转操作后直接填充,所有操作在一个循环内完成, 提高效率
*
*/
int convert_rgb2pixel(uint8_t* rgb,  uint16_t* pbuf, uint32_t width,uint32_t height,
                      uint32_t fb_w, uint32_t fb_h, uint32_t sp_w, uint32_t sp_h,
                      struct rgb_pixel_fmt fmt, uint8_t degree);
/**
* @fn int convert_yuv422p_resolution(uint8_t* src_buf, int src_w, int src_h,
                                     uint8_t* dst_buf, int dst_w, int dst_h);
*
* @brief  yuv422p图像进行任意分辨率转换
*
* @param src_buf 源数据buf
*        src_w   源图片宽度
*        src_h   源图片高度
*        dst_buf 输出数据buf
*        dst_w   输出图片宽度
*        dst_h   输出图片高度
*
* @retval 0 成功
* @retval <0 失败
*
*/
int convert_yuv422p_resolution(uint8_t* src_buf, int src_w, int src_h,
                               uint8_t* dst_buf, int dst_w, int dst_h);


/**
* @fn int image_size(enum AVPixelFormat format, int width, int height);
*
* @brief  通过图像格式宽高计算图像大小
*
* @param format      图片格式
*        width       图片宽度
*        height      图片高度
*
* @retval image size
*/
int image_size(enum AVPixelFormat format, int width, int height);

/**
* @fn int image_convert(uint8_t* src_buf, int src_w, int src_h, enum AVPixelFormat src_format,
                        uint8_t* dst_buf, int dst_w, int dst_h, enum AVPixelFormat dst_format);
*
* @brief  图像转换(格式转换，分辨率转换)
*
* @param src_buf    源数据buf
*        src_w      源图片宽度
*        src_h      源图片高度
*        src_format 源图片格式
*        dst_buf    输出数据buf
*        dst_w      输出图片宽度
*        dst_h      输出图片高度
*        dst_format 输出图片格式
*
* @retval 0 成功
* @retval -1 失败
*/
int image_convert(uint8_t* src_buf, int src_w, int src_h, enum AVPixelFormat src_format,
                        uint8_t* dst_buf, int dst_w, int dst_h, enum AVPixelFormat dst_format);

/**
* @fn int image_zoom(enum AVPixelFormat format,
                    uint8_t* src_buf, int src_w, int src_h,
                    uint8_t* dst_buf, int dst_w, int dst_h);
*
* @brief  图像缩放
*
* @param format     图片格式
*        src_buf    源数据buf
*        src_w      源图片宽度
*        src_h      源图片高度
*        dst_buf    输出数据buf
*        dst_w      输出图片宽度
*        dst_h      输出图片高度
*
* @retval 0 成功
* @retval -1 失败
*/
int image_zoom(enum AVPixelFormat format,
                    uint8_t* src_buf, int src_w, int src_h,
                    uint8_t* dst_buf, int dst_w, int dst_h);



#endif /* _YUV2BMP_H */
