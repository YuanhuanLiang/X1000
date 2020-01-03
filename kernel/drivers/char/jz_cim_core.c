/*
 * Ingenic JZ4775 camera (CIM) host driver only for zk
 *
 * Copyright (C) 2012, Ingenic Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <asm/dma.h>
#include <linux/gpio.h>
#include <soc/gpio.h>
#include <linux/workqueue.h>


#define CIM_IMG_DEFAULT_WIDTH   640
#define CIM_IMG_DEFAULT_HEIGHT  480
#define CIM_IMG_DEFAULT_BPP     16

#define SENSOR_DEFAULT_FRAME_SIZE	(CIM_IMG_DEFAULT_WIDTH * CIM_IMG_DEFAULT_HEIGHT * (CIM_IMG_DEFAULT_BPP / 8))
#define CIM_CLOCK_DEFAULT		24000000
#define CIM1_CLOCK_DEFAULT		48000000

#define GET_BUF             1
#define SWAP_BUF            1
#define CIM_FRAME_BUF_CNT   1
//#define CIM_FRAME_BUF_CNT	(GET_BUF + SWAP_BUF)

static int debug = 3;
module_param(debug, int, 0644);

#define dprintk(level, fmt, arg...)					\
	do {								\
		if (debug >= level)					\
			printk("jz-camera: " fmt, ## arg);	\
	} while (0)

/*
 * CIM registers
 */
#define CIM_CFG			(0x00)
#define CIM_CTRL		(0x04)
#define CIM_STATE		(0x08)
#define CIM_IID			(0x0c)
#define CIM_DA			(0x20)
#define CIM_FA			(0x24)
#define CIM_FID			(0x28)
#define CIM_CMD			(0x2c)
#define CIM_SIZE		(0x30)
#define CIM_OFFSET		(0x34)
#define CIM_CTRL2		(0x50)
#define CIM_FS			(0x54)
#define CIM_IMR			(0x58)

/*CIM Configuration Register (CIMCFG)*/
#define CIM_CFG_BS0             16
#define CIM_CFG_BS0_2_OBYT0		(0 << CIM_CFG_BS0)
#define CIM_CFG_BS1_2_OBYT0     (1 << CIM_CFG_BS0)
#define CIM_CFG_BS2_2_OBYT0     (2 << CIM_CFG_BS0)
#define CIM_CFG_BS3_2_OBYT0     (3 << CIM_CFG_BS0)


#define CIM_CFG_BS1             18
#define CIM_CFG_BS0_2_OBYT1		(0 << CIM_CFG_BS1)
#define CIM_CFG_BS1_2_OBYT1     (1 << CIM_CFG_BS1)
#define CIM_CFG_BS2_2_OBYT1     (2 << CIM_CFG_BS1)
#define CIM_CFG_BS3_2_OBYT1     (3 << CIM_CFG_BS1)

#define CIM_CFG_BS2             20
#define CIM_CFG_BS0_2_OBYT2		(0 << CIM_CFG_BS2)
#define CIM_CFG_BS1_2_OBYT2     (1 << CIM_CFG_BS2)
#define CIM_CFG_BS2_2_OBYT2     (2 << CIM_CFG_BS2)
#define CIM_CFG_BS3_2_OBYT2     (3 << CIM_CFG_BS2)

#define CIM_CFG_BS3             22
#define CIM_CFG_BS0_2_OBYT3		(0 << CIM_CFG_BS3)
#define CIM_CFG_BS1_2_OBYT3     (1 << CIM_CFG_BS3)
#define CIM_CFG_BS2_2_OBYT3     (2 << CIM_CFG_BS3)
#define CIM_CFG_BS3_2_OBYT3     (3 << CIM_CFG_BS3)

#define CIM_CFG_VSP			(1 << 14) /* VSYNC Polarity:0-rising edge active,1-falling edge active */
#define CIM_CFG_HSP			(1 << 13) /* HSYNC Polarity:0-rising edge active,1-falling edge active */
#define CIM_CFG_PCP			(1 << 12) /* PCLK working edge: 0-rising, 1-falling */
#define CIM_CFG_DMA_BURST_TYPE_BIT	10
#define CIM_CFG_DMA_BURST_TYPE_MASK	(0x3 << CIM_CFG_DMA_BURST_TYPE_BIT)
#define CIM_CFG_DMA_BURST_INCR8     (0 << CIM_CFG_DMA_BURST_TYPE_BIT)
#define CIM_CFG_DMA_BURST_INCR16	(1 << CIM_CFG_DMA_BURST_TYPE_BIT)	/* Suggested */
#define CIM_CFG_DMA_BURST_INCR32	(2 << CIM_CFG_DMA_BURST_TYPE_BIT)	/* Suggested High speed AHB*/
#define CIM_CFG_DMA_BURST_INCR64	(3 << CIM_CFG_DMA_BURST_TYPE_BIT)	/* Suggested High speed AHB*/

#define CIM_CFG_PACK                4
#define CIM_CFG_PACK_VY1UY0         (0 << CIM_CFG_PACK)
#define CIM_CFG_PACK_Y0VY1U         (1 << CIM_CFG_PACK)
#define CIM_CFG_PACK_UY0VY1         (2 << CIM_CFG_PACK)
#define CIM_CFG_PACK_Y1UY0V         (3 << CIM_CFG_PACK)
#define CIM_CFG_PACK_Y0UY1V         (4 << CIM_CFG_PACK)
#define CIM_CFG_PACK_UY1VY0         (5 << CIM_CFG_PACK)
#define CIM_CFG_PACK_Y1VY0U         (6 << CIM_CFG_PACK)
#define CIM_CFG_PACK_VY0UY1         (7 << CIM_CFG_PACK)

