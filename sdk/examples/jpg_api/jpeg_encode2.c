#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <jpa_api/jpeg.h>
#include <jpa_api/jz_mem.h>
#include <jpa_api/jpeg_private.h>
#include <jpa_api/vpu_common.h>
#include <jpa_api/jzm_jpeg_enc.h>
#include <jpa_api/qt.h>
#include <jpa_api/ht.h>
#include <jpa_api/head.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* #define OPERATE_IN_KERNEL */


//#define V4L2_SRC_BUF_MMAP
//#define V4L2_DST_BUF_MMAP
#include <sys/mman.h>
#include <errno.h>
#include <linux/videodev2.h>
#define BUFFER_COUNT 1
typedef struct VideoBuffer {
    void   *start;
    size_t  length;
} VideoBuffer;
VideoBuffer jpegdstbuf[BUFFER_COUNT];
VideoBuffer jpegsrcbuf[BUFFER_COUNT];


struct jz_jpeg_encode{
        void *vpu;
        YUYV_INFO yuyv_info;
        unsigned int bs_size;
        unsigned int *input_yuyv;
        enum v4l2_memory src_buf_type;
        enum v4l2_memory dst_buf_type;
};
#define JPGE_TILE_MODE  0
#define JPGE_NV12_MODE  8
#define JPGE_NV21_MODE  12
/*************************************************************************************
 Put the Vector Vaule in JPEG Encoder Software Struct
*************************************************************************************/
static int JPEGE_SW_Structa_Pad(JPEGE_SwInfo *swinfo, YUYV_INFO *yuyv_info)
{
        int width, height;

        width = yuyv_info->width;
        height = yuyv_info->height;

        swinfo->YBuf              = yuyv_info->buf;
        swinfo->BitStreamBuf      = yuyv_info->BitStreamBuf;
        swinfo->des_va            = yuyv_info->des_va;//VDMA Chain first address.
        swinfo->des_pa            = yuyv_info->des_pa;
        swinfo->InDaMd            = JPGE_NV21_MODE;
        swinfo->nmcu              = width * height / 256 - 1;
        swinfo->nrsm              = 0;
        swinfo->rsm               = 0;
        swinfo->ncol              = 2; /* yuv(3) - 1 */
        swinfo->ql_sel            = yuyv_info->ql_sel;
        swinfo->huffenc_sel       = 0;
        swinfo->width             = width;
        swinfo->height            = height;
        return 0;
}

