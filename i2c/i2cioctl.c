#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>


#define PMOD_HYGRO_ADDRESS 0x40
#define TEMPERATURE_REG 0xE3
#define HUMIDITY_REG 0xE5

int open_i2c_bus(char *bus_filename) {
    int file;
    if ((file = open(bus_filename, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }
    return file;
}

double read_temperature(int i2c_file) {
    char buf[2];
    buf[0] = TEMPERATURE_REG;
    if (write(i2c_file, buf, 1) != 1) {
        perror("Failed to write to the i2c bus");
        exit(1);
    }
    
    if (read(i2c_file, buf, 2) != 2) {
        perror("Failed to read from the i2c bus");
        exit(1);
    }

    int temp = (buf[0] << 8) + buf[1];
    return -45.0 + (175.0 * temp / 65535.0);
}

double read_humidity(int i2c_file) {
    char buf[2];
    buf[0] = HUMIDITY_REG;
    if (write(i2c_file, buf, 1) != 1) {
        perror("Failed to write to the i2c bus");
        exit(1);
    }
    
    if (read(i2c_file, buf, 2) != 2) {
        perror("Failed to read from the i2c bus");
        exit(1);
    }

    int humidity = (buf[0] << 8) + buf[1];
    return 100.0 * (humidity / 65535.0);
}

int main() {
    int i2c_file = open_i2c_bus("/dev/i2c-1");
    if (ioctl(i2c_file, I2C_SLAVE, PMOD_HYGRO_ADDRESS) < 0) {
        perror("Failed to acquire bus access or talk to slave");
        exit(1);
    }

    double temperature = read_temperature(i2c_file);
    double humidity = read_humidity(i2c_file);

    printf("Temperature: %.2f C\n", temperature);
    printf("Humidity: %.2f %%\n", humidity);

    close(i2c_file);

    return 0;
}

