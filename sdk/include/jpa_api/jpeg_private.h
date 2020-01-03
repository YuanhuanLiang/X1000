#ifndef __JPEG_PRIVATE_H__
#define __JPEG_PRIVATE__

#include "jpeg.h"
typedef struct {
    uint8_t *YBuf;
    uint8_t *BitStreamBuf;
    unsigned int des_va;    /* descriptor virtual address */
    unsigned int des_pa;    /* descriptor physical address */
    uint32_t InDaMd;
    uint32_t nmcu;
    uint32_t nrsm;
    uint32_t width;
    uint32_t height;
    uint32_t bslen;
    uint32_t ncol;
    uint32_t rsm;
    uint32_t ql_sel;
    uint8_t huffenc_sel;
} JPEGE_SwInfo;

typedef struct _JPEGE_SliceInfo {
    unsigned int des_va;    /* descriptor virtual address */
    unsigned int des_pa;    /* descriptor physical address */
    uint8_t ncol;        /* number of color/components of a MCU minus one */
    uint8_t rsm;        /* Re-sync-marker enable */
    uint8_t *bsa;        /* bitstream buffer address  */
    uint8_t nrsm;        /* Re-Sync-Marker gap number */
    uint32_t nmcu;        /* number of MCU minus one */
    uint32_t raw[3];    /* {rawy, rawu, rawv} or {rawy, rawc, N/C} */
    uint32_t stride[2];    /* {stride_y, stride_c}, only used in raster raw */
    uint32_t mb_width;
    uint32_t mb_height;
    uint8_t raw_format;

    /* Quantization level select,0-2 level */
    QUANT_QUALITY ql_sel;
    uint8_t huffenc_sel;           /* Huffman ENC Table select */
}JPEGE_SliceInfo;
/* int JPEGE_SW_Structa_Pad(JPEGE_SwInfo *swinfo, YUYV_INFO *yuyv_info); */
/* int JPEGE_Struct_Pad(JPEGE_SliceInfo *s, JPEGE_SwInfo *swinfo); */
/* #ifdef CHECK_RESULT */
/* int JPEGE_Result_check(YUYV_INFO *yuyv_info, int bslen); */
/* #endif */
/* int gen_image(YUYV_INFO *yuyv_info, FILE *fpo, int bs_len); */
/* void JPEGE_SliceInit(JPEGE_SliceInfo *s); */
/* int jpge_struct_init(YUYV_INFO *yuyv_info); */
/* int genhead(FILE *fp, YUYV_INFO *yuyv_info); */
#endif    /* __JPEG_PRIVATE__ */
