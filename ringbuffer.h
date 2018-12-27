#pragma once



#define RINGBUFFER_DEFAULT_DATA_SIZE 3990

typedef struct ringbuffer_block_s
{
	MP_LIST_ENTRY Entry;
	unsigned char *data;
	size_t maxsize;
	size_t cursize;
	size_t noffset;
}ringbuffer_block_t;

//创建一块ringbuffer的BLOCK
ringbuffer_block_t *ringbuffer_block_new( size_t nSize );

//释放一块ringbuffer的BLOCK
void ringbuffer_block_free( ringbuffer_block_t *ringbuf_block );

typedef struct ringbuffer_s
{
	MP_LIST_ENTRY block_head;
	int block_count;
	size_t length;
}ringbuffer_t;

void ringbuffer_init( ringbuffer_t *ringbuf );
void ringbuffer_destory( ringbuffer_t *ringbuf );
void ringbuffer_write( ringbuffer_t *ringbuf, const unsigned char *buffer, size_t size );
size_t ringbuffer_read( ringbuffer_t *ringbuf, unsigned char *buffer, size_t size );