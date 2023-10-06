#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shotaro Oyama");
MODULE_DESCRIPTION("PMOD HYGRO Sensor Driver");

static struct i2c_client *hygro_client;

static int hygro_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    // センサーの初期化コードをここに記述

    return 0;
}

static int hygro_remove(struct i2c_client *client) {
    // センサーの後片付けコードをここに記述

    return 0;
}

static const struct i2c_device_id hygro_id[] = {
    { "hygro", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, hygro_id);

static struct i2c_driver hygro_driver = {
    .driver = {
        .name   = "hygro",
    },
    .probe      = hygro_probe,
    .remove     = hygro_remove,
    .id_table   = hygro_id,
};

static int __init hygro_init(void) {
    printk("test build!");
    return i2c_add_driver(&hygro_driver);
}

static void __exit hygro_exit(void) {
    i2c_del_driver(&hygro_driver);
    printk("Bye bye my module\n");
}

module_init(hygro_init);
module_exit(hygro_exit);


