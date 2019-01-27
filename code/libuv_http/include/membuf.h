#ifndef __MEMBUF_H__
#define __MEMBUF_H__

#include <stdlib.h>

typedef struct {
	unsigned char* data;
	unsigned int   size;
	unsigned int   buffer_size;
	unsigned char  uses_local_buffer;  // local buffer, e.g. on stack
} membuf_t;

static void swap_data(membuf_t* buf1, membuf_t* buf2);
static void swap_size(membuf_t* buf1, membuf_t* buf2);
static void swap_buffer_size(membuf_t* buf1, membuf_t* buf2);

class Membuf {
public:
    static void Init(membuf_t* buf, unsigned int initial_buffer_size);
    static void InitLocal(membuf_t* buf, void* local_buffer, unsigned int local_buffer_size);
    static void InitMoveFrom(membuf_t* buf, membuf_t* other); // don't use other anymore
    static void Uninit(membuf_t* buf);

    static unsigned int AppendData(membuf_t* buf, void* data, unsigned int size);
    static unsigned int AppendZeros(membuf_t* buf, unsigned int size);
    static unsigned int AppendText(membuf_t* buf, const char* str, unsigned int len);
    static unsigned int AppendTextZero(membuf_t* buf, const char* str, unsigned int len);

    static void* GetData(membuf_t* buf) { return (buf->size == 0 ? NULL : buf->data); }
    static unsigned int GetSize(membuf_t* buf) { return buf->size; }
    static unsigned int IsEmpty(membuf_t* buf) { return buf->size > 0; }
    static void Empty(membuf_t* buf) { buf->size = 0; }

    static void Reserve(membuf_t* buf, unsigned int extra_size);
    static void* Detach(membuf_t* buf, unsigned int* psize); // need free() result if not NULL

    static unsigned int AppendByte(membuf_t* buf, unsigned char b) {
        return AppendData(buf, &b, sizeof(b));
    }
    static unsigned int AppendInt(membuf_t* buf, int i) {
        return AppendData(buf, &i, sizeof(i));
    }
    static unsigned int AppendUint(membuf_t* buf, unsigned int ui) {
        return AppendData(buf, &ui, sizeof(ui));
    }
    static unsigned int AppendShort(membuf_t* buf, short s) {
        return AppendData(buf, &s, sizeof(s));
    }
    static unsigned int AppendUshort(membuf_t* buf, unsigned short us) {
        return AppendData(buf, &us, sizeof(us));
    }
    static unsigned int AppendFloat(membuf_t* buf, float f) {
        return AppendData(buf, &f, sizeof(f));
    }
    static unsigned int AppendDouble(membuf_t* buf, double d) {
        return AppendData(buf, &d, sizeof(d));
    }
    static unsigned int AppendPtr(membuf_t* buf, void* ptr) {
        return AppendData(buf, &ptr, sizeof(ptr));
    }
};

#ifndef MEMBUF_INIT_LOCAL
#define MEMBUF_INIT_LOCAL(buf,n) membuf_t buf; unsigned char buf##n[n]; Membuf::InitLocal(&buf, &buf##n, n);
#endif


#endif //__MEMBUF_H__
