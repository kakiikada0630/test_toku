#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* �f�[�^��ǂݍ��ށD�߂�l�F�ǂݍ��߂��o�C�g�� */
unsigned int fifo_read(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret = 0;
	
	while ( obj->read != obj->write && ret < size ) {
		buf[ret] = obj->buf[obj->read];
		obj->read = (obj->read + 1) % obj->size;
		ret++;
	}
	
	return ret;
}

/* �f�[�^��ǂݍ��ށD�߂�l�F�ǂݍ��߂��o�C�g�� */
unsigned int fifo_read_block(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int ret         = 0;
	
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
	unsigned int write_index = 0;
	write_index = ( obj->write < obj->read )? obj->write+FIFO_BUFSIZE : obj->write;
	if( BLOCK_SIZE > ( write_index - obj->read ) )
	{
		// �u���b�N���̃f�[�^���s�����Ă���(=��M�̓r��)���ߏ����I��
		return ret;
	}

	for( ret=0 ; ret<BLOCK_SIZE ; ret++ ) 
	{
		buf[ret] = obj->buf[obj->read];
		obj->read = (obj->read + 1) % obj->size;
	}
	
	return ret;
}

/* �i�[����Ă���f�[�^�����擾���� */
unsigned int fifo_length(fifo_t *obj)
{
	return (obj->size + obj->write - obj->read ) % obj->size;
}