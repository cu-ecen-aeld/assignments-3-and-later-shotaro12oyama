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


#define I2CDEVICE 1


MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Shotaro Oyama");
MODULE_DESCRIPTION("PMOD HYGRO Sensor Driver");

struct pmod_dev pmod_device;
//long signed int tFine;

//static struct i2c_client *pmod_client;



static const struct i2c_device_id pmod_id[] = {
    { "pmod", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pmod_id);




static int pmod_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    // センサーの初期化コードをここに記述
    struct i2c_adapter *adapter = client->adapter;
    struct i2c_msg msgs[2];
    uint8_t buf[2];
    int ret;
    //float temperature, humidity;
    int tempRaw;
    int temperature, humidity;
    
    printk("Probe is wokring!\n");
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_I2C_BLOCK)) {
	printk(KERN_ERR "%s: needed i2c functionality is not supported\n", __func__);
	return -ENODEV;
    }
    usleep_range(15,50);
    // パワーアップ後、センサが温度と湿度の計測を開始するまで最大15ms待機する
    msleep(15);


    if (pmod_read_temperature(client, &temperature) < 0) {
        printk(KERN_ERR "Failed to read temperature from Pmod\n");
        return -1;
    }

    if (pmod_read_humidity(client, &humidity) < 0) {
        printk(KERN_ERR "Failed to read humidity from Pmod\n");
        return -1;
    }

    printk(KERN_INFO "Pmod Temperature: %d°C, Humidity: %d%%\n", temperature, humidity);


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

// レジスタのポインタを設定する関数
static int pmod_set_register_pointer(struct i2c_client *client, u8 reg) {
    return i2c_smbus_write_byte(client, reg);
}

// HDC1080 からデータを読み取る関数
static int pmod_read(struct i2c_client *client, u8 reg, u8 *buf, size_t len) {
    int ret;
    ret = pmod_set_register_pointer(client, reg);
    if (ret < 0)
        return ret;
    return i2c_smbus_read_i2c_block_data(client, I2C_ADDR, len, buf);
}

// 温度を読み取る関数
static int pmod_read_temperature(struct i2c_client *client, int *temperature) {
    u8 buf[2];
    int ret;
    int raw_temp;
    ret  = pmod_read(client, TMP_REG, buf, 2);
    if (ret < 0)
        return ret;

    // データの変換（HDC1080の仕様に基づいて）
    raw_temp = (buf[0] << 8) | buf[1];
    *temperature = (int)(raw_temp); /* 65536.0) * 165 - 40); // ℃*/
 

    return 0;
}

// 湿度を読み取る関数
static int pmod_read_humidity(struct i2c_client *client, int *humidity) {
    u8 buf[2];
    int ret;
    int raw_humidity;
    ret  = pmod_read(client, HUM_REG, buf, 2);
    if (ret < 0)
        return ret;

    // データの変換（HDC1080の仕様に基づいて）
    raw_humidity = (buf[0] << 8) | buf[1];
    *humidity = (int)(raw_humidity); /* / 65536.0) * 100); // %RH */

    return 0;
}





static int pmod_remove(struct i2c_client *client) {
    // センサーの後片付けコードをここに記述
    dev_info(&client->dev, "pmod sensor removed\n");
    return 0;
}


static struct i2c_driver pmod_driver = {
    .driver = {
        .name   = "pmod",
    },
    .probe      = pmod_probe,
    .remove     = pmod_remove,
    .id_table   = pmod_id,
};


static int __init pmod_init(void) {
    pr_info("test build!\n");
    return i2c_add_driver(&pmod_driver);
}



static void __exit pmod_exit(void) {
    pr_info("Finish Module\n");
}

module_init(pmod_init);
module_exit(pmod_exit);


