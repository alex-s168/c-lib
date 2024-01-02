#ifndef C_MEMALLOC_H
#define C_MEMALLOC_H

#include <stddef.h>

typedef struct {
    void *(*Alloc)(void *state, size_t size);
    void (*Free)(void *state, void *data, size_t size);
} Allocator;

static void *MemCopy(void *dstIn, const void *srcIn, size_t len) {
    unsigned char *dst = (unsigned char *) dstIn;
    unsigned char *src = (unsigned char *) srcIn;

    while (len) {
        *dst++ = *src++;
        -- len;
    }

    return dst;
}

void *ReAlloc(Allocator allocator, void *allocatorState, void *data, size_t sizeNew, size_t sizeOld) {
    void *newAlloc = allocator.Alloc(allocatorState, sizeNew);
    if (newAlloc == NULL) {
        return NULL;
    }

    size_t x = sizeOld;
    if (sizeNew < sizeOld) {
        x = sizeNew;
    }

    MemCopy(newAlloc, data, x);

    if (data != NULL) {
        allocator.Free(allocatorState, data, sizeOld);
    }

    return newAlloc;
}

#ifdef USE_LIBC_ALLOC
#include <stdlib.h>
static void *MemAlloc_H_libcAlloc_alloc(void *state, size_t size) {
    return malloc(size);
}
static void MemAlloc_H_libcAlloc_free(void *state, void *data, size_t size) {
    free(data);
}
Allocator MemAlloc_H_libcAlloc = (Allocator) { .Alloc = MemAlloc_H_libcAlloc_alloc, .Free = MemAlloc_H_libcAlloc_free };
#define LIBC_ALLOC MemAlloc_H_libcAlloc
#define LIBC_ALLOC_STATE ((void *) {})
#endif

#endif //C_MEMALLOC_H
