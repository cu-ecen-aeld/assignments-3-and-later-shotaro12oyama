//
//  pmod_app.c
//
//  Datasheets:
//  https://www.ti.com/product/HDC1080/datasheet
//

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/init.h>
#include <linux/i2c.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>

#include <linux/time.h>

#define pmod_REG_TEMP                    0x00
#define pmod_REG_HUMIDITY                0x01

#define pmod_REG_CONFIG                  0x02
#define pmod_REG_CONFIG_ACQ_MODE         BIT(12)
#define pmod_REG_CONFIG_HEATER_EN        BIT(13)

struct pmod_data {
    struct i2c_client *client;
    struct mutex lock;
    u16 config;

    /* integration time of the sensor */
    int adc_int_us[2];
    /* Ensure natural alignment of timestamp */
    struct {
        __be16 channels[2];
        s64 ts __aligned(8);
    } scan;
};

/* integration time in us */
static const int pmod_int_time[][3] = {
    { 6350, 3650, 0 },    /* IIO_TEMP channel*/
    { 6500, 3850, 2500 },    /* IIO_HUMIDITYRELATIVE channel */
};

/* pmod_REG_CONFIG shift and mask values */
static const struct {
    int shift;
    int mask;
} pmod_resolution_shift[2] = {
    { /* IIO_TEMP channel */
        .shift = 10,
        .mask = 1
    },
    { /* IIO_HUMIDITYRELATIVE channel */
        .shift = 8,
        .mask = 3,
    },
};

static IIO_CONST_ATTR(temp_integration_time_available,
        "0.00365 0.00635");

static IIO_CONST_ATTR(humidityrelative_integration_time_available,
        "0.0025 0.00385 0.0065");

static IIO_CONST_ATTR(out_current_heater_raw_available,
        "0 1");

static struct attribute *pmod_attributes[] = {
    &iio_const_attr_temp_integration_time_available.dev_attr.attr,
    &iio_const_attr_humidityrelative_integration_time_available.dev_attr.attr,
    &iio_const_attr_out_current_heater_raw_available.dev_attr.attr,
    NULL
};

static const struct attribute_group pmod_attribute_group = {
    .attrs = pmod_attributes,
};

static const struct iio_chan_spec pmod_channels[] = {
    {
        .type = IIO_TEMP,
        .address = pmod_REG_TEMP,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
            BIT(IIO_CHAN_INFO_SCALE) |
            BIT(IIO_CHAN_INFO_INT_TIME) |
            BIT(IIO_CHAN_INFO_OFFSET),
        .scan_index = 0,
        .scan_type = {
            .sign = 's',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_BE,
        },
    },
    {
        .type = IIO_HUMIDITYRELATIVE,
        .address = pmod_REG_HUMIDITY,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
            BIT(IIO_CHAN_INFO_SCALE) |
            BIT(IIO_CHAN_INFO_INT_TIME),
        .scan_index = 1,
        .scan_type = {
            .sign = 'u',
            .realbits = 16,
            .storagebits = 16,
            .endianness = IIO_BE,
        },
    },
    {
        .type = IIO_CURRENT,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
        .extend_name = "heater",
        .output = 1,
        .scan_index = -1,
    },
    IIO_CHAN_SOFT_TIMESTAMP(2),
};

static const unsigned long pmod_scan_masks[] = {0x3, 0};

static int pmod_update_config(struct pmod_data *data, int mask, int val)
{
    int tmp = (~mask & data->config) | val;
    int ret;

    ret = i2c_smbus_write_word_swapped(data->client,
                        pmod_REG_CONFIG, tmp);
    if (!ret)
        data->config = tmp;

    return ret;
}

static int pmod_set_it_time(struct pmod_data *data, int chan, int val2)
{
    int shift = pmod_resolution_shift[chan].shift;
    int ret = -EINVAL;
    int i;

    for (i = 0; i < ARRAY_SIZE(pmod_int_time[chan]); i++) {
        if (val2 && val2 == pmod_int_time[chan][i]) {
            ret = pmod_update_config(data,
                pmod_resolution_shift[chan].mask << shift,
                i << shift);
            if (!ret)
                data->adc_int_us[chan] = val2;
            break;
        }
    }

    return ret;
}

