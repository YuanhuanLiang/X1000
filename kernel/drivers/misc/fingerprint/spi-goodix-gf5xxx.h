#ifndef __SPI_GOODIX_GF5XXX_H
#define __SPI_GOODIX_GF5XXX_H

#define GF_W                          0xF0
#define GF_R                           0xF1
#define GF_WDATA_OFFSET     (0x3)
#define GF_RDATA_OFFSET      (0x5)

struct spidev_data {
    struct goodix_platform_data* pdata;
	dev_t			devt;
	spinlock_t		spi_lock;
	struct spi_device	*spi;
	struct list_head	device_entry;

	/* buffer is NULL unless this device is open (users > 0) */
	struct mutex		buf_lock;
	unsigned		users;

	struct fasync_struct *async;
	int irq;
	int irq_enabled;
	u8* buffer;
};

struct gf_spi_transfer {
    unsigned char cmd;
    unsigned char reserve;
    unsigned short addr;
    unsigned int len;
    unsigned long buf;
};

#define  GF_IOC_MAGIC             'g'
#define  GF_IOC_DISABLE_IRQ _IO(GF_IOC_MAGIC,   0)
#define  GF_IOC_ENABLE_IRQ	 _IO(GF_IOC_MAGIC,   1)
#define  GF_IOC_SETSPEED      _IOW(GF_IOC_MAGIC, 2, unsigned int)
#define  GF_IOC_RESET            _IOW(GF_IOC_MAGIC,3,unsigned int)
#define  GF_IOC_SPI_TRANSFER  _IOWR(GF_IOC_MAGIC, 4, struct gf_spi_transfer)
#define  GF_IOC_MAXNR   5 

#endif //__SPI_GOODIX_GF5XXX_Hw