#define CIM_CFG_DSM_BIT		0
#define CIM_CFG_DSM_MASK	(0x3 << CIM_CFG_DSM_BIT)
#define CIM_CFG_DSM_CPM	  	(0 << CIM_CFG_DSM_BIT) /* CCIR656 Progressive Mode */
#define CIM_CFG_DSM_CIM	  	(1 << CIM_CFG_DSM_BIT) /* CCIR656 Interlace Mode */
#define CIM_CFG_DSM_GCM	  	(2 << CIM_CFG_DSM_BIT) /* Gated Clock Mode */

/* CIM State Register  (CIM_STATE) */
#define CIM_STATE_DMA_EEOF	(1 << 11) /* DMA Line EEOf irq */
#define CIM_STATE_DMA_STOP	(1 << 10) /* DMA stop irq */
#define CIM_STATE_DMA_SOF	(1 << 8) /* DMA start irq */
#define CIM_STATE_DMA_EOF	(1 << 9) /* DMA end irq */
#define CIM_STATE_SIZE_ERR	(1 << 3) /* Frame size check error */
#define CIM_STATE_RXF_OF	(1 << 2) /* RXFIFO over flow irq */
#define CIM_STATE_RXF_EMPTY	(1 << 1) /* RXFIFO empty irq */
#define CIM_STATE_VDD		(1 << 0) /* CIM disabled irq */

/* CIM DMA Command Register (CIM_CMD) */
#define CIM_CMD_SOFINT		(1 << 31) /* enable DMA start irq */
#define CIM_CMD_EOFINT		(1 << 30) /* enable DMA end irq */
#define CIM_CMD_EEOFINT		(1 << 29) /* enable DMA EEOF irq */
#define CIM_CMD_STOP		(1 << 28) /* enable DMA stop irq */
#define CIM_CMD_OFRCV       (1 << 27) /* Auto recovery enable when there is RXFIFO overflow */
#define CIM_CMD_LEN_MASK	(0xffffff << 0) /* DMA len mask */

/*CIM Control Register (CIMCR)*/
#define CIM_CTRL_FRC_BIT	16
#define CIM_CTRL_FRC_MASK	(0xf << CIM_CTRL_FRC_BIT)
#define CIM_CTRL_FRC_1		(0x0 << CIM_CTRL_FRC_BIT) /* Sample every frame */
#define CIM_CTRL_FRC_10		(10 << CIM_CTRL_FRC_BIT)
#define CIM_CTRL_DMA_SYNC	(1 << 7)	/*when change DA, do frame sync */
#define CIM_CTRL_CIM_RST	(1 << 3)
#define CIM_CTRL_DMA_EN		(1 << 2) /* Enable DMA */
#define CIM_CTRL_RXF_RST	(1 << 1) /* RxFIFO reset */
#define CIM_CTRL_ENA		(1 << 0) /* Enable CIM */

/* cim control2 */
#define CIM_CTRL2_FSC		(1 << 23)	/* enable frame size check */
#define CIM_CTRL2_ARIF		(1 << 22)	/* enable auto-recovery for incomplete frame */
#define CIM_CTRL2_OPG_BIT	4		/* option priority configuration */
#define CIM_CTRL2_OPG_MASK	(0x3 << CIM_CTRL2_OPG_BIT)
#define CIM_CTRL2_OPE		(1 << 2)	/* optional priority mode enable */
#define CIM_CTRL2_APM		(1 << 0)	/* auto priority mode enable*/

/*CIM Interrupt Mask Register (CIMIMR)*/
#define CIM_IMR_STOP		(1<<10)
#define CIM_IMR_EOFM		(1<<9)
#define CIM_IMR_SOFM		(1<<8)
#define CIM_IMR_FSEM		(1<<3)
#define CIM_IMR_RFIFO_OFM	(1<<2)

/* CIM Frame Size Register (CIM_FS) */
#define CIM_FS_FVS_BIT		16	/* vertical size of the frame */
#define CIM_FS_FVS_MASK		(0x1fff << CIM_FS_FVS_BIT)
#define CIM_FS_BPP_BIT		14	/* bytes per pixel */
#define CIM_FS_BPP_MASK		(0x3 << CIM_FS_BPP_BIT)
#define CIM_FS_FHS_BIT		0	/* horizontal size of the frame */
#define CIM_FS_FHS_MASK		(0x1fff << CIM_FS_FHS_BIT)

#define DRIVER_NAME         "jz-cim"

/*
 * ioctl commands
 */
#define IOCTL_SET_IMG_FORMAT     8	//arg type:enum imgformat
#define IOCTL_SET_TIMING_PARAM   9	// arg type: timing_param_t *
#define IOCTL_SET_IMG_PARAM     10	// arg type: img_param_t *
#define IOCTL_GET_FRAME         11
#define IOCTL_GET_FRAME_BLOCK   12

/*
 * Structures
 */
#define MAX_SOC_CAM_NUM         	2
struct camera_sensor_priv_data {
	unsigned int gpio_rst;
	unsigned int gpio_power;
	unsigned int gpio_en;
};

