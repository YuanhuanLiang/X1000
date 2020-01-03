/*
 * pca9543 Camera Driver
 *
 * Copyright (C) 2017, Ingenic Semiconductor Inc.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>

#include <linux/i2c/pca9543.h>

struct i2c_client* pca9543_i2c_client = NULL;


static inline struct i2c_client* pca9543_get_client(void)
{
    return pca9543_i2c_client;
}

static inline void pca9543_set_client(struct i2c_client* client)
{
    pca9543_i2c_client = client;
}


__attribute__((unused)) static char pca9543_i2c_read(struct i2c_client* client)
{
    return i2c_smbus_read_byte_data(client, client->addr);
}


static int pca9543_i2c_write(struct i2c_client* client, char val)
{
    return i2c_smbus_write_byte_data(client, client->addr, val);
}


static int pca9543_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    printk("PCA9543: probe\n");

    pca9543_set_client(client);

    pca9543_i2c_write(client,0x01);

    return 0;
}

//static int pca9543_detach_client(struct i2c_client *client)
static int pca9543_remove(struct i2c_client *client)
{
    pca9543_set_client(NULL);

    printk("pca9543_remove: driver removed!\n");

    return 0;
}

int pca9543_suspend(struct i2c_client *client, pm_message_t mesg)
{
    return 0;
}

static int pca9543_resume(struct i2c_client *client)
{
    pca9543_i2c_write(client,0x01);

    return 0;
}

int pca9543_switch_i2c_channel(char channel)
{
    if (channel != 0x00 && channel != 0x01){
        printk("unknow channel!\n");
        return -1;
    }
    pca9543_i2c_write(pca9543_get_client(), channel+1);
    return 0;
}
EXPORT_SYMBOL(pca9543_switch_i2c_channel);


static const struct i2c_device_id pca9543_id[] = {
    { "pca9543", 0},
};


static struct i2c_driver pca9543_i2c_driver = {
    .driver = {
        .name  = "pca9543",
        .owner = THIS_MODULE,
    },
    .id_table = pca9543_id,
    .probe    = pca9543_probe,
    .suspend  = pca9543_suspend,
    .resume   = pca9543_resume,
    .remove   = pca9543_remove,
};

static int __init pca9543_init(void)
{
    return i2c_add_driver(&pca9543_i2c_driver);
}

static void __exit pca9543_exit(void)
{
    i2c_del_driver(&pca9543_i2c_driver);
}

module_init(pca9543_init);
module_exit(pca9543_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PCA9543 I2C-SWITCH simple driver.");
MODULE_AUTHOR("Monk <rongjin.su@ingenic.com>");