/*************************************************************************************
 Padding JPEG Encoder Software and Hardware used Struct
*************************************************************************************/
static int JPEGE_Struct_Pad(JPEGE_SliceInfo *s, JPEGE_SwInfo *swinfo)
{
        int width, height;

        width = swinfo->width;
        height = swinfo->height;

        /* hardware struct padding */
        s->des_va      = swinfo->des_va;
        s->des_pa      = swinfo->des_pa;
        s->bsa         = (uint8_t *)get_phy_addr((uint32_t)swinfo->BitStreamBuf);
        s->raw[0]      = get_phy_addr((uint32_t)swinfo->YBuf);
        s->nmcu        = swinfo->nmcu;
        s->nrsm        = swinfo->nrsm;
        s->rsm         = swinfo->rsm;
        s->ncol        = swinfo->ncol;
        s->ql_sel      = swinfo->ql_sel;
        s->huffenc_sel = swinfo->huffenc_sel;
        s->mb_width    = width/16;
        s->mb_height   = height/16;
        s->stride[0]   = width * 2;
        s->stride[1]   = width;
        s->raw_format  = swinfo->InDaMd; /* EFE working, RAW data is NV21 */

        //tabcheck((uint32_t)s->bsa, (uint32_t)bs_addr, ecs_LEN + 256);
        return 0;
}
static void JPEGE_SliceInit(JPEGE_SliceInfo *s)
{
        unsigned int i;
        volatile unsigned int *chn = (volatile unsigned int *)s->des_va;

        GEN_VDMA_ACFG(chn, TCSM_FLUSH, 0, 0x0);

        /*printf("open clock!\n");*/
        /* Open clock configuration */
        GEN_VDMA_ACFG(chn, REG_JPGC_GLBI, 0, OPEN_CLOCK);

        /*printf("write huffman table!\n");*/
        /**
         * Huffman Encode Table configuration
         */
        for(i = 0; i < HUFFENC_LEN; i++)
                GEN_VDMA_ACFG(chn, REG_JPGC_HUFE+i*4, 0, huffenc[s->huffenc_sel][i]);

        /*printf("write quantization table!\n");*/
        /**
         * Quantization Table configuration
         */
        for(i = 0; i < QMEM_LEN; i++)
                GEN_VDMA_ACFG(chn, REG_JPGC_QMEM+i*4, 0, qmem[s->ql_sel][i]);

        /*printf("write regs!\n");*/
        /**
        * REGs configuration
        */
        GEN_VDMA_ACFG(chn, REG_JPGC_STAT, 0,STAT_CLEAN);
        GEN_VDMA_ACFG(chn,REG_JPGC_BSA, 0, (unsigned int)s->bsa);
        GEN_VDMA_ACFG(chn, REG_JPGC_P0A, 0, VRAM_RAWY_BA);

        GEN_VDMA_ACFG(chn,REG_JPGC_NMCU, 0,s->nmcu);
        GEN_VDMA_ACFG(chn,REG_JPGC_NRSM, 0,s->nrsm);

        GEN_VDMA_ACFG(chn,REG_JPGC_P0C, 0,YUV420P0C);
        GEN_VDMA_ACFG(chn,REG_JPGC_P1C, 0,YUV420P1C);
        GEN_VDMA_ACFG(chn,REG_JPGC_P2C, 0,YUV420P2C);

        GEN_VDMA_ACFG(chn, REG_JPGC_GLBI, 0, (YUV420PVH          |
                                              JPGC_NCOL          |
                                              JPGC_SPEC/* MODE */|
                                              JPGC_EFE           |
                                              JPGC_EN));
        GEN_VDMA_ACFG(chn, REG_JPGC_TRIG, 0,                                \
                      JPGC_BS_TRIG | JPGC_PP_TRIG | JPGC_CORE_OPEN);
        /**
         * EFE configuration
         */
        GEN_VDMA_ACFG(chn, REG_EFE_GEOM, 0, (EFE_JPGC_LST_MBY(s->mb_height-1) |
                                             EFE_JPGC_LST_MBX(s->mb_width-1)));

        GEN_VDMA_ACFG(chn, REG_EFE_RAWY_SBA, 0, s->raw[0]);
        GEN_VDMA_ACFG(chn, REG_EFE_RAWU_SBA, 0, s->raw[1]);
        GEN_VDMA_ACFG(chn, REG_EFE_RAWV_SBA, 0, s->raw[2]);
        GEN_VDMA_ACFG(chn, REG_EFE_RAW_STRD, 0, (EFE_RAW_STRDY(s->stride[0]) |
                                                 EFE_RAW_STRDC(s->stride[1])));

        GEN_VDMA_ACFG(chn, REG_EFE_RAW_DBA, 0, VRAM_RAWY_BA);
        GEN_VDMA_ACFG(chn, REG_EFE_CTRL, VDMA_ACFG_TERM, (YUV_YUY2 |
                                                          EFE_PLANE_NV12 |
                                                          EFE_ID_JPEG    |
                                                          EFE_EN         |
                                                          (s->raw_format)|
                                                          EFE_RUN));
}

static int jpge_struct_init(struct jz_jpeg_encode *jz_jpeg)
{
        JPEGE_SliceInfo vpu_s;
        JPEGE_SwInfo swinfo;
        YUYV_INFO *yuyv_info = &jz_jpeg->yuyv_info;
        /* 1. Put the vector value in swinfo */
        /*printf("sw struct padding\n");*/
        JPEGE_SW_Structa_Pad(&swinfo, yuyv_info);

        /*printf("struct padding\n");*/
        /* 2. Padding the JPEG Encoder API struct and swinfo */
        JPEGE_Struct_Pad(&vpu_s, &swinfo);

        /* 3. Write the JPEG Encoder VDMA chain */
        /*printf("write vdma chn\n");*/
        JPEGE_SliceInit(&vpu_s);

        /* 6. refresh cache */
                /*printf("refresh cache\n");*/
        return 0;
}