struct jz_camera_pdata {
	unsigned long mclk_10khz;
	unsigned long flags;
	struct camera_sensor_priv_data sensor_pdata[MAX_SOC_CAM_NUM];
};

struct jz4775_cim_dma_desc {
	dma_addr_t next;
	unsigned int id;
	unsigned int buf;
	unsigned int cmd;
} __attribute__ ((aligned (32)));

/* timing parameters */
typedef struct
{
	unsigned long mclk_freq;
	unsigned char pclk_active_level;  //0 for rising edge, 1 for falling edge
	unsigned char hsync_active_level;
	unsigned char vsync_active_level;
} timing_param_t;

/* image parameters */
typedef struct
{
	unsigned int width;      /* width */
	unsigned int height;     /* height */
	unsigned int bpp;        /* bits per pixel: 8/16/32 */
} img_param_t;

/* image format */
typedef enum {
	NO_FORMAT = 0,
	YUV422_SEP,
	YUV422_PACK,
	YUV420_SEP,
	RGB565,
	RGB888,
	RAW
} img_format_t;

struct cim_device {
	int id;		/* CIM0 or CIM1 */
	unsigned int irq;
	struct clk *clk;
	struct clk *mclk;
	struct resource	*res;
	void __iomem *base;

	struct miscdevice misc_dev;
	struct device *dev;

	unsigned char *framebuf_vaddr;
	unsigned char *framebuf_paddr;
	void *desc_vaddr;
	struct jz4775_cim_dma_desc *dma_desc_paddr;

	timing_param_t timing;
	img_param_t img;
	img_format_t img_format;
	unsigned int frame_size;
	int dma_started;
	int read_flag;
	int fresh_buf;

	int fid;
	wait_queue_head_t wait;
	spinlock_t lock;
	atomic_t opened;

	char name[8];
	char clkname[8];
	char cguclkname[16];

	struct jz_camera_pdata *pdata;
};

static void jz4775_cim_dump_reg(struct cim_device *cim)
{
	int id = cim->id;

	printk("=========================================\n");
	printk("REG_CIM_CFG(%d)     = 0x%08x\n", id, readl(cim->base + CIM_CFG));
	printk("REG_CIM_CTRL(%d)    = 0x%08x\n", id, readl(cim->base + CIM_CTRL));
	printk("REG_CIM_CTRL2(%d)   = 0x%08x\n", id, readl(cim->base + CIM_CTRL2));
	printk("REG_CIM_STATE(%d)   = 0x%08x\n", id, readl(cim->base + CIM_STATE));
	printk("REG_CIM_IID(%d)     = 0x%08x\n", id, readl(cim->base + CIM_IID));
	printk("REG_CIM_DA(%d)      = 0x%08x\n", id, readl(cim->base + CIM_DA));
	printk("REG_CIM_FA(%d)      = 0x%08x\n", id, readl(cim->base + CIM_FA));
	printk("REG_CIM_FID(%d)     = 0x%08x\n", id, readl(cim->base + CIM_FID));
	printk("REG_CIM_CMD(%d)     = 0x%08x\n", id, readl(cim->base + CIM_CMD));
	printk("REG_CIM_SIZE(%d)    = 0x%08x\n", id, readl(cim->base + CIM_SIZE));
	printk("REG_CIM_OFFSET(%d)  = 0x%08x\n", id, readl(cim->base + CIM_OFFSET));
	printk("REG_CIM_FS(%d)      = 0x%08x\n", id, readl(cim->base + CIM_FS));
	printk("REG_CIM_IMR(%d)     = 0x%08x\n", id, readl(cim->base + CIM_IMR));
	printk("=========================================\n");
}

static void jz4775_cim_enable_mclk(struct cim_device *cim)
{
	int ret = -1;
	if (cim->mclk) {
		ret = clk_set_rate(cim->mclk, cim->timing.mclk_freq);
		ret = clk_enable(cim->mclk);
	}
	if (ret)
		dprintk(3, "enable mclock failed!\n");
	msleep(10);
}
static void jz4775_cim_disable_mclk(struct cim_device *cim)
{
	if (cim->mclk)
		clk_disable(cim->mclk);
}
static void jz4775_cim_enable_clk(struct cim_device *cim)
{
	int ret = -1;
	dprintk(7, "Activate device\n");

	if (cim->clk)
		ret = clk_enable(cim->clk);

	if (ret)
		dprintk(3, "enable clock failed!\n");
	msleep(10);
}

static void jz4775_cim_disable_clk(struct cim_device *cim)
{
	if (cim->clk)
		clk_disable(cim->clk);
}

static void jz4775_cim_activate(struct cim_device *cim)
{
	unsigned long temp = 0;

	jz4775_cim_enable_clk(cim);
	jz4775_cim_enable_mclk(cim);

	/* enable end of frame interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp &= (~CIM_IMR_EOFM);
	writel(temp, cim->base + CIM_IMR);

	/* enable error of frame interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp &= (~CIM_IMR_FSEM);
	writel(temp, cim->base + CIM_IMR);

	/* enable rx overflow interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp &= (~CIM_IMR_RFIFO_OFM);
	writel(temp, cim->base + CIM_IMR);
}

static void jz4775_cim_deactivate(struct cim_device *cim)
{
	unsigned long temp = 0;

	/* disable end of frame interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp |= CIM_IMR_EOFM;
	writel(temp, cim->base + CIM_IMR);

	/* enable error of frame interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp |= CIM_IMR_FSEM;
	writel(temp, cim->base + CIM_IMR);

	/* disable rx overflow interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp |= CIM_IMR_RFIFO_OFM;
	writel(temp, cim->base + CIM_IMR);

	/* disable dma */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_DMA_EN;
	writel(temp, cim->base + CIM_CTRL);

	/* clear rx fifo */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	/* disable cim */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_ENA;
	writel(temp, cim->base + CIM_CTRL);

	/* clear state */
	writel(0, cim->base + CIM_STATE);

	jz4775_cim_disable_mclk(cim);
	jz4775_cim_disable_clk(cim);

	dprintk(7, "Deactivate device\n");
}

