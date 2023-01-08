#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "fifo.h"


/* インスタンス生成 */
fifo_t *fifo_create(void)
{
	fifo_t *obj = (fifo_t *) malloc(sizeof(fifo_t));
	if ( obj == NULL )	return NULL;
	
	obj->read = obj->write = 0;
	obj->size = FIFO_BUFSIZE;
	
	return obj;
}

/* インスタンス消去 */
void fifo_delete(fifo_t *obj)
{
	free(obj);
}

/* データを書き込む. 戻り値：書き込めたバイト数 */
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

/* データを64bit(8Byte)ごとに書き込む. 戻り値：書き込めたバイト数 */
unsigned int fifo_write_64(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int  ret         = 0;
	unsigned int  i           = 0;
	uint64_t      *buf_src    = (uint64_t *)(buf);
	uint64_t      *buf_dst    = (uint64_t *)( &(obj->buf[obj->write    ] ) ); // fifoバッファ先頭
	uint64_t      *buf_dst_lst= (uint64_t *)( &(obj->buf[FIFO_BUFSIZE-8] ) ); // fifoバッファ終端
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

/* データを読み込む．戻り値：読み込めたバイト数 */
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

/* データを読み込む．戻り値：読み込めたバイト数 */
unsigned int fifo_read_block(fifo_t *obj, unsigned char *buf, unsigned int size)
{
	unsigned int   ret         = 0;
	unsigned int   write_index = 0;
	uint64_t  *buf_src    = (uint64_t *)( &(obj->buf[obj->read     ] ) ); // fifoバッファ先頭
	uint64_t  *buf_src_lst= (uint64_t *)( &(obj->buf[FIFO_BUFSIZE-8] ) ); // fifoバッファ終端
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
			// ブロック先頭を検出できなかったためここで処理終了
			return ret;
		}
		
		if( 0== strncmp( &obj->buf[obj->read], BLOCK_START, BLOCK_START_SIZE ) )
		{
			// ブロックの先頭情報取得
			break;
		}
		
		obj->read = (obj->read + 1) % obj->size;
	}

	// バッファ内データ数がブロックサイズ分存在するか確認する。
	// なお、サイクリックバッファのため、必要に応じて、データサイズ
	// 計算前に書き込み完了位置を補正する。

	write_index = ( obj->write < obj->read )? obj->write+FIFO_BUFSIZE : obj->write;
	if( BLOCK_SIZE > ( write_index - obj->read ) )
	{
		// ブロック情報のデータが不足している(=受信の途中)ため処理終了
		return ret;
	}

	// 8Byteずつ一気にコピーする
	for( ret=0 ; ret<l_cpy_size ; ret++ ) 
	{
		*buf_dst = *buf_src;

		//buf_dstをインクリメント
		buf_dst++;
		
		//buf_srcをインクリメント。ただし、リングバッファのため、終端チェックを行う
		buf_src = ( (buf_src+1) > buf_src_lst )? (uint64_t *)(&obj->buf[0]) : buf_src+1;
	}
	
	obj->read = (obj->read + BLOCK_SIZE) % obj->size;
	
	return ret*8;
}

/* 格納されているデータ数を取得する */
unsigned int fifo_length(fifo_t *obj)
{
	return (obj->size + obj->write - obj->read ) % obj->size;
}