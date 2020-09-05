#ifndef _FIFO_H_
#define _FIFO_H_

#define BLOCK_SIZE       270
#define FIFO_BUFSIZE	 BLOCK_SIZE * 200
#define BLOCK_START_SIZE 8
#define BLOCK_TICK       8
#define BLOCK_PWM        12
#define BLOCK_SPI        40
#define BLOCK_UART       140
#define BLOCK_LIN        240
#define BLOCK_DAC        260


static const unsigned char BLOCK_START[BLOCK_START_SIZE]={0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7};

/* FIFOデータ構造 */
typedef struct _TAG_FIFO {
	unsigned char buf[FIFO_BUFSIZE];
	unsigned int size;
	unsigned int read;
	unsigned int write;
}fifo_t;

/* プロトタイプ */
fifo_t *fifo_create(void);
void fifo_delete(fifo_t *obj);
unsigned int fifo_write(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_read(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_read_block(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_length(fifo_t *obj);

#endif /* _FIFO_H_ */