static int pmod_get_measurement(struct pmod_data *data,
                   struct iio_chan_spec const *chan)
{
    struct i2c_client *client = data->client;
    int delay = data->adc_int_us[chan->address] + 1*USEC_PER_MSEC;
    int ret;
    __be16 val;

    /* start measurement */
    ret = i2c_smbus_write_byte(client, chan->address);
    if (ret < 0) {
        dev_err(&client->dev, "cannot start measurement");
        return ret;
    }

    /* wait for integration time to pass */
    usleep_range(delay, delay + 1000);

    /* read measurement */
    ret = i2c_master_recv(data->client, (char *)&val, sizeof(val));
    if (ret < 0) {
        dev_err(&client->dev, "cannot read sensor data\n");
        return ret;
    }
    return be16_to_cpu(val);
}

static int pmod_get_heater_status(struct pmod_data *data)
{
    return !!(data->config & pmod_REG_CONFIG_HEATER_EN);
}

static int pmod_read_raw(struct iio_dev *indio_dev,
                struct iio_chan_spec const *chan, int *val,
                int *val2, long mask)
{
    struct pmod_data *data = iio_priv(indio_dev);

    switch (mask) {
    case IIO_CHAN_INFO_RAW: {
        int ret;

        mutex_lock(&data->lock);
        if (chan->type == IIO_CURRENT) {
            *val = pmod_get_heater_status(data);
            ret = IIO_VAL_INT;
        } else {
            ret = iio_device_claim_direct_mode(indio_dev);
            if (ret) {
                mutex_unlock(&data->lock);
                return ret;
            }

            ret = pmod_get_measurement(data, chan);
            iio_device_release_direct_mode(indio_dev);
            if (ret >= 0) {
                *val = ret;
                ret = IIO_VAL_INT;
            }
        }
        mutex_unlock(&data->lock);
        return ret;
    }
    case IIO_CHAN_INFO_INT_TIME:
        *val = 0;
        *val2 = data->adc_int_us[chan->address];
        return IIO_VAL_INT_PLUS_MICRO;
    case IIO_CHAN_INFO_SCALE:
        if (chan->type == IIO_TEMP) {
            *val = 165000;
            *val2 = 65536;
            return IIO_VAL_FRACTIONAL;
        } else {
            *val = 100000;
            *val2 = 65536;
            return IIO_VAL_FRACTIONAL;
        }
        break;
    case IIO_CHAN_INFO_OFFSET:
        *val = -15887;
        *val2 = 515151;
        return IIO_VAL_INT_PLUS_MICRO;
    default:
        return -EINVAL;
    }
}

static int pmod_write_raw(struct iio_dev *indio_dev,
                 struct iio_chan_spec const *chan,
                 int val, int val2, long mask)
{
    struct pmod_data *data = iio_priv(indio_dev);
    int ret = -EINVAL;

    switch (mask) {
    case IIO_CHAN_INFO_INT_TIME:
        if (val != 0)
            return -EINVAL;

        mutex_lock(&data->lock);
        ret = pmod_set_it_time(data, chan->address, val2);
        mutex_unlock(&data->lock);
        return ret;
    case IIO_CHAN_INFO_RAW:
        if (chan->type != IIO_CURRENT || val2 != 0)
            return -EINVAL;

        mutex_lock(&data->lock);
        ret = pmod_update_config(data, pmod_REG_CONFIG_HEATER_EN,
                    val ? pmod_REG_CONFIG_HEATER_EN : 0);
        mutex_unlock(&data->lock);
        return ret;
    default:
        return -EINVAL;
    }
}

static int pmod_buffer_postenable(struct iio_dev *indio_dev)
{
    struct pmod_data *data = iio_priv(indio_dev);
    int ret;

    /* Buffer is enabled. First set ACQ Mode, then attach poll func */
    mutex_lock(&data->lock);
    ret = pmod_update_config(data, pmod_REG_CONFIG_ACQ_MODE,
                    pmod_REG_CONFIG_ACQ_MODE);
    mutex_unlock(&data->lock);

    return ret;
}

