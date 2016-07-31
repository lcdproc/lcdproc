#ifndef I2C_H
#define I2C_H

struct i2c_data;

typedef struct i2c_data I2CHandle;

#define I2C_DEFAULT_DEVICE "/dev/i2c-0"

extern I2CHandle *i2c_open(const char *device, unsigned int addr);
extern void i2c_close(I2CHandle *h);
extern int i2c_write(I2CHandle *h, void *buf, unsigned int count);

#endif /* I2C_H */
