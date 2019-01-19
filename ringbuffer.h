#pragma once

#ifdef __cplusplus
extern "C"{
#endif

#define RINGBUFFER_DEFAULT_DATA_SIZE 3990
#define RB_SUCCESS 0
#define RB_NO_MEMORY -1

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
void ringbuffer_destroy( ringbuffer_t *ringbuf );
void ringbuffer_clear( ringbuffer_t *ringbuf );
int ringbuffer_write( ringbuffer_t *ringbuf, const unsigned char *buffer, size_t size );

//读取数据 返回数据长度 有可能不会读完
size_t ringbuffer_read( ringbuffer_t *ringbuf, unsigned char *buffer, size_t size );

//读取全部数据 缓冲区内的数据小于传入的大小则返回0
size_t ringbuffer_read2( ringbuffer_t *ringbuf, unsigned char *buffer, size_t size );

#ifdef __cplusplus
};
#endif