#ifdef USE_V4L2
#define CLEAR(x)        memset(&(x), 0, sizeof(x))

int xioctl(int fd, int request, void *arg)
{
        int r;
        do {
                r = ioctl(fd, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}
#endif


void* jz_jpeg_encode_init(int width,int height)
{
        struct jz_jpeg_encode *jz_jpeg;
        jz_jpeg = malloc(sizeof(struct jz_jpeg_encode));
        jz_jpeg->vpu = vpu_init();
#ifndef USE_V4L2
        if(!jz_jpeg->vpu){
                fprintf(stderr,"VPU init failure!\n");
        }
        /*jz_jpeg->yuyv_info.des_va = (unsigned int)JZMalloc(128,sizeof(int));*/
        jz_jpeg->yuyv_info.des_va = (unsigned int)JZMalloc(128,100000);

        if(!jz_jpeg->yuyv_info.des_va)
                fprintf(stderr,"Alloc jz_jpeg->yuyv_info.des_va memory failure!\n");
        jz_jpeg->yuyv_info.width = width;
        jz_jpeg->yuyv_info.height = height;
        jz_jpeg->yuyv_info.BitStreamBuf = JZMalloc(128,width * height *2 /10);
        if(!jz_jpeg->yuyv_info.BitStreamBuf)
                fprintf(stderr,"Alloc jz_jpeg->yuyv_info.BitStreamBuf memory failure!\n");
        jz_jpeg->input_yuyv = (unsigned int *)JZMalloc(128,width*height*2);
        if(!jz_jpeg->input_yuyv)
                fprintf(stderr,"Alloc input buffer fail!\n");
        return (void *)jz_jpeg;
#else
        int fd,ret,i;
        struct v4l2_format fmt;
        struct v4l2_requestbuffers reqbuf;
        struct v4l2_buffer buf;

        if(!((struct vpu_struct*)jz_jpeg->vpu)->vpu_fd){
                fprintf(stderr,"VPU init failure!\n");
                return NULL;
        } else
                fd = ((struct vpu_struct *)jz_jpeg->vpu)->vpu_fd;

#ifdef V4L2_SRC_BUF_MMAP
        jz_jpeg->src_buf_type = V4L2_MEMORY_MMAP;
#else
        jz_jpeg->src_buf_type = V4L2_MEMORY_USERPTR;
#endif

#ifdef V4L2_DST_BUF_MMAP
        jz_jpeg->dst_buf_type = V4L2_MEMORY_MMAP;
#else
        jz_jpeg->dst_buf_type = V4L2_MEMORY_USERPTR;
#endif

        jz_jpeg->yuyv_info.width = width;
        jz_jpeg->yuyv_info.height = height;

        //set output buffer parameter
        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
        fmt.fmt.pix.field        = V4L2_FIELD_NONE;

        ret = xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (ret < 0) {
                printf("VIDIOC_S_FMT failed (%d)\n", ret);
                goto err;
        }

        //set input buffer parameter
        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        fmt.fmt.pix.width = width;
        fmt.fmt.pix.height = height;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        fmt.fmt.pix.field        = V4L2_FIELD_NONE;

        ret = xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (ret < 0) {
                printf("VIDIOC_S_FMT failed (%d)\n", ret);
                goto err;
        }

        // request buffers
        CLEAR(reqbuf);
        reqbuf.count = BUFFER_COUNT;
        reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        reqbuf.memory = jz_jpeg->dst_buf_type;
        ret = xioctl(fd , VIDIOC_REQBUFS, &reqbuf);
        if(ret < 0) { printf("VIDIOC_REQBUFS failed (%d)\n", ret);
                goto err;
        }

        CLEAR(reqbuf);
        reqbuf.count = BUFFER_COUNT;
        reqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        reqbuf.memory = jz_jpeg->src_buf_type;
        ret = xioctl(fd , VIDIOC_REQBUFS, &reqbuf);
        if(ret < 0) { printf("VIDIOC_REQBUFS failed (%d)\n", ret);
                goto err;
        }

        // fill struct jpegdstbuf.
        for (i = 0; i < reqbuf.count; i++)
        {
                // take out buffer
                CLEAR(buf);
                buf.index = i;
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = jz_jpeg->dst_buf_type;
                ret = xioctl(fd , VIDIOC_QUERYBUF, &buf);
                if(ret < 0) {
                        printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
                        goto err;
                }
                // mmap buffer
                jpegdstbuf[i].length = buf.length;
                if(jz_jpeg->dst_buf_type == V4L2_MEMORY_MMAP) {
                        jpegdstbuf[i].start = (char *) mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
                        if (jpegdstbuf[i].start == MAP_FAILED) {
                                printf("mmap (%d) failed: %s\n", i, strerror(errno));
                                goto err;
                        }
                } else {
                        jpegdstbuf[i].start = JZMalloc(128, buf.length);
                        if(!jpegdstbuf[i].start) {
                                fprintf(stderr,"Alloc jpegdstbuf failure!\n");
                                goto err;
                        }
                        buf.m.userptr = (unsigned long)jpegdstbuf[i].start;
                }
                // queen buffer
                ret = xioctl(fd , VIDIOC_QBUF, &buf);
                if (ret < 0) {
                        printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
                        return NULL;
                }
                printf("JPEG dst buffer %d: address=0x%x, length=%d\n", i, (unsigned int)jpegdstbuf[i].start, jpegdstbuf[i].length);
        }
        if(jz_jpeg->src_buf_type == V4L2_MEMORY_MMAP) {
                for (i = 0; i < reqbuf.count; i++)
                {
                        // take out buffer
                        CLEAR(buf);
                        buf.index = i;
                        buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
                        buf.memory = V4L2_MEMORY_MMAP;
                        ret = xioctl(fd , VIDIOC_QUERYBUF, &buf);
                        if(ret < 0) {
                                printf("VIDIOC_QUERYBUF (%d) failed (%d)\n", i, ret);
                                goto err;
                        }
                        // mmap buffer
                        jpegsrcbuf[i].length = buf.length;
                        jpegsrcbuf[i].start = (char *) mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
                        if (jpegsrcbuf[i].start == MAP_FAILED) {
                                printf("mmap (%d) failed: %s\n", i, strerror(errno));
                                goto err;
                        }
                        printf("JPEG src buffer %d: address=0x%x, length=%d\n",
                                        i, (unsigned int)jpegsrcbuf[i].start, jpegsrcbuf[i].length);
                }
        }

        return (void *)jz_jpeg;

err:
        close(fd);
        return NULL;
#endif
}
void jz_jpeg_encode_deinit(void *handle){
        struct jz_jpeg_encode *jz_jpeg = (struct jz_jpeg_encode *)handle;
        int i;

        if(jz_jpeg->dst_buf_type == V4L2_MEMORY_MMAP) {
                for(i = 0; i < BUFFER_COUNT; i++)
                        if (-1 == munmap(jpegdstbuf[i].start, jpegdstbuf[i].length))
                                printf("JPEG munmap failed!!!\n");
        }
        vpu_exit(jz_jpeg->vpu);
        free(jz_jpeg);
        jz47_free_alloc_mem();
}

static int copy_image(struct jz_jpeg_encode *jz_jpeg,unsigned char *buf)
{
        int i, j;
        char *ptr;
        YUYV_INFO *yuyv_info = &jz_jpeg->yuyv_info;
        int width = yuyv_info->width;
        int height = yuyv_info->height;
        int ql_sel = yuyv_info->ql_sel;
        unsigned char *bs = yuyv_info->BitStreamBuf;
        int bs_len = jz_jpeg->bs_size;
        int count = 0;
        /*************** SOI *****************/
        buf[count++] = 0xFF;
        buf[count++] = M_SOI;
        /************** DQT --0 **************/
        buf[count++] = 0xFF;
        buf[count++] = M_DQT;
        //Lq=64*2+3
        buf[count++] = 0x0;
        buf[count++] = 0x43;
        //Pq=0 Tq=0
        buf[count++] = 0x00;
        //Qk
        ptr = (char *)&qt[ql_sel][0];
        for(i=0; i<64; i++)
                buf[count++] = *ptr++;

        /************** DQT --1 *************/
        buf[count++] = 0xFF;
        buf[count++] = M_DQT;
        //Lq=64*2+3
        buf[count++] = 0x0;
        buf[count++] = 0x43;
        //Pq=0 Tq=1
        buf[count++] = 0x01;
        //Qk
        for(i=0; i<64; i++)
                buf[count++] = *ptr++;

        /************* SOF ***************/
        buf[count++] = 0xFF;
        buf[count++] = M_SOF0;
        //Lf=17
        buf[count++] = 0x0;
        buf[count++] = 0x11;
        //P=8 --> 8-bit sample
        buf[count++] = 0x8;
        //Y=height
        buf[count++] = (height & 0xFF00)>>8;
        buf[count++] = height & 0xFF;
        //X=width
        buf[count++] = (width & 0xFF00)>>8;
        buf[count++] = width & 0xFF;
        //Nf=3 (number of components)
        buf[count++] = 0x3;
        //C1 --> Y
        buf[count++] = 0x1;
        //H=2,V=2
        buf[count++] = 0x22;
        //Tq --> QT0
        buf[count++] = 0x0;
        //C2 --> U
        buf[count++] = 0x2;
        //H=1,V=1
        buf[count++] = 0x11;
        //Tq --> QT1
        buf[count++] = 0x1;
        //C3 --> V
        buf[count++] = 0x3;
        //H=1,V=1
        buf[count++] = 0x11;
        //Tq --> QT1
        buf[count++] = 0x1;
        /**************** DHT **************/
        for(j=0; j<4; j++){
                buf[count++] = 0xFF;
                buf[count++] = M_DHT;
                //Lh
                int size = ht_size[j];
                buf[count++] = (size & 0xFF00)>>8;
                buf[count++] = size & 0xFF;
                //Tc Th
                buf[count++] = dht_sel[j];
                //Li
                for(i=0; i<16; i++)
                        buf[count++] = ht_len[j][i];
                //Vij
                for(i=0; i<16; i++){
                        int m;
                        for(m=0; m<ht_len[j][i]; m++)
                                buf[count++] = ht_val[j][i][m];
                }
        }
        /************** SOS ****************/
        buf[count++] = 0xFF;
        buf[count++] = M_SOS;
        //Ls = 12
        buf[count++] = 0x0;
        buf[count++] = 0xC;
        //Ns
        buf[count++] = 0x3;
        //Cs1 - Y
        buf[count++] = 0x1;
        //Td, Ta
        buf[count++] = 0x00;
        //Cs2 - U
        buf[count++] = 0x2;
        //Td, Ta
        buf[count++] = 0x11;
        //Cs3 - V
        buf[count++] = 0x3;
        //Td, Ta
        buf[count++] = 0x11;
        //Ss
        buf[count++] = 0x00;
        //Se
        buf[count++] = 0x3f;
        //Ah, Al
        buf[count++] = 0x0;
        memcpy(&buf[count],bs,bs_len);
        buf[count + bs_len] = 255;
        buf[count + bs_len + 1] = 217;
        return count + bs_len + 2;
}

static void convert_yuv420p_yuv422(unsigned char *dest, unsigned char *src, int width, int height)
{
        int i, j;
        unsigned char *PY420_0 = src;
        unsigned char *PY420_1 = src + width;
        unsigned char *PU420 = src + width * height;
        unsigned char *PV420 = src + width * height * 5/4;


        unsigned char *PY422_0 = dest;
        unsigned char *PY422_1 = dest + width * 2;

        for (i = 0; i < height / 2; i++)
        {
                for (j = 0;j < width * 2; j += 4)
                {
                        *PY422_0++ = *PY420_0++;
                        *PY422_1++ = *PY420_1++;
                        *PY422_0++ = *PU420;
                        *PY422_1++ = *PU420++;

                        *PY422_0++ = *PY420_0++;
                        *PY422_1++ = *PY420_1++;
                        *PY422_0++ = *PV420;
                        *PY422_1++ = *PV420++;


                }

                PY420_0 += width;
                PY420_1 += width;
                PY422_0 += width*2;
                PY422_1 += width*2;
        }
}

int jz_put_jpeg_yuv420p_memory(void *handle,unsigned char *dest_image, int image_size,
                                      unsigned char *input_image, int width, int height, int quality)
{
        struct jz_jpeg_encode *jz_jpeg = (struct jz_jpeg_encode *)handle;
        if(!handle)
        {
                fprintf(stderr,"jz_put_jpeg_yuv420p_memor:handle isn't init or init failed!\n");
                return -1;
        }
        convert_yuv420p_yuv422((unsigned char *)jz_jpeg->input_yuyv, input_image, width, height);
//        memcpy(jz_jpeg->input_yuyv,input_image,width * height * 2);
        jz_jpeg->yuyv_info.width = width;
        jz_jpeg->yuyv_info.height = height;
        jz_jpeg->yuyv_info.ql_sel = quality;
        jz_jpeg->yuyv_info.buf = (unsigned char *)jz_jpeg->input_yuyv;
        jz_jpeg->yuyv_info.des_pa = (unsigned int)get_phy_addr(jz_jpeg->yuyv_info.des_va);

/* 2: jpge strcut init */
        jpge_struct_init(jz_jpeg);
        jz_jpeg->bs_size = jz_start_hw_compress(jz_jpeg->vpu,jz_jpeg->yuyv_info.des_va,jz_jpeg->yuyv_info.des_pa);
        return copy_image(jz_jpeg,dest_image);
}


static int genhead(FILE *fp, YUYV_INFO *yuyv_info)
{
        int i, j;
        char *ptr;
        int width = yuyv_info->width;
        int height = yuyv_info->height;
        int ql_sel = yuyv_info->ql_sel;
        /*************** SOI *****************/
        fputc(0xFF, fp);
        fputc(M_SOI, fp);
        /************** DQT --0 **************/
        fputc(0xFF, fp);
        fputc(M_DQT, fp);
        //Lq=64*2+3
        fputc(0x0, fp);
        fputc(0x43, fp);
        //Pq=0 Tq=0
        fputc(0x00, fp);
        //Qk
        ptr = (char *)&qt[ql_sel][0];
        for(i=0; i<64; i++)
                fputc(*ptr++, fp);

        /************** DQT --1 *************/
        fputc(0xFF, fp);
        fputc(M_DQT, fp);
        //Lq=64*2+3
        fputc(0x0, fp);
        fputc(0x43, fp);
        //Pq=0 Tq=1
        fputc(0x01, fp);
        //Qk
        for(i=0; i<64; i++)
                fputc(*ptr++, fp);

        /************* SOF ***************/
        fputc(0xFF, fp);
        fputc(M_SOF0, fp);
        //Lf=17
        fputc(0x0, fp);
        fputc(0x11, fp);
        //P=8 --> 8-bit sample
        fputc(0x8, fp);
        //Y=height
        fputc((height & 0xFF00)>>8, fp);
        fputc(height & 0xFF, fp);
        //X=width
        fputc((width & 0xFF00)>>8, fp);
        fputc(width & 0xFF, fp);
        //Nf=3 (number of components)
        fputc(0x3, fp);
        //C1 --> Y
        fputc(0x1, fp);
        //H=2,V=2
        fputc(0x22, fp);
        //Tq --> QT0
        fputc(0x0, fp);
        //C2 --> U
        fputc(0x2, fp);
        //H=1,V=1
        fputc(0x11, fp);
        //Tq --> QT1
        fputc(0x1, fp);
        //C3 --> V
        fputc(0x3, fp);
        //H=1,V=1
        fputc(0x11, fp);
        //Tq --> QT1
        fputc(0x1, fp);
        /**************** DHT **************/
        for(j=0; j<4; j++){
                fputc(0xFF, fp);
                fputc(M_DHT, fp);
                //Lh
                int size = ht_size[j];
                fputc((size & 0xFF00)>>8, fp);
                fputc(size & 0xFF, fp);
                //Tc Th
                fputc(dht_sel[j], fp);
                //Li
                for(i=0; i<16; i++)
                        fputc(ht_len[j][i], fp);
                //Vij
                for(i=0; i<16; i++){
                        int m;
                        for(m=0; m<ht_len[j][i]; m++)
                                fputc(ht_val[j][i][m], fp);
                }
        }
        /************** SOS ****************/
        fputc(0xFF, fp);
        fputc(M_SOS, fp);
        //Ls = 12
        fputc(0x0, fp);
        fputc(0xC, fp);
        //Ns
        fputc(0x3, fp);
        //Cs1 - Y
        fputc(0x1, fp);
        //Td, Ta
        fputc(0x00, fp);
        //Cs2 - U
        fputc(0x2, fp);
        //Td, Ta
        fputc(0x11, fp);
        //Cs3 - V
        fputc(0x3, fp);
        //Td, Ta
        fputc(0x11, fp);
        //Ss
        fputc(0x00, fp);
        //Se
        fputc(0x3f, fp);
        //Ah, Al
        fputc(0x0, fp);

        return 0;
}

int gen_image(YUYV_INFO *yuyv_info, FILE *fpo, int bs_len)
{
        unsigned char *bs = yuyv_info->BitStreamBuf;

        genhead(fpo, yuyv_info);
        fwrite(bs, 1, bs_len, fpo);
        fputc(255, fpo);
        fputc(217, fpo);
        fflush(fpo);
        return 0;
}

int yuv422_to_jpeg(void *handle,unsigned char *input_image, FILE *fp, int width, int height, int quality)
{
        struct jz_jpeg_encode *jz_jpeg = (struct jz_jpeg_encode *)handle;
        if(!handle)
        {
                fprintf(stderr,"yuv422_to_jpeg:handle isn't init or init failed!\n");
                return -1;
        }
        jz_jpeg->yuyv_info.width = width;
        jz_jpeg->yuyv_info.height = height;
        jz_jpeg->yuyv_info.ql_sel = quality;
        jz_jpeg->yuyv_info.buf = input_image;

#ifndef USE_V4L2
  #ifndef OPERATE_IN_KERNEL
        jz_jpeg->yuyv_info.des_pa = (unsigned int)get_phy_addr(jz_jpeg->yuyv_info.des_va);
/* 2: jpge strcut init */
        jpge_struct_init(jz_jpeg);
        jz_jpeg->bs_size = jz_start_hw_compress(jz_jpeg->vpu,jz_jpeg->yuyv_info.des_va,jz_jpeg->yuyv_info.des_pa);
  #else
        {
                unsigned char *temp_buf = jz_jpeg->yuyv_info.buf;
                unsigned char *temp_bitstreambuf = jz_jpeg->yuyv_info.BitStreamBuf;

                // give the virtual addr to kernel what it can recognize.
                jz_jpeg->yuyv_info.buf = (uint8_t*)(0x80000000 | get_phy_addr((uint32_t)input_image));
                jz_jpeg->yuyv_info.BitStreamBuf = (uint8_t*)(0x80000000 | get_phy_addr((uint32_t)jz_jpeg->yuyv_info.BitStreamBuf));

                if (ioctl(((struct vpu_struct *)jz_jpeg->vpu)->vpu_fd, CMD_WAIT_COMPLETE, &jz_jpeg->yuyv_info) >= 0) {
                        jz_jpeg->bs_size = jz_jpeg->yuyv_info.bslen;
                } else {
                        printf ("vpu convert error!");
                        return -1;
                }

                jz_jpeg->yuyv_info.buf = temp_buf;
                jz_jpeg->yuyv_info.BitStreamBuf = temp_bitstreambuf;
        }
  #endif
#else
        int fd = ((struct vpu_struct *)jz_jpeg->vpu)->vpu_fd;
        int ret;
        struct v4l2_buffer cap_buf;
        struct v4l2_buffer out_buf;
        enum v4l2_buf_type type;

        CLEAR(out_buf);
        out_buf.index = 0;
        out_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        out_buf.memory = jz_jpeg->src_buf_type;
        ret = xioctl(fd , VIDIOC_QUERYBUF, &out_buf);
        if(ret < 0) {
                printf("VIDIOC_QUERYBUF failed (%d)\n", ret);
                return ret;
        }

        jpegsrcbuf[0].length = out_buf.length;
        out_buf.bytesused = out_buf.length;
        if(jz_jpeg->src_buf_type != V4L2_MEMORY_MMAP)
                out_buf.m.userptr = (unsigned long)input_image;

        ret = xioctl(fd, VIDIOC_QBUF, &out_buf);
        if (ret < 0) {
                printf("VIDIOC_QBUF failed (%d)\n", ret);
                return ret;
        }

        // set compression quality.
        {
                struct v4l2_control control;
                CLEAR(control);
                control.id = V4L2_CID_JPEG_COMPRESSION_QUALITY;
                control.value = quality;
                ret = xioctl(fd , VIDIOC_S_CTRL, &control);
        }

        // start compression
        type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        ret = xioctl(fd, VIDIOC_STREAMON, &type);
        if (ret < 0) {
                printf("VIDIOC_STREAMON failed (%d)\n", ret);
                return ret;
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ret = xioctl(fd, VIDIOC_STREAMON, &type);
        if (ret < 0) {
                printf("VIDIOC_STREAMON failed (%d)\n", ret);
                return ret;
        }
        // Get frame
        CLEAR(cap_buf);
        cap_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ret = xioctl(fd, VIDIOC_DQBUF, &cap_buf);
        if (ret < 0) {
                printf("VIDIOC_DQBUF failed (%d)\n", ret);
                return ret;
        }

        CLEAR(out_buf);
        out_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        ret = xioctl(fd, VIDIOC_DQBUF, &out_buf);
        if (ret < 0) {
                printf("VIDIOC_DQBUF failed (%d)\n", ret);
                return ret;
        }

        // stop compression
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ret = xioctl(fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0) {
                printf("VIDIOC_STREAMOFF failed (%d)\n", ret);
                return ret;
        }

        type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        ret = xioctl(fd, VIDIOC_STREAMOFF, &type);
        if (ret < 0) {
                printf("VIDIOC_STREAMOFF failed (%d)\n", ret);
                return ret;
        }

        // Process the frame
        /* printf("length = %d\n", cap_buf.length); */
        /* printf("reserved  %d \n", cap_buf.reserved); */
        jz_jpeg->yuyv_info.BitStreamBuf = jpegdstbuf[cap_buf.index].start;
        jz_jpeg->bs_size = cap_buf.bytesused;
#endif
        gen_image(&jz_jpeg->yuyv_info, fp, jz_jpeg->bs_size);
        return 0;
}

#ifdef USE_V4L2
void* get_src_mmap_buf(void *handle, int index)
{
        struct jz_jpeg_encode *jz_jpeg = (struct jz_jpeg_encode *)handle;
        if(!handle)
        {
                fprintf(stderr,"handle isn't init or init failed!\n");
                return NULL;
        }
        if(jz_jpeg->src_buf_type != V4L2_MEMORY_MMAP)
                return NULL;

        return jpegsrcbuf[index].start;
}
#endif

int jz_put_jpeg_yuv420p_file(void *handle, FILE *fp, unsigned char *input_image, int width,
                                 int height, int quality)
{
        struct jz_jpeg_encode *jz_jpeg = (struct jz_jpeg_encode *)handle;
        //YUYV_INFO *yuyv_info = &jz_jpeg->yuyv_info;
        if (!handle){
                fprintf(stderr, "jz_take_to_jpeg:handle isn't init or init failed!\n");
                return -1;
        }
        convert_yuv420p_yuv422((unsigned char *)jz_jpeg->input_yuyv, input_image, width, height);
        yuv422_to_jpeg(handle, (unsigned char *)jz_jpeg->input_yuyv, fp, width, height, quality);
        return 0;
}

