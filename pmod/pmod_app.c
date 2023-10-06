#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include "PmodHYGRO.h"
#include <linux/delay.h>
#include <linux/kernel.h>

// Pmod HYGROのI2Cアドレス
#define I2C_ADDR 0x40

// レジスタアドレス
#define TMP_REG 0x00
#define HUM_REG 0x01
#define CONFIG_REG 0x02

//int pmod_major =   0; // use dynamic major
//int pmod_minor =   0;

#define I2CDEVICE "/dev/i2c-1"


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Shotaro Oyama");
MODULE_DESCRIPTION("PMOD HYGRO Sensor Driver");

struct pmod_dev pmod_device;
//long signed int tFine;

//static struct i2c_client *pmod_client;

static int pmod_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    // センサーの初期化コードをここに記述
    struct i2c_adapter *adapter = client->adapter;
    struct i2c_msg msgs[2];
    uint8_t buf[2];
    int ret;
    //float temperature, humidity;
    int tempRaw;

    // 温度データの取得
    buf[0] = CONFIG_REG;
    buf[1] = 0x00;

    msgs[0].addr = I2C_ADDR;
    msgs[0].flags = 0;
    msgs[0].buf = buf;
    msgs[0].len = 2;

    msgs[1].addr = I2C_ADDR;
    msgs[1].flags = I2C_M_RD;
    msgs[1].buf = buf;
    msgs[1].len = 2;

    ret = i2c_transfer(adapter, msgs, 2);
    if (ret < 0) {
        printk("Failed to transfer data\n");
        return ret;
    }

    tempRaw = (buf[0] << 8) | buf[1];
    //temperature = ((float)tempRaw / 65536.0) * 165.0 - 40.0;

    // 湿度データの取得（同様に実装）

    // 温度と湿度をカーネルログに出力
    printk("Temperature: %d°C\n", tempRaw);
    //printk("Humidity: %.2f%%\n", humidity);
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
    pr_info("test build!");
    return i2c_add_driver(&pmod_driver);
}

static void __exit pmod_exit(void) {
    
    //dev_t devno;
    pr_info("Finish Module\n");
    // Remove the I2C client from the I2C subsystem
    //i2c_unregister_device(pmod_device.pmod_i2c_client);

    // Delete the driver
    i2c_del_driver(&pmod_driver);

    //devno = MKDEV(pmod_major, pmod_minor);

    //cdev_del(&pmod_device.cdev);

    //unregister_chrdev_region(devno, 1);
}

module_init(pmod_init);
module_exit(pmod_exit);


