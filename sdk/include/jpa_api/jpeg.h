#ifndef __JPEG_H__
#define __JPEG_H__

#define uint8_t unsigned char
#define uint32_t unsigned int

/* JPEG encode quantization table select level */
typedef enum {
    LOW_QUALITY,
    MEDIUMS_QUALITY,
    HIGH_QUALITY,
} QUANT_QUALITY;

typedef struct {
    uint8_t *buf;
    uint8_t *BitStreamBuf;
#ifdef CHECK_RESULT
    uint8_t *soft_buf;
    uint8_t *soft_bts;
#endif
    unsigned int des_va;    /* descriptor virtual address */
    unsigned int des_pa;    /* descriptor physical address */
    int width;
    int height;
    int ql_sel;
    int bslen;
} YUYV_INFO;

void *JZMalloc(int align, int size);
#endif    /* __JPEG_H__ */
