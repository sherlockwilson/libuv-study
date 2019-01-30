#ifndef __MEMBUF_H__
#define __MEMBUF_H__

#include <stdlib.h>
#include <stdint.h>

namespace top {

typedef struct {
	unsigned char* data;
	unsigned int   size;
	unsigned int   buffer_size;
	unsigned char  uses_local_buffer;  // local buffer, e.g. on stack
} membuf_t;

class Membuf {
public:
    static void Init(membuf_t* buf, uint32_t initial_buffer_size);
    static void InitLocal(membuf_t* buf, void* local_buffer, uint32_t local_buffer_size);
    static void InitMoveFrom(membuf_t* buf, membuf_t* other); // don't use other anymore
    static void Uninit(membuf_t* buf);

    static unsigned int AppendData(membuf_t* buf, void* data, uint32_t size);
    static unsigned int AppendZeros(membuf_t* buf, uint32_t size);
    static unsigned int AppendText(membuf_t* buf, const char* str, uint32_t len);
    static unsigned int AppendTextZero(membuf_t* buf, const char* str, uint32_t len);

    static void* GetData(membuf_t* buf) { return (buf->size == 0 ? NULL : buf->data); }
    static unsigned int GetSize(membuf_t* buf) { return buf->size; }
    static unsigned int IsEmpty(membuf_t* buf) { return buf->size > 0; }
    static void Empty(membuf_t* buf) { buf->size = 0; }

    static void Reserve(membuf_t* buf, uint32_t extra_size);
    static void* Detach(membuf_t* buf, uint32_t* psize); // need free() result if not NULL

    static uint32_t AppendByte(membuf_t* buf, unsigned char b) {
        return AppendData(buf, &b, sizeof(b));
    }
    static uint32_t AppendInt32(membuf_t* buf, int32_t i) {
        return AppendData(buf, &i, sizeof(i));
    }
    static uint32_t AppendUint32(membuf_t* buf, uint32_t ui) {
        return AppendData(buf, &ui, sizeof(ui));
    }
    static uint32_t AppendInt8(membuf_t* buf, int8_t s) {
        return AppendData(buf, &s, sizeof(s));
    }
    static uint32_t AppendUshort(membuf_t* buf, uint8_t us) {
        return AppendData(buf, &us, sizeof(us));
    }
    static uint32_t AppendFloat(membuf_t* buf, float f) {
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

}  //  namespace top
#endif //__MEMBUF_H__