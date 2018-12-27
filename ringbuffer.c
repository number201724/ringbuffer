#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"
#include "ringbuffer.h"

ringbuffer_block_t *ringbuffer_block_new( size_t nSize )
{
	ringbuffer_block_t *ringbuf_data = NULL;

	if ( nSize < RINGBUFFER_DEFAULT_DATA_SIZE )
	{
		nSize = RINGBUFFER_DEFAULT_DATA_SIZE;
	}

	ringbuf_data = (ringbuffer_block_t *)malloc( sizeof( ringbuffer_block_t ) + nSize );

	ringbuf_data->data = (unsigned char *)&ringbuf_data[1];
	ringbuf_data->maxsize = nSize;
	ringbuf_data->cursize = 0;
	ringbuf_data->noffset = 0;

	return ringbuf_data;
}

void ringbuffer_block_free( ringbuffer_block_t *ringbuf_block )
{
	free( ringbuf_block );
}

void ringbuffer_init( ringbuffer_t *ringbuf )
{
	ringbuf->block_count = 0;
	ringbuf->length = 0;
	InitializeMPListEntry( &ringbuf->block_head );
}

void ringbuffer_destory( ringbuffer_t *ringbuf )
{
	ringbuffer_block_t *ringbuf_data;

	while ( !MPListIsEmpty( &ringbuf->block_head ) )
	{
		ringbuf_data = MPListLastEntry( &ringbuf->block_head, ringbuffer_block_t, Entry );
		MPRemoveEntryList( &ringbuf_data->Entry );
		ringbuf->block_count--;
		ringbuffer_block_free( ringbuf_data );
	}
}

void ringbuffer_write( ringbuffer_t *ringbuf, const unsigned char *buffer, size_t size )
{
	ringbuffer_block_t *ringbuf_block;

	if ( MPListIsEmpty( &ringbuf->block_head ) )
	{
		ringbuf_block = ringbuffer_block_new( size );
		MPListInsertToHead( &ringbuf->block_head, &ringbuf_block->Entry );
		ringbuf->block_count++;
	}
	else
	{
		ringbuf_block = MPListFirstEntry( &ringbuf->block_head, ringbuffer_block_t, Entry );
	}

	//���ringbuffer�ķֿ��㹻������������ ��ֱ�Ӹ������ݽ�ȥ
	if ( ringbuf_block->maxsize > ringbuf_block->cursize + size )
	{
		memcpy( &ringbuf_block->data[ringbuf_block->cursize], buffer, size );
		ringbuf_block->cursize += size;
		ringbuf->length += size;
		return;
	}


	//ringbuffer�ֿ��޷�������������,ѭ��д�벻ͬ�ֿ�
	size_t lastsize = size;

	const unsigned char *input_data = buffer;

	while ( lastsize > 0 )
	{
		if ( ringbuf_block->maxsize - ringbuf_block->cursize == 0 )	//���ringbuf_dataû��ʣ������ ��ִ���½�����
		{
			ringbuf_block = ringbuffer_block_new( lastsize );
			MPListInsertToHead( &ringbuf->block_head, &ringbuf_block->Entry );
			ringbuf->block_count++;

			//׷�����ݵ�����ķֿ��� ������ʣ���С
			size_t copylen = min( ringbuf_block->maxsize - ringbuf_block->cursize, lastsize );
			memcpy( &ringbuf_block->data[ringbuf_block->cursize], input_data, copylen );
			ringbuf_block->cursize += copylen;

			//�ƶ�ָ��λ��
			input_data += copylen;
			lastsize -= copylen;
			ringbuf->length += copylen;
		}
		else
		{
			//׷������
			size_t copylen = min( ringbuf_block->maxsize - ringbuf_block->cursize, lastsize );
			memcpy( &ringbuf_block->data[ringbuf_block->cursize], input_data, copylen );

			ringbuf_block->cursize += copylen;
			input_data += copylen;
			lastsize -= copylen;
			ringbuf->length += copylen;
		}
	}
}

size_t ringbuffer_read( ringbuffer_t *ringbuf, unsigned char *buffer, size_t size )
{
	if ( ringbuf->length == 0 )
		return 0;

	size_t totalread = min( ringbuf->length, size );
	size_t nread = 0;

	ringbuffer_block_t *current_block = MPListLastEntry( &ringbuf->block_head, ringbuffer_block_t, Entry );

	unsigned char *target = buffer;

	while ( nread < totalread )
	{
		size_t readlen = min( current_block->cursize - current_block->noffset, totalread - nread );

		memcpy( target, current_block->data + current_block->noffset, readlen );

		current_block->noffset += readlen;
		nread += readlen;
		target += readlen;
		ringbuf->length -= readlen;

		//BLOCK����ȫ����ȡ����ж�
		if ( current_block->noffset == current_block->cursize )
		{
			if ( current_block->cursize < current_block->maxsize )//��������Ѿ������˾�����ѭ��
			{
				break;
			}

			//BLOCKȫ�������꣬����BLOCK�Ѿ�ȫ����ʹ�����ͷŵ�����ڴ�
			ringbuffer_block_t *ringbuf_temp = current_block;

			current_block = (ringbuffer_block_t *)current_block->Entry.prev;

			MPRemoveEntryList( &ringbuf_temp->Entry );
			ringbuffer_block_free( ringbuf_temp );
			ringbuf->block_count--;

			if ( MPListIsEmpty( &ringbuf->block_head ) )	//����б��ǿյľ�ֱ������ѭ��
				break;
		}
	}

	return nread;
}
