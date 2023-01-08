#ifndef _FIFO_H_
#define _FIFO_H_

// �e�f�[�^�̈�T�C�Y(4�̔{���Ƃ��鎖!!)
#define BLOCK_START_SIZE 8
#define TICK_SIZE        4
#define VER_SIZE         12
#define PWM_SIZE         56
#define SPITX_SIZE       200
#define SPIRX_SIZE       200
#define UART_SIZE        200
#define LIN_SIZE         20
#define CAN_SIZE         16
#define DAC_SIZE         16
#define ADC_SIZE         16
#define SW_SIZE          4

// �e�f�[�^�̈�̐擪�A�h���X
#define BLOCK_TICK       BLOCK_START_SIZE
#define BLOCK_VER        (BLOCK_TICK + TICK_SIZE   )
#define BLOCK_PWM        (BLOCK_VER  + VER_SIZE    )
#define BLOCK_SPITX      (BLOCK_PWM  + PWM_SIZE    )
#define BLOCK_SPIRX      (BLOCK_SPITX+ SPITX_SIZE  )
#define BLOCK_UART       (BLOCK_SPIRX+ SPIRX_SIZE  )
#define BLOCK_LIN        (BLOCK_UART + UART_SIZE   )
#define BLOCK_CAN        (BLOCK_LIN  + LIN_SIZE    )
#define BLOCK_DAC        (BLOCK_CAN  + CAN_SIZE    )
#define BLOCK_ADC        (BLOCK_DAC  + DAC_SIZE    )
#define BLOCK_SW         (BLOCK_ADC  + ADC_SIZE    )
#define BLOCK_SIZE       (BLOCK_SW   + SW_SIZE     )

#define FIFO_BUFSIZE     ((unsigned long)BLOCK_SIZE * 400UL)


#define SW_VBU_STATUS    0x00010000
#define SW_IG1_STATUS    0x00040000
#define SW_TURNS_STATUS  0x00020000
#define SW_HLBKUP_STATUS 0x00080000

#define VERSION "a. base.0010"
								/* 1�o�C�g: ��� a= PC�ド�C�u�����Ab=�f�o�b�O�}�C�R����\�t�g */
								/* 6�o�C�g: �Ώۋ@�햼(���p��"base"�Ƃ���  �@�@�@�@�@�@�@�@�@*/
								/* 4�o�C�g: �o�[�W�����ԍ�(16�i�C���N�������g)�@�@�@�@�@�@�@�@�@�@�@*/


static const unsigned char BLOCK_START[BLOCK_START_SIZE]={0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7};

/* FIFO�f�[�^�\�� */
typedef struct _TAG_FIFO {
	unsigned char buf[FIFO_BUFSIZE];
	unsigned int size;
	unsigned int read;
	unsigned int write;
}fifo_t;

/* �v���g�^�C�v */
fifo_t *fifo_create(void);
void fifo_delete(fifo_t *obj);
unsigned int fifo_write(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_write_64(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_read(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_read_block(fifo_t *obj, unsigned char *buf, unsigned int size);
unsigned int fifo_length(fifo_t *obj);

#endif /* _FIFO_H_ */