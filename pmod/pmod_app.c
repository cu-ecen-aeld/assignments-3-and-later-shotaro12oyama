#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include "PmodHYGRO.h"

int pmod_major =   0; // use dynamic major
int pmod_minor =   0;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shotaro Oyama");
MODULE_DESCRIPTION("PMOD HYGRO Sensor Driver");

struct pmod_dev pmod_device;
long signed int tFine;

static struct i2c_client *pmod_client;

static int pmod_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    // センサーの初期化コードをここに記述

    return 0;
}

static int pmod_remove(struct i2c_client *client) {
    // センサーの後片付けコードをここに記述

    return 0;
}

static const struct i2c_device_id pmod_id[] = {
    { "pmod", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pmod_id);

static struct i2c_driver pmod_driver = {
    .driver = {
        .name   = "pmod",
    },
    .probe      = pmod_probe,
    .remove     = pmod_remove,
    .id_table   = pmod_id,
};

static int __init pmod_init(void) {
    printk("test build!");
    return i2c_add_driver(&pmod_driver);
}

static void __exit pmod_exit(void) {
    
    dev_t devno;
    printk("Finish Module\n");
    // Remove the I2C client from the I2C subsystem
    i2c_unregister_device(pmod_device.pmod_i2c_client);

    // Delete the driver
    i2c_del_driver(&pmod_driver);

    devno = MKDEV(pmod_major, pmod_minor);

    cdev_del(&pmod_device.cdev);

    unregister_chrdev_region(devno, 1);
}

module_init(pmod_init);
module_exit(pmod_exit);