static void jz4775_cim_reset(struct cim_device *cim)
{
	unsigned long temp = 0;

	/* disable cim */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_ENA;
	writel(temp, cim->base + CIM_CTRL);

	/* disable dma */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_DMA_EN;
	writel(temp, cim->base + CIM_CTRL);

	/* clear rx fifo */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	/*clear state*/
	writel(0, cim->base + CIM_STATE);

	temp = (unsigned int)(cim->dma_desc_paddr);
	writel(temp, cim->base + CIM_DA);
}

#if (CIM_FRAME_BUF_CNT == 1)
static void cim_stop_dma(struct cim_device *cim)
{
	unsigned long temp = 0;

	/* disable end of frame interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp |= CIM_IMR_EOFM;
	writel(temp, cim->base + CIM_IMR);

	/* disable dma */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_DMA_EN;
	writel(temp, cim->base + CIM_CTRL);

	/* clear rx fifo */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	/* disable cim */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_ENA;
	writel(temp, cim->base + CIM_CTRL);

	cim->dma_started = 0;
}
#endif

static void cim_deal_irqerr(struct cim_device *cim)
{
	unsigned long temp = 0;

	/* disable cim */
	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_ENA;
	writel(temp, cim->base + CIM_CTRL);

	/* clear rx fifo */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	/* clear status register */
	writel(0, cim->base + CIM_STATE);

	/* enable cim */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_ENA;
	writel(temp, cim->base + CIM_CTRL);
}

static inline unsigned long cim_get_and_check_iid(struct cim_device *cim)
{
	unsigned long fid;
	unsigned long fid2;
	int loop = 4;

	do {
		fid = readl(cim->base + CIM_IID);
		fid2 = readl(cim->base + CIM_IID);
		if(fid == fid2 && fid >= 0 && fid < CIM_FRAME_BUF_CNT)
			break;
	} while (--loop);

	if (fid < 0 || fid >= CIM_FRAME_BUF_CNT)
		fid = 0;

	return fid;
}

static irqreturn_t jz4775_cim_irq_handler(int irq, void *data)
{
    int fid;
	unsigned long flags;
	unsigned int status = 0;
	struct cim_device *cim = (struct cim_device *)data;

	/* read interrupt status register */
	status = readl(cim->base + CIM_STATE);

	if (status & CIM_STATE_RXF_OF) {
		writel(status & ~(CIM_STATE_RXF_OF), cim->base + CIM_STATE);
		cim_deal_irqerr(cim);
		printk("ERR: Rx FIFO Overflow interrupt!\n");
	}

	if (status & CIM_STATE_SIZE_ERR) {
		writel(status & ~(CIM_STATE_SIZE_ERR), cim->base + CIM_STATE);
		cim_deal_irqerr(cim);
		printk("ERR: Frame size error interrupt!\n");
	}

	if (status & CIM_STATE_DMA_EOF) {
		writel(status & ~(CIM_STATE_DMA_EOF), cim->base + CIM_STATE);

		spin_lock_irqsave(&cim->lock, flags);
		fid = cim_get_and_check_iid(cim);
		fid--;
		if (fid < 0)
			fid = CIM_FRAME_BUF_CNT - 1;
		cim->fid = fid;

		cim->read_flag = 1;
#if (CIM_FRAME_BUF_CNT == 1)
		cim_stop_dma(cim);
#endif
		spin_unlock_irqrestore(&cim->lock, flags);
		if (waitqueue_active(&cim->wait))
			wake_up(&cim->wait);
	}

	/* clear status register */
	writel(0, cim->base + CIM_STATE);

	return IRQ_HANDLED;
}

static void jz4775_cim_set_timing(struct cim_device *cim)
{
	unsigned long cfg_reg = 0;
	timing_param_t *timing = &cim->timing;

	if (cim->id == 0) {
		if (timing->mclk_freq != CIM_CLOCK_DEFAULT) {
			jz4775_cim_disable_mclk(cim);
			jz4775_cim_disable_clk(cim);
			jz4775_cim_enable_clk(cim);
			jz4775_cim_enable_mclk(cim);
		} else if (timing->mclk_freq == 0) {
			jz4775_cim_disable_mclk(cim);
			jz4775_cim_disable_clk(cim);
		}
	}

	if (cim->id == 1) {
		if (timing->mclk_freq != CIM1_CLOCK_DEFAULT) {
			jz4775_cim_disable_mclk(cim);
			jz4775_cim_disable_clk(cim);
			jz4775_cim_enable_clk(cim);
			jz4775_cim_enable_mclk(cim);
		} else if (timing->mclk_freq == 0) {
			jz4775_cim_disable_mclk(cim);
			jz4775_cim_disable_clk(cim);
		}
	}

	cfg_reg = readl(cim->base + CIM_CFG);

	cfg_reg = (timing->pclk_active_level) ? cfg_reg & (~CIM_CFG_PCP) : cfg_reg | CIM_CFG_PCP;
	printk("jz-camera: pclk active %s\n", (timing->pclk_active_level) ? "rising" : "falling");

	cfg_reg = (timing->hsync_active_level) ? cfg_reg & (~CIM_CFG_HSP) : cfg_reg | CIM_CFG_HSP;
	printk("jz-camera: hsync active %s\n", (timing->hsync_active_level) ? "rising" : "falling");

	cfg_reg = (timing->vsync_active_level) ? cfg_reg & (~CIM_CFG_VSP) : cfg_reg | CIM_CFG_VSP;
	printk("jz-camera: vsync active %s\n", (timing->vsync_active_level) ? "rising" : "falling");

	cfg_reg |= CIM_CFG_BS1_2_OBYT1 | CIM_CFG_BS2_2_OBYT2 | CIM_CFG_BS3_2_OBYT3;
	writel(cfg_reg, cim->base + CIM_CFG);
}

static void jz4775_cim_set_img(struct cim_device *cim, unsigned int width,
					unsigned int height, unsigned int bpp)
{
	cim->img.width	= width;
	cim->img.height	= height;
	cim->img.bpp	= bpp;
	cim->frame_size	= cim->img.width * cim->img.height * (cim->img.bpp / 8);
}

#define KSEG_MSK	0xE0000000
#define PHYS(addr) 	((unsigned int)(addr)  & ~KSEG_MSK)
static int jz4775_cim_fb_alloc(struct cim_device *cim)
{
	cim->framebuf_paddr = NULL;

	/*cim->framebuf_vaddr = dma_alloc_coherent(cim->dev,
					cim->frame_size * CIM_FRAME_BUF_CNT + 128,
					(dma_addr_t *)&cim->framebuf_paddr, GFP_KERNEL); */

	cim->framebuf_vaddr = kzalloc(cim->frame_size * CIM_FRAME_BUF_CNT + 128, GFP_KERNEL);
	cim->framebuf_paddr = (unsigned char *)PHYS(cim->framebuf_vaddr);

	if (!cim->framebuf_paddr)
		return -ENOMEM;

	//framebuf paddr aligned to 32-word boundary, bit[6:0] = 0
	if ((unsigned int)cim->framebuf_paddr & 0x3f) {//not aligned
		cim->framebuf_paddr += ((unsigned int)cim->framebuf_paddr / 128 + 1) * 128 - (unsigned int)cim->framebuf_paddr;
		cim->framebuf_vaddr += ((unsigned int)cim->framebuf_vaddr / 128 + 1) * 128 - (unsigned int)cim->framebuf_vaddr;
	}

	return 0;
}

static void jz4775_cim_fb_free(struct cim_device *cim)
{
	/*dma_free_coherent(cim->dev, cim->frame_size * CIM_FRAME_BUF_CNT + 128,
				cim->framebuf_vaddr, GFP_KERNEL); */

	kzfree(cim->framebuf_vaddr);
	cim->framebuf_vaddr = NULL;
	cim->framebuf_paddr = NULL;
}

static int jz4775_cim_desc_set(struct cim_device *cim)
{
	int i;
	unsigned long temp = 0;
	struct jz4775_cim_dma_desc *dma_desc
		= (struct jz4775_cim_dma_desc *)cim->desc_vaddr;

	if (cim->img_format == NO_FORMAT) {
		printk("WARNING: cim%d img format not set, use default YUV422_PACK format\n", cim->id);
		cim->img_format = YUV422_PACK;
	}

	memset(dma_desc, 0 , sizeof(struct jz4775_cim_dma_desc) * CIM_FRAME_BUF_CNT);

	for (i = 0; i < CIM_FRAME_BUF_CNT; i++) {
		dma_desc[i].id = i;

		if (i == CIM_FRAME_BUF_CNT - 1)
			dma_desc[i].next = (dma_addr_t)(cim->dma_desc_paddr);
		else
			dma_desc[i].next = (dma_addr_t)(&cim->dma_desc_paddr[i + 1]);

		dma_desc[i].buf = (dma_addr_t)(cim->framebuf_paddr + cim->frame_size * i);
		dma_desc[i].cmd |= cim->frame_size >> 2;
		dma_desc[i].cmd |= CIM_CMD_EOFINT;
	}

	temp = (unsigned int)(cim->dma_desc_paddr);
	writel(temp, cim->base + CIM_DA);

	return 0;
}

static int jz4775_cim_desc_alloc(struct cim_device *cim)
{
	cim->desc_vaddr = dma_alloc_coherent(cim->dev,
					sizeof(struct jz4775_cim_dma_desc) * CIM_FRAME_BUF_CNT,
					(dma_addr_t *)&cim->dma_desc_paddr, GFP_KERNEL);
	if (!cim->dma_desc_paddr)
		return -ENOMEM;

	return 0;
}

static void jz4775_cim_desc_free(struct cim_device *cim)
{
	dma_free_coherent(cim->dev,
				sizeof(struct jz4775_cim_dma_desc) * CIM_FRAME_BUF_CNT,
				cim->desc_vaddr, GFP_KERNEL);
}