static int pmod_buffer_predisable(struct iio_dev *indio_dev)
{
    struct pmod_data *data = iio_priv(indio_dev);
    int ret;

    mutex_lock(&data->lock);
    ret = pmod_update_config(data, pmod_REG_CONFIG_ACQ_MODE, 0);
    mutex_unlock(&data->lock);

    return ret;
}

static const struct iio_buffer_setup_ops hdc_buffer_setup_ops = {
    .postenable  = pmod_buffer_postenable,
    .predisable  = pmod_buffer_predisable,
};

static irqreturn_t pmod_trigger_handler(int irq, void *p)
{
    struct iio_poll_func *pf = p;
    struct iio_dev *indio_dev = pf->indio_dev;
    struct pmod_data *data = iio_priv(indio_dev);
    struct i2c_client *client = data->client;
    int delay = data->adc_int_us[0] + data->adc_int_us[1] + 2*USEC_PER_MSEC;
    int ret;

    /* dual read starts at temp register */
    mutex_lock(&data->lock);
    ret = i2c_smbus_write_byte(client, pmod_REG_TEMP);
    if (ret < 0) {
        dev_err(&client->dev, "cannot start measurement\n");
        goto err;
    }
    usleep_range(delay, delay + 1000);

    ret = i2c_master_recv(client, (u8 *)data->scan.channels, 4);
    if (ret < 0) {
        dev_err(&client->dev, "cannot read sensor data\n");
        goto err;
    }

    iio_push_to_buffers_with_timestamp(indio_dev, &data->scan,
                       iio_get_time_ns(indio_dev));
err:
    mutex_unlock(&data->lock);
    iio_trigger_notify_done(indio_dev->trig);

    return IRQ_HANDLED;
}

static const struct iio_info pmod_info = {
    .read_raw = pmod_read_raw,
    .write_raw = pmod_write_raw,
    .attrs = &pmod_attribute_group,
};

static int pmod_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct iio_dev *indio_dev;
    struct pmod_data *data;
    int ret;

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_WORD_DATA |
                     I2C_FUNC_SMBUS_BYTE | I2C_FUNC_I2C))
        return -EOPNOTSUPP;

    indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
    if (!indio_dev)
        return -ENOMEM;

    data = iio_priv(indio_dev);
    i2c_set_clientdata(client, indio_dev);
    data->client = client;
    mutex_init(&data->lock);

    indio_dev->name = dev_name(&client->dev);
    indio_dev->modes = INDIO_DIRECT_MODE;
    indio_dev->info = &pmod_info;

    indio_dev->channels = pmod_channels;
    indio_dev->num_channels = ARRAY_SIZE(pmod_channels);
    indio_dev->available_scan_masks = pmod_scan_masks;

    /* be sure we are in a known state */
    pmod_set_it_time(data, 0, pmod_int_time[0][0]);
    pmod_set_it_time(data, 1, pmod_int_time[1][0]);
    pmod_update_config(data, pmod_REG_CONFIG_ACQ_MODE, 0);

    ret = devm_iio_triggered_buffer_setup(&client->dev,
                     indio_dev, NULL,
                     pmod_trigger_handler,
                     &hdc_buffer_setup_ops);
    if (ret < 0) {
        dev_err(&client->dev, "iio triggered buffer setup failed\n");
        return ret;
    }

    return devm_iio_device_register(&client->dev, indio_dev);
}

static const struct i2c_device_id pmod_id[] = {
    { "pmod", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pmod_id);

static const struct of_device_id pmod_dt_ids[] = {
    { .compatible = "ti,hdc1080" },
    { }
};
MODULE_DEVICE_TABLE(of, pmod_dt_ids);

static const struct acpi_device_id pmod_acpi_match[] = {
    { "TXNW1010" },
    { }
};
MODULE_DEVICE_TABLE(acpi, pmod_acpi_match);

static struct i2c_driver pmod_driver = {
    .driver = {
        .name    = "pmod",
        .of_match_table = pmod_dt_ids,
        .acpi_match_table = pmod_acpi_match,
    },
    .probe = pmod_probe,
    .id_table = pmod_id,
};
module_i2c_driver(pmod_driver);

MODULE_AUTHOR("Shotaro Oyama");
MODULE_DESCRIPTION("pmod humidity and temperature sensor driver");
MODULE_LICENSE("GPL");

