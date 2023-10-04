#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/errno.h> 
#include <linux/string.h> 
#include <linux/slab.h> 
#include <linux/uaccess.h> 
#include <linux/fs.h> 
#include "pmod_hygro.h"

#define PMOD_HYGRO_ADDRESS 0x40


static int pmod_hygro_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct pmod_hygro_data *data;
    
    data = devm_kzalloc(&client->dev, sizeof(struct pmod_hygro_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->client = client;
    i2c_set_clientdata(client, data);

    // will add sensor data read/write functionalities

    printk(KERN_INFO "Pmod HYGRO module loaded\n");
    return 0;
}

static int pmod_hygro_remove(struct i2c_client *client) {
    printk(KERN_INFO "Pmod HYGRO module removed\n");
    return 0;
}

static const struct i2c_device_id pmod_hygro_id[] = {
    { "pmod_hygro", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, pmod_hygro_id);

static struct i2c_driver pmod_hygro_driver = {
    .driver = {
        .name = "pmod_hygro",
    },
    .probe = pmod_hygro_probe,
    .remove = pmod_hygro_remove,
    .id_table = pmod_hygro_id,
};

module_i2c_driver(pmod_hygro_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("I2C driver for Pmod HYGRO");

