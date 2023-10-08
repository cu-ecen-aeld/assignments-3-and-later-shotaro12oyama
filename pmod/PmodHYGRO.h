/**
 * @file    PmodHYGRO.h
 * @brief   Functions and data related to the Pmod HYGRO driver implementation
 *
 * @author  Shotaro Oyama
 * @date    2023-10-06
 *
 */

#ifndef PMODHYGRO_H_
#define PMODHYGRO_H_

#define PMOD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef PMOD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "pmod: " fmt, ## args)
#include <linux/types.h>
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#include <stddef.h> 
#include <stdint.h> 
#include <stdbool.h>
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#include <linux/i2c.h>
#include <linux/cdev.h>
#define CALIB_DATA_PT_LEN (24)
#define LONG_SIGNED_INT_NUM (1)
#define MEASUREMENT_LEN (25)

// Only for Temperature and Pressure
enum calib_data_digits
{
    dig_T1 = 0,
    dig_T2,
    dig_T3,
    dig_P1,
    dig_P2,
    dig_P3,
    dig_P4,
    dig_P5,
    dig_P6,
    dig_P7,
    dig_P8,
    dig_P9
};


struct pmod_dev
{
    struct cdev cdev;     /* Char device structure */
    struct i2c_adapter *pmod_i2c_adapter;
    struct i2c_client *pmod_i2c_client;
    uint8_t calib_data[CALIB_DATA_PT_LEN];
};

static int pmod_read_temperature(struct i2c_client *client, int *temperature);
static int pmod_read_humidity(struct i2c_client *client, int *humidity);

#endif /* PMODHYGRO_H_ */