static void jz4775_cim_reg_default_init(struct cim_device *cim)
{
	unsigned long cfg_reg = 0;
	unsigned long ctrl_reg = 0;
	unsigned long ctrl2_reg = 0;
	unsigned long fs_reg = 0;
	unsigned long da = 0;

	cfg_reg |= CIM_CFG_PCP;
	cfg_reg &= ~CIM_CFG_VSP;
	cfg_reg &= ~CIM_CFG_HSP;

	cfg_reg |= CIM_CFG_DMA_BURST_INCR64 | CIM_CFG_DSM_GCM | CIM_CFG_PACK_Y0UY1V;
	cfg_reg |= CIM_CFG_BS1_2_OBYT1 | CIM_CFG_BS2_2_OBYT2 | CIM_CFG_BS3_2_OBYT3;

	ctrl_reg |= CIM_CTRL_DMA_SYNC | CIM_CTRL_FRC_1;

	ctrl2_reg |= CIM_CTRL2_APM | CIM_CTRL2_OPE |
			(1 << CIM_CTRL2_OPG_BIT) | CIM_CTRL2_FSC
			| CIM_CTRL2_ARIF;

	fs_reg = (cim->img.width - 1) << CIM_FS_FHS_BIT |
		(cim->img.height - 1) << CIM_FS_FVS_BIT |
		(cim->img.bpp / 8 - 1) << CIM_FS_BPP_BIT;

	da = (unsigned int)(cim->dma_desc_paddr);
	writel(da, cim->base + CIM_DA);
	writel(cfg_reg, cim->base + CIM_CFG);
	writel(ctrl_reg, cim->base + CIM_CTRL);
	writel(ctrl2_reg, cim->base + CIM_CTRL2);
	writel(fs_reg, cim->base + CIM_FS);
}

static int jz4775_cim_lowlevel_init(struct cim_device *cim)
{
	int err;

	err = jz4775_cim_fb_alloc(cim);
	if (err)
		goto exit;

	err = jz4775_cim_desc_alloc(cim);
	if (err)
		goto exit_free_fb;

	return 0;

exit_free_fb:
	jz4775_cim_fb_free(cim);
exit:
	return err;
}

static void jz4775_cim_lowlevel_deinit(struct cim_device *cim)
{
	jz4775_cim_fb_free(cim);
	jz4775_cim_desc_free(cim);
}

static void jz4775_cim_start_dma(struct cim_device *cim)
{
	unsigned long temp = 0;

	/* clear state */
	writel(0, cim->base + CIM_STATE);

	/* enable dma */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_DMA_EN;
	writel(temp, cim->base + CIM_CTRL);

	/*clear rx fifo */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	temp = readl(cim->base + CIM_CTRL);
	temp &= ~CIM_CTRL_RXF_RST;
	writel(temp, cim->base + CIM_CTRL);

	/* enable cim */
	temp = readl(cim->base + CIM_CTRL);
	temp |= CIM_CTRL_ENA;
	writel(temp, cim->base + CIM_CTRL);

	/* enable end of frame interrupt */
	temp = readl(cim->base + CIM_IMR);
	temp &= ~CIM_IMR_EOFM;
	writel(temp, cim->base + CIM_IMR);
}

static int jz4775_cim_open(struct inode *inode, struct file *file)
{
	struct miscdevice *dev = file->private_data;
	struct cim_device *cim = container_of(dev, struct cim_device, misc_dev);

	if (!(atomic_dec_and_test(&cim->opened))) {
		atomic_inc(&cim->opened);
		return -EBUSY;
	}

	jz4775_cim_activate(cim);
	jz4775_cim_reg_default_init(cim);
	cim->img_format = NO_FORMAT;
	cim->fresh_buf = 0;
	cim->read_flag = 0;
	cim->fid = -1;

	return 0;
}

static int jz4775_cim_release(struct inode *inode, struct file *file)
{
	struct miscdevice *dev = file->private_data;
	struct cim_device *cim = container_of(dev, struct cim_device, misc_dev);

	atomic_inc(&cim->opened);

	jz4775_cim_deactivate(cim);
	cim->dma_started = 0;
	cim->read_flag = 0;
	return 0;
}

static unsigned int jz4775_cim_get_frame(struct cim_device *cim, int is_block_mode)
{
	unsigned int buffer;
	unsigned long flags;
	int ret;
	int fid;
	struct jz4775_cim_dma_desc *dma_desc = (struct jz4775_cim_dma_desc *)cim->desc_vaddr;

	if (cim->dma_started == 0) {
		cim->dma_started = 1;
		cim->read_flag = 0;
		jz4775_cim_start_dma(cim);
	}

	if (is_block_mode) {
		ret = wait_event_timeout(cim->wait, cim->read_flag, 2 * HZ);
		if (ret == 0) {
			printk("cim%d read timeout\n", cim->id);
			jz4775_cim_dump_reg(cim);
			jz4775_cim_reset(cim);
			return -EIO;
		}
	} else {
		if (cim->read_flag == 0)
			return -EIO;
	}

	spin_lock_irqsave(&cim->lock,flags);
	cim->read_flag = 0;
	fid = cim->fid;
	buffer = (unsigned int)(dma_desc[fid].buf);
	spin_unlock_irqrestore(&cim->lock, flags);

	return buffer;
}

static ssize_t jz4775_cim_read(struct file *file, char *buf, size_t size, loff_t *l)
{
	int ret;
	int fid;
	unsigned int cp_size;
	unsigned long flags;
	struct miscdevice *dev = file->private_data;
	struct cim_device *cim = container_of(dev, struct cim_device, misc_dev);

	if (cim->dma_started == 0) {
		cim->dma_started = 1;
		cim->read_flag = 0;
		jz4775_cim_start_dma(cim);
	}

	ret = wait_event_timeout(cim->wait, cim->read_flag, 1 * HZ);
	if (ret == 0) {
		printk("cim%d read timeout\n", cim->id);
		jz4775_cim_dump_reg(cim);
		jz4775_cim_reset(cim);
		return -EIO;
	}

	spin_lock_irqsave(&cim->lock,flags);
	fid = cim->fid;
	cim->read_flag = 0;
	spin_unlock_irqrestore(&cim->lock,flags);

	cp_size = size < cim->frame_size ? size : cim->frame_size;
	ret = copy_to_user(buf, cim->framebuf_vaddr + cim->frame_size * cim->fid, cp_size);
	if (ret) {
		printk("ERROR: copy_to_user failed at %s %d cim id = %d\n", __FUNCTION__, __LINE__, cim->id);
		return -EFAULT;
	}

	return cp_size;
}

static ssize_t jz4775_cim_write(struct file *file, const char *buf, size_t size, loff_t *l)
{
	printk("cim error: write is not implemented\n");
	return -1;
}

static long jz4775_cim_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int err;
	int offset;
	img_param_t img;
	unsigned long fs_reg = 0;
	void __user *argp = (void __user *)arg;
	struct miscdevice *dev = file->private_data;
	struct cim_device *cim = container_of(dev, struct cim_device, misc_dev);

	switch (cmd) {
	case IOCTL_SET_TIMING_PARAM:
		if (copy_from_user(&cim->timing, (void *)arg, sizeof(timing_param_t)))
			return -EFAULT;

		jz4775_cim_set_timing(cim);
		break;

	case IOCTL_SET_IMG_PARAM:
		if (copy_from_user(&img, (void *)arg, sizeof(img_param_t)))
			return -EFAULT;

		jz4775_cim_set_img(cim, img.width, img.height, img.bpp);
		printk("%s set: width = %d, height = %d, bpp = %d\n", __FUNCTION__,
				img.width, img.height, img.bpp);

		if (cim->frame_size > SENSOR_DEFAULT_FRAME_SIZE) {
			jz4775_cim_fb_free(cim);
			err = jz4775_cim_fb_alloc(cim);
			if (err < 0) {
				printk("%s: jz4775_cim_fb_alloc() failed", __FUNCTION__);
				return -EFAULT;
			}
		}

		fs_reg = (cim->img.width - 1) << CIM_FS_FHS_BIT |
				 (cim->img.height - 1) << CIM_FS_FVS_BIT |
				 (cim->img.bpp / 8 - 1) << CIM_FS_BPP_BIT;
		writel(fs_reg, cim->base + CIM_FS);

		err = jz4775_cim_desc_set(cim);
		if (err < 0) {
			printk("%s: jz4775_cim_desc_set() failed\n", __FUNCTION__);
			return -EFAULT;
		}
		break;

	case IOCTL_GET_FRAME:
	case IOCTL_GET_FRAME_BLOCK:
		offset = jz4775_cim_get_frame(cim, cmd == IOCTL_GET_FRAME ? 0 : 1);
		if (offset == (unsigned int)(~0))
			return -1;
		offset -= (unsigned int)cim->framebuf_paddr;
		return copy_to_user(argp, &offset, sizeof(offset)) ? -EFAULT : 0;

	default:
		printk("Not supported command: 0x%x\n", cmd);
		return -EINVAL;
	}

	return 0;
}

static int jz4775_cim_mmap(struct file *file, struct vm_area_struct *vma)
{
	unsigned long start;
	unsigned long off;
	struct miscdevice *dev = file->private_data;
	struct cim_device *cim = container_of(dev, struct cim_device, misc_dev);

	if (cim->framebuf_vaddr == NULL) {
		printk("==>%s L%d: no mem\n", __func__, __LINE__);
		return -ENOMEM;
	}

	off = vma->vm_pgoff << PAGE_SHIFT;
	// frame buffer memory
	start = (unsigned int)cim->framebuf_paddr;
	start &= PAGE_MASK;
	off += start;

	vma->vm_pgoff = off >> PAGE_SHIFT;
	vma->vm_flags |= VM_IO;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);  /* Uncacheable */
#if defined(CONFIG_MIPS32)
	pgprot_val(vma->vm_page_prot) &= ~_CACHE_MASK;
	pgprot_val(vma->vm_page_prot) |= _CACHE_UNCACHED;		  /* Uncacheable */
