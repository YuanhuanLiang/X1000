#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <jpa_api/vpu_common.h>


#ifdef USE_V4L2
#include <linux/videodev2.h>
#include <string.h>
#define JPEG_NAME              "jz-vpu encoder"
extern int xioctl(int fd, int request, void *arg);
#endif

#define REG(addr) *((volatile unsigned int*)(addr))
static unsigned int read_reg(unsigned int vpu_base,unsigned int offset)
{
        unsigned int val;
        val = REG(vpu_base + offset);
        /* printf("read:base_addr + offset %x = %x\n", base_addr + offset, val); */
        return val;
}
static void write_reg(unsigned int vpu_base,unsigned int offset, unsigned int value)
{
        REG(vpu_base + offset) = value;
}
static inline void *reg_mmap(int vpu_fd, unsigned int size, unsigned int offset)
{
        void * vaddr = mmap((void *)0, size, PROT_READ | PROT_WRITE, MAP_SHARED, vpu_fd, offset);
        if (vaddr == MAP_FAILED) {
                printf("Map 0x%08x addr with 0x%x size failed.\n", offset, size);
                return NULL;
        }
        return vaddr;
}
static int vpu_reg_mmap(struct vpu_struct *vpu)
{
        vpu->vpu_base = (unsigned int)reg_mmap(vpu->vpu_fd, VPU_SIZE, VPU_BASE);
        vpu->cpm_base = (unsigned int)reg_mmap(vpu->vpu_fd, CPM_SIZE, CPM_BASE);

        printf("VAE mmap successfully done! vpu_base = 0x%x\n", (unsigned int)vpu->vpu_base);
        printf("VAE mmap successfully done! cpm_base = 0x%x\n", (unsigned int)vpu->cpm_base);
        return 0;
}
static void vpu_reg_unmap(struct vpu_struct *vpu)
{
        munmap((void*)vpu->vpu_base, VPU_SIZE);
        munmap((void*)vpu->cpm_base, CPM_SIZE);
        printf("VAE unmap successfully done!\n");
}

static unsigned int cpm_read_reg(unsigned int base_addr,unsigned int offset)
{
        unsigned int val;
        val = REG(base_addr + offset);
        /* printf("read:base_addr + offset %x = %x\n", base_addr + offset, val); */
        return val;
}
static void cpm_write_reg(unsigned int base_addr,unsigned int offset, unsigned int value)
{
        REG(base_addr + offset) = value;
}

static void vpu_reset(struct vpu_struct *vpu)
{

        unsigned int val;
        /* return ioctl(vpu_fd, CMD_VPU_RESET, 0); */
        val = cpm_read_reg(vpu->cpm_base,REG_CPM_SRBC);
        val |= CPM_VPU_STP;
        cpm_write_reg(vpu->cpm_base,REG_CPM_SRBC, val);

        while(!(cpm_read_reg(vpu->cpm_base,REG_CPM_SRBC) & CPM_VPU_ACK))
                ;
        val = cpm_read_reg(vpu->cpm_base,REG_CPM_SRBC);
        val |= CPM_VPU_SR;
        val &= ~CPM_VPU_STP;
        cpm_write_reg(vpu->cpm_base,REG_CPM_SRBC, val);

        val = cpm_read_reg(vpu->cpm_base,REG_CPM_SRBC);
        val &= ~CPM_VPU_SR;
        val &= ~CPM_VPU_STP;
        cpm_write_reg(vpu->cpm_base,REG_CPM_SRBC, val);

}

static int jz_flush_cache(struct vpu_struct *vpu,unsigned int vaddr, unsigned int size)
{
        unsigned int value[2];
        value[0] = vaddr;
        value[1] = size;
        return ioctl(vpu->vpu_fd, CMD_VPU_CACHE, value);
}
int jz_start_hw_compress(void *handle,unsigned int des_va,unsigned int des_pa)
{
        int bslen;
        int wait_time = 0;
        unsigned int val;

        struct vpu_struct *vpu = (struct vpu_struct *)handle;
        /* VPU reset */
        vpu_reset(vpu);
        jz_flush_cache(vpu,(unsigned int)des_va, 0x100000);
        val = SCH_GLBC_HIAXI;
        write_reg(vpu->vpu_base,REG_SCH_GLBC, val);

        write_reg(vpu->vpu_base,REG_VMDA_TRIG, des_pa | VDMA_ACFG_RUN);
        while(!(read_reg(vpu->vpu_base,REG_JPGC_STAT) & 0x80000000)) {
                wait_time++;
                if (wait_time > 10000000) {
                        fprintf(stderr,"timemout\n");
                        fprintf(stderr,"NMCU: 0x%08x\n", read_reg(vpu->vpu_base,REG_JPGC_STAT) & 0xFFFFFF);
                        fprintf(stderr,"JPGC STATUS: 0x%08x\n", read_reg(vpu->vpu_base,REG_JPGC_STAT));
                        break;
                }
        }
        bslen = read_reg(vpu->vpu_base,REG_JPGC_STAT) & 0xFFFFFF;

        return bslen;
}

/*************************************************************************************
 Test Environment initialize/uninitialize
*************************************************************************************/
void* vpu_init()
{
        struct vpu_struct *vpu = malloc(sizeof(struct vpu_struct));
#ifndef USE_V4L2
        vpu->vpu_fd = open(DEVICES_VPU_NAME, O_RDWR);
        if (vpu->vpu_fd < 0) {
                printf("open %s error.\n", DEVICES_VPU_NAME);
                return NULL;
        }
        vpu_reg_mmap(vpu);    // @FPGA, VPU and other used register address mapping.
#else
        struct v4l2_capability cap;
        int fd, i;
        char dev_name[20];

        // findout the VPU device.
        for (i=0; i<3; i++) {
                sprintf(dev_name, "/dev/video%d", i);
                fd = open(dev_name, O_RDWR);
                if (fd) {
                        xioctl(fd, VIDIOC_QUERYCAP, &cap);
                        if(!strcmp((const char*)cap.driver, JPEG_NAME)) {
                                printf("vpu fd = %d\n", fd);
                                vpu->vpu_fd = fd;
                                break;
                        }else
                                close(fd);
                }
        }
#endif
        return (void*)vpu;
}

void vpu_exit(void *vpu)
{
#ifndef USE_V4L2
        struct vpu_struct *v = (struct vpu_struct *)vpu;
        vpu_reg_unmap(v);
        if(v->vpu_fd)
                close(v->vpu_fd);
        free(v);
#else
        struct vpu_struct *v = (struct vpu_struct *)vpu;
        if(v->vpu_fd)
                close(v->vpu_fd);
        free(v);
#endif
}
