#include "membuf.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

namespace top {

void Membuf::Init(membuf_t* buf, unsigned int initial_buffer_size) {
	memset(buf, 0, sizeof(membuf_t));
	buf->data = initial_buffer_size > 0 ? (unsigned char*) malloc(initial_buffer_size) : NULL;
	buf->buffer_size = initial_buffer_size;
	buf->uses_local_buffer = 0;
}

void Membuf::InitLocal(membuf_t* buf, void* local_buffer, unsigned int local_buffer_size) {
	memset(buf, 0, sizeof(membuf_t));
	buf->data = static_cast<unsigned char*>((local_buffer && (local_buffer_size > 0)) ? local_buffer : NULL);
	buf->buffer_size = local_buffer_size;
	buf->uses_local_buffer = 1;
}

void Membuf::InitMoveFrom(membuf_t* buf, membuf_t* other) {
    if(other->uses_local_buffer) {
        Init(buf, 0);
        if(other->size > 0)
            AppendData(buf, other->data, other->size);
    } else {
        *buf = *other;
    }
    memset(other, 0, sizeof(membuf_t)); // other is hollowed now
}

void Membuf::Uninit(membuf_t* buf) {
	if(!buf->uses_local_buffer && buf->data)
		free(buf->data);
    memset(buf, 0, sizeof(membuf_t));
}

void Membuf::Reserve(membuf_t* buf, unsigned int extra_size) {
	if(extra_size > buf->buffer_size - buf->size) {
		//calculate new buffer size
		unsigned int new_buffer_size = buf->buffer_size == 0 ? extra_size : buf->buffer_size << 1;
		unsigned int new_data_size = buf->size + extra_size;
		while(new_buffer_size < new_data_size)
			new_buffer_size <<= 1;

		// malloc/realloc new buffer
		if(buf->uses_local_buffer) {
            void* local = buf->data;
			buf->data = static_cast<unsigned char*>(realloc(NULL, new_buffer_size)); // alloc new buffer
			memcpy(buf->data, local, buf->size); // copy local bufer to new buffer
			buf->uses_local_buffer = 0;
		} else {
			buf->data = static_cast<unsigned char*>(realloc(buf->data, new_buffer_size)); // realloc new buffer
		}
		buf->buffer_size = new_buffer_size;
	}
}

unsigned int Membuf::AppendData(membuf_t* buf, void* data, unsigned int size) {
	assert(data && size > 0);
    Reserve(buf, size);
	memmove(buf->data + buf->size, data, size);
	buf->size += size;
	return (buf->size - size);
}

unsigned int Membuf::AppendZeros(membuf_t* buf, unsigned int size) {
    Reserve(buf, size);
	memset(buf->data + buf->size, 0, size);
	buf->size += size;
	return (buf->size - size);
}

unsigned int Membuf::AppendText(membuf_t* buf, const char* str, unsigned int len) {
	if(str && (len == (unsigned int)(-1)))
		len = strlen(str);
	return AppendData(buf, (void*)str, len);
}

unsigned int Membuf::AppendTextZero(membuf_t* buf, const char* str, unsigned int len) {
	unsigned int offset;
	if(str && (len == (unsigned int)(-1)))
		len = strlen(str);
    Reserve(buf, len + 1);
	offset = AppendData(buf, (void*)str, len);
    AppendZeros(buf, 1);
	return offset;
}

void* Membuf::Detach(membuf_t* buf, unsigned int* psize) {
	void* result = buf->data;
	if(psize) *psize = buf->size;
	if(buf->uses_local_buffer) {
		result = buf->size > 0 ? malloc(buf->size) : NULL;
		memcpy(result, buf->data, buf->size);
	} else {
		buf->buffer_size = 0;
	}
	buf->data = NULL;
	buf->size = 0;
	return result;
}

void swap_data(membuf_t* buf1, membuf_t* buf2) {
	unsigned char* tmp_data = buf1->data;
	buf1->data = buf2->data; buf2->data = tmp_data;
}

 void swap_size(membuf_t* buf1, membuf_t* buf2) {
	unsigned int tmp_size = buf1->size;
	buf1->size = buf2->size; buf2->size = tmp_size;
}

void swap_buffer_size(membuf_t* buf1, membuf_t* buf2) {
	unsigned int tmp_buffer_size = buf1->buffer_size;
	buf1->buffer_size = buf2->buffer_size; buf2->buffer_size = tmp_buffer_size;
}

}  //  namespace top