#endif

	if (remap_pfn_range(vma, vma->vm_start, off >> PAGE_SHIFT,
			vma->vm_end - vma->vm_start,
			vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

static struct file_operations jz4775_cim_fops = {
	.owner		= THIS_MODULE,
	.open		= jz4775_cim_open,
	.release	= jz4775_cim_release,
	.read    	= jz4775_cim_read,
	.write    	= jz4775_cim_write,
	.unlocked_ioctl	= jz4775_cim_ioctl,
	.mmap		= jz4775_cim_mmap,
};

static int jz4775_cim_probe(struct platform_device *pdev)
{
	struct cim_device *cim;
	struct resource *res;
	void __iomem *base;
	unsigned int irq;
	int err = 0;

	if ((pdev->id > 1) || (pdev->id < 0)) {
		printk(KERN_ERR "%s: Invalid ID: %d\n", __FUNCTION__, pdev->id);
		err = -EINVAL;
		goto exit;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!res || (int)irq <= 0) {
		err = -ENODEV;
		goto exit;
	}

	cim = kzalloc(sizeof(*cim), GFP_KERNEL);
	if (!cim) {
		dprintk(3, "Could not allocate cim\n");
		err = -ENOMEM;
		goto exit;
	}

	cim->id = pdev->id;
	cim->irq = irq;
	cim->res = res;
	cim->dev = &pdev->dev;

	sprintf(cim->name, "cim%d", cim->id);
	sprintf(cim->clkname, "cim");
	sprintf(cim->cguclkname, "cgu_cim");

	cim->clk = clk_get(&pdev->dev, cim->clkname);
	if (IS_ERR(cim->clk)) {
		printk(KERN_ERR "%s: get cim%d clk failed\n", __FUNCTION__, cim->id);
		err = PTR_ERR(cim->clk);
		goto exit_kfree;
	}

	cim->mclk = clk_get(&pdev->dev, cim->cguclkname);
	if (IS_ERR(cim->mclk)) {
		printk(KERN_ERR "%s: get cim%d mclk failed\n", __FUNCTION__, cim->id);
		err = PTR_ERR(cim->mclk);
		goto exit_put_clk_cim;
	}

	cim->pdata = (struct jz_camera_pdata *)pdev->dev.platform_data;
	if (!cim->pdata)
		cim->timing.mclk_freq = cim->pdata->mclk_10khz * 10000;
	else {
		if (cim->id == 0)
			cim->timing.mclk_freq = CIM_CLOCK_DEFAULT;
		else if (cim->id == 1)
			cim->timing.mclk_freq = CIM1_CLOCK_DEFAULT;
	}

	/*
	 * Request the regions.
	 */
	if (!request_mem_region(res->start, resource_size(res), DRIVER_NAME)) {
		err = -EBUSY;
		goto exit_put_clk_cgucim;
	}

	base = ioremap(res->start, resource_size(res));
	if (!base) {
		err = -ENOMEM;
		goto exit_release;
	}
	cim->base = base;

	/* request irq */
	err = request_irq(cim->irq, jz4775_cim_irq_handler, IRQF_DISABLED,
						dev_name(&pdev->dev), cim);
	if (err) {
		dprintk(3, "request irq failed!\n");
		goto exit_iounmap;
	}

	init_waitqueue_head(&cim->wait);
	spin_lock_init(&cim->lock);
	atomic_set(&cim->opened, 1);

	cim->misc_dev.minor = MISC_DYNAMIC_MINOR;
	cim->misc_dev.name = cim->name;
	cim->misc_dev.fops = &jz4775_cim_fops;
	err = misc_register(&cim->misc_dev);
	if (err < 0)
		goto exit_free_irq;
	jz4775_cim_set_img(cim, CIM_IMG_DEFAULT_WIDTH, CIM_IMG_DEFAULT_HEIGHT, CIM_IMG_DEFAULT_BPP);
	err = jz4775_cim_lowlevel_init(cim);
	if (err < 0) {
		printk("ERROR: jz4775_cim_lowlevel_init failed %s %d\n", __FUNCTION__, __LINE__);
		return -EFAULT;
	}

	platform_set_drvdata(pdev, cim);
	printk("JZ Camera%d driver loaded, use cim%d.\n", cim->id, cim->id);
	return 0;

exit_free_irq:
	free_irq(cim->irq, cim);
exit_iounmap:
	iounmap(base);
exit_release:
	release_mem_region(res->start, resource_size(res));
exit_put_clk_cgucim:
	clk_put(cim->mclk);
exit_put_clk_cim:
	clk_put(cim->clk);
exit_kfree:
	kfree(cim);
exit:
	return err;
}

static int jz4775_cim_remove(struct platform_device *pdev)
{
	struct cim_device *cim = platform_get_drvdata(pdev);
	struct resource *res;

	jz4775_cim_lowlevel_deinit(cim);

	misc_deregister(&cim->misc_dev);

	free_irq(cim->irq, cim);

	clk_put(cim->clk);
	clk_put(cim->mclk);

	iounmap(cim->base);

	res = cim->res;
	release_mem_region(res->start, resource_size(res));

	kfree(cim);

	dprintk(6, "JZ Camera driver unloaded\n");

	return 0;
}

static int jz4775_cim_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct cim_device *cim = platform_get_drvdata(pdev);

	if (!atomic_read(&cim->opened)) {
		cim->read_flag = 0;
		cim->dma_started = 0;
		jz4775_cim_deactivate(cim);
	}

	return 0;
}

static int jz4775_cim_resume(struct platform_device *pdev)
{
	struct cim_device *cim = platform_get_drvdata(pdev);

	if (!atomic_read(&cim->opened))
		jz4775_cim_activate(cim);

	return 0;
}

static struct platform_driver jz4775_cim_driver = {
	.driver 	= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= jz4775_cim_probe,
	.remove     = jz4775_cim_remove,
	.suspend    = jz4775_cim_suspend,
	.resume     = jz4775_cim_resume,
};

static int __init jz4775_cim_init(void)
{
	return platform_driver_register(&jz4775_cim_driver);
}

static void __exit jz4775_cim_exit(void)
{
	platform_driver_unregister(&jz4775_cim_driver);
}

module_init(jz4775_cim_init);
module_exit(jz4775_cim_exit);

MODULE_DESCRIPTION("JZ Camera Host driver");
MODULE_AUTHOR("DengYequan <yqdeng@ingenic.cn>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRIVER_NAME);
