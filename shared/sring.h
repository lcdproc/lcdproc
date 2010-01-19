#ifndef SRING_H
#define SRING_H

/** Ring buffer data structure */
typedef struct sring_buffer_t {
	char *data;		/**< Dynamically allocated data storage */
	unsigned int size;	/**< The buffer's size */
	unsigned int w;		/**< write pointer */
	unsigned int r;		/**< read pointer */
} sring_buffer;

sring_buffer* sring_create(int iSize);
void sring_destroy(sring_buffer *buf);
void sring_clear(sring_buffer *buf);
int  sring_getMaxWrite(sring_buffer *buf);
int  sring_getMaxRead(sring_buffer *buf);
int  sring_write(sring_buffer *buf, char *src, int src_len);
int  sring_read(sring_buffer *buf, char *dst, int dst_len);
char* sring_read_string(sring_buffer *buf);
void sring_dump(sring_buffer *buf);

#endif
