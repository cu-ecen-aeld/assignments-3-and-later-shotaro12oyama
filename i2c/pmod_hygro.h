#ifndef _PMOD_HYGRO_H_
#define _PMOD_HYGRO_H_

/* Pmod HYGRO I2C Address */
#define PMOD_HYGRO_ADDRESS 0x40

/* Pmod HYGRO Data Structure */
struct pmod_hygro_data {
    struct i2c_client *client;
};

/* Function prototypes (if any) */
// For example: int pmod_hygro_read_temperature(struct i2c_client *client);

#endif /* _PMOD_HYGRO_H_ */

