#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "fifo.h"


/* �C���X�^���X���� */
fifo_t *fifo_create(void)
{
	fifo_t *obj = (fifo_t *) malloc(sizeof(fifo_t));
	if ( obj == NULL )	return NULL;
	
	obj->read = obj->write = 0;
	obj->size = FIFO_BUFSIZE;
	
	return obj;
}

/* �C���X�^���X���� */
void fifo_delete(fifo_t *obj)
{
	free(obj);
}

/* �f�[�^����������. �߂�l�F�������߂��o�C�g�� */
unsigned int fifo_write(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret = 0;
	unsigned int next = (obj->write + 1) % obj->size;
	
	while ( next != obj->read && ret < size ) {
		obj->buf[obj->write] = buf[ret];
		obj->write = next;
		next = (obj->write + 1) % obj->size;
		ret++;
	}
	
	return ret;
}

/* �f�[�^��64bit(8Byte)���Ƃɏ�������. �߂�l�F�������߂��o�C�g�� */
unsigned int fifo_write_64(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int  ret         = 0;
	unsigned int  i           = 0;
	uint64_t      *buf_src    = (uint64_t *)(buf);
	uint64_t      *buf_dst    = (uint64_t *)( &(obj->buf[obj->write    ] ) ); // fifo�o�b�t�@�擪
	uint64_t      *buf_dst_lst= (uint64_t *)( &(obj->buf[FIFO_BUFSIZE-8] ) ); // fifo�o�b�t�@�I�[
	unsigned int  l_cpy_size  = size / 8U;

	//printf("write : %d, %d \n", obj->write, obj->read);
	
	for( ret=0 ; ret<l_cpy_size ; ret++ )
	{
		*buf_dst = *buf_src;
		
		buf_src++;
		buf_dst = ( buf_dst+1 > buf_dst_lst )? (uint64_t *)(&obj->buf[0]) : buf_dst+1;
	}

	obj->write = (size + obj->write ) % obj->size;
	
	return size;
}

/* �f�[�^��ǂݍ��ށD�߂�l�F�ǂݍ��߂��o�C�g�� */
unsigned int fifo_read(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret = 0;
	
	while ( (obj->read != obj->write) && (ret < size) ) {
		buf[ret] = obj->buf[obj->read];
		obj->read = (obj->read + 1) % obj->size;
		
		if( buf[ret] == '.' )
		{
			ret++;
			break;
		}
		else
		{
			ret++;
		}
	}
	
	return ret;
}

/* �f�[�^��ǂݍ��ށD�߂�l�F�ǂݍ��߂��o�C�g�� */
unsigned int fifo_read_block(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int   ret         = 0;
	unsigned int   write_index = 0;
	uint64_t  *buf_src    = (uint64_t *)( &(obj->buf[obj->read     ] ) ); // fifo�o�b�t�@�擪
	uint64_t  *buf_src_lst= (uint64_t *)( &(obj->buf[FIFO_BUFSIZE-8] ) ); // fifo�o�b�t�@�I�[
	uint64_t  *buf_dst    = (uint64_t *)(buf);
	unsigned int   l_cpy_size  = (int)(BLOCK_SIZE) / 8;
	
	if( size < BLOCK_SIZE )
	{
		return ret;
	}
	while(1)
	{
		while(	(obj->read          != obj->write) &&
				(obj->buf[obj->read]!= 0xE0      ) )
		{
			obj->read = (obj->read + 1) % obj->size;
		}
		
		if(obj->read == obj->write)
		{
			// �u���b�N�擪�����o�ł��Ȃ��������߂����ŏ����I��
			return ret;
		}
		
		if( 0== strncmp( &obj->buf[obj->read], BLOCK_START, BLOCK_START_SIZE ) )
		{
			// �u���b�N�̐擪���擾
			break;
		}
		
		obj->read = (obj->read + 1) % obj->size;
	}

	// �o�b�t�@���f�[�^�����u���b�N�T�C�Y�����݂��邩�m�F����B
	// �Ȃ��A�T�C�N���b�N�o�b�t�@�̂��߁A�K�v�ɉ����āA�f�[�^�T�C�Y
	// �v�Z�O�ɏ������݊����ʒu��␳����B

	write_index = ( obj->write < obj->read )? obj->write+FIFO_BUFSIZE : obj->write;
	if( BLOCK_SIZE > ( write_index - obj->read ) )
	{
		// �u���b�N���̃f�[�^���s�����Ă���(=��M�̓r��)���ߏ����I��
		return ret;
	}

	// 8Byte����C�ɃR�s�[����
	for( ret=0 ; ret<l_cpy_size ; ret++ ) 
	{
		*buf_dst = *buf_src;

		//buf_dst���C���N�������g
		buf_dst++;
		
		//buf_src���C���N�������g�B�������A�����O�o�b�t�@�̂��߁A�I�[�`�F�b�N���s��
		buf_src = ( (buf_src+1) > buf_src_lst )? (uint64_t *)(&obj->buf[0]) : buf_src+1;
	}
	
	obj->read = (obj->read + BLOCK_SIZE) % obj->size;
	
	return ret*8;
}

/* �i�[����Ă���f�[�^�����擾���� */
unsigned int fifo_length(fifo_t *obj)
{
	return (obj->size + obj->write - obj->read ) % obj->size;
}