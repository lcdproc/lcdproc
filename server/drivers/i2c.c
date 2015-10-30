/** \file server/drivers/i2c.c
 * OS agnostic functions to access i2c devices.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/ioctl.h>

#ifdef HAVE_DEV_IICBUS_IIC_H
#include <dev/iicbus/iic.h>
#else /* HAVE_LINUX_I2C_DEV_H */
#include <linux/i2c-dev.h>
/* I2C_SLAVE is missing in linux/i2c-dev.h from kernel headers of 2.4.x kernels
*/
#ifndef I2C_SLAVE
#define I2C_SLAVE 0x0703  /* ioctl to change slave address */
#endif
#endif

#define I2C_DEFAULT_DEVICE "/dev/i2c-0"

/** data to access an i2c slave */
typedef struct {
	int fd;			/**< file descriptor of the i2c device */
#ifdef HAVE_DEV_IICBUS_IIC_H
	unsigned int slave;	/**< slave address */
#endif
} I2CHandle;


/**
 * setup connection to i2c slave
 * \param device   device file of the i2c bus
 * \param addr     use this slave address
 * \retval NULL    Error
 * \retval	 Pointer to handle of the connection
 */
I2CHandle *i2c_open(const char *device, unsigned int addr)
{
	I2CHandle *h;
#ifdef HAVE_DEV_IICBUS_IIC_H
	struct iiccmd cmd;
	bzero(&cmd, sizeof(cmd));
#endif


	h = malloc(sizeof(*h));
	if(!h)
		return NULL;

	h->fd = open(device, O_RDWR);
	if(h->fd < 0)
		goto free;

#ifdef HAVE_DEV_IICBUS_IIC_H
	cmd.slave = h->slave = addr << 1;
	cmd.last = 0;
	cmd.count = 0;
	if (ioctl(h->fd, I2CRSTCARD, &cmd) < 0)
		goto close;
	if (ioctl(h->fd, I2CSTART, &cmd) < 0)
		goto close;
#else /* HAVE_LINUX_I2C_DEV_H */
	if (ioctl(h->fd, I2C_SLAVE, addr) < 0) {
		goto close;
	}
#endif

	return h;

close:
	close(h->fd);
free:
	free(h);
	return NULL;
}

/**
 * close connection to i2c slave
 */
void i2c_close(I2CHandle *h)
{
#ifdef HAVE_DEV_IICBUS_IIC_H
	ioctl(h->fd, I2CSTOP);
#endif
	close(h->fd);
	free(h);
}

/**
 * write data to an i2c slave
 * \param h	Handle of i2c slave
 * \param buf   Buffer to send to the device
 * \param count Number of bytes to write
 * \retval >=0     Success.
 * \retval <0      Error.
 */
int i2c_write(I2CHandle *h, void *buf, unsigned int count)
{
#ifdef HAVE_DEV_IICBUS_IIC_H
	struct iiccmd cmd;
	bzero(&cmd, sizeof(cmd));
	cmd.slave = h->slave;
	cmd.last = 1;
	cmd.count = count;
	cmd.buf = buf;

	return ioctl(h->fd, I2CWRITE, &cmd);
#else /* HAVE_LINUX_I2C_DEV_H */
	if (write(h->fd, buf, count) != count)
		return -1;
	return 0;
#endif
}
