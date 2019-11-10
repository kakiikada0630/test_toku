#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* データを読み込む．戻り値：読み込めたバイト数 */
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

/* データを読み込む．戻り値：読み込めたバイト数 */
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
	unsigned int write_index = 0;
	write_index = ( obj->write < obj->read )? obj->write+FIFO_BUFSIZE : obj->write;
	if( BLOCK_SIZE > ( write_index - obj->read ) )
	{
		// ブロック情報のデータが不足している(=受信の途中)ため処理終了
		return ret;
	}

	for( ret=0 ; ret<BLOCK_SIZE ; ret++ ) 
	{
		buf[ret] = obj->buf[obj->read];
		obj->read = (obj->read + 1) % obj->size;
	}
	
	return ret;
}

/* 格納されているデータ数を取得する */
unsigned int fifo_length(fifo_t *obj)
{
	return (obj->size + obj->write - obj->read ) % obj->size;
}