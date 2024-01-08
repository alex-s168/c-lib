#ifndef C_INTARRAY_H
#define C_INTARRAY_H

#include "memAlloc.h"
#include <stddef.h>

/**
 * Error values returned by IntArray functions.
 * @see IntArrayErrStr
 */
enum IntArrayErr {
    INTARRAY_H_OK,
    INTARRAY_H_NOMEM,
    INTARRAY_H_BOUNDS
};

/**
 * Integer array
 */
typedef struct {
    size_t length;
    size_t allocation;
    int *data;

    Allocator allocator;
    void *allocatorState;
} IntArray;

/**
 * Initializes an IntArray
 * @param arr The destination array
 * @param preAlloc The amount of elements to pre-allocate
 * @return @see IntArrayErr
 */
enum IntArrayErr IntArrayInit(IntArray *arr, size_t preAlloc, Allocator allocator, void *allocatorState) {
    int *alloc = allocator.Alloc(allocatorState, preAlloc * sizeof(int));
    if (alloc == NULL) {
        return INTARRAY_H_NOMEM;
    }
    *arr = (IntArray) {
        .length = 0,
        .allocation = preAlloc,
        .data = alloc,

        .allocator = allocator,
        .allocatorState = allocatorState
    };
    return INTARRAY_H_OK;
}

/**
 * Reserves an additional x elements to be stored in the array.
 * @param arr The array
 * @param additional The amount of extra elements to reserve
 * @return @see IntArrayErr
 */
enum IntArrayErr IntArrayReserve(IntArray *arr, size_t additional) {
    size_t x = arr->allocation - arr->length;
    if (x >= additional) {
        return INTARRAY_H_OK;
    }
    size_t y = arr->length + additional;
    int *alloc = ReAlloc(
            arr->allocator,
            arr->allocatorState,
            arr->data,
            y * sizeof(int),
            arr->allocation * sizeof(int));
    if (alloc == NULL) {
        return INTARRAY_H_NOMEM;
    }
    arr->data = alloc;
    arr->allocation = y;
    return INTARRAY_H_OK;
}

/**
 * Clears the given array (and deallocates the memory associated with it)
 * @param arr The array to clear
 */
enum IntArrayErr IntArrayClear(IntArray *arr) {
    arr->allocator.Free(arr->allocatorState, arr->data, arr->allocation);
    arr->data = NULL;
    arr->allocation = 0;
    arr->length = 0;
    return INTARRAY_H_OK;
}

/**
 * Copies the contents of an IntArray to a buffer
 * @param arr the source
 * @param buf the destination
 * @param count the amount of elements to copy from source to destination
 * @param offset the source offset
 * @return @see IntArrayErr
 */
enum IntArrayErr IntArrayStore(IntArray arr, int *buf, size_t count, size_t offset) {
    if (count + offset > arr.length) {
        return INTARRAY_H_BOUNDS;
    }

    MemCopy(
            buf,
            arr.data + offset,
            count * sizeof(int));

    return INTARRAY_H_OK;
}

/**
 * Appends the given element to an array
 * @param arr The array to append to
 * @param x The element to append
 * @return @see IntArrayErr
 */
enum IntArrayErr IntArrayAppend(IntArray *arr, int x) {
    enum IntArrayErr err = IntArrayReserve(arr, 1);
    if (err != INTARRAY_H_OK) {
        return err;
    }

    arr->data[arr->length ++] = x;

    return INTARRAY_H_OK;
}

/**
 * Removes the last n elements from an array
 * @param arr The array to remove from
 * @param count The amount of elements to remove
 * @return @see IntArrayErr
 */
enum IntArrayErr IntArrayRemoveLast(IntArray *arr, int count) {
    size_t l = (arr->length -= count);

    int *alloc = ReAlloc(
            arr->allocator,
            arr->allocatorState,
            arr->data,
            l * sizeof(int),
            arr->allocation * sizeof(int));
    if (alloc == NULL) {
        return INTARRAY_H_NOMEM;
    }
    arr->data = alloc;
    arr->allocation = l;

    return INTARRAY_H_OK;
}

/**
 * Copies the contents of an buffer into an IntArray
 * @param arr the destination
 * @param buf the source
 * @param count the amount of elements to copy
 * @return @see IntArrayErr
 */
enum IntArrayErr IntArrayBulkAppend(IntArray *arr, const int *buf, size_t count) {
    enum IntArrayErr err = IntArrayReserve(arr, count);
    if (err != INTARRAY_H_OK) {
        return err;
    }

    MemCopy(
            arr->data + arr->length,
            buf,
            count * sizeof(int));

    return INTARRAY_H_OK;
}

/**
 * Returns the error string of an IntArray error.
 * @param @see IntArrayErr
 * @return The string value of that error
 */
char *IntArrayErrStr(enum IntArrayErr error) {
    switch (error) {
        case INTARRAY_H_OK:
        default:
            return "";
        case INTARRAY_H_NOMEM:
            return "Out of memory!";
        case INTARRAY_H_BOUNDS:
            return "Out of bounds!";
    }
}

#ifdef C_STRINGUTILS_H
OwnedString *StringAppendIntArray(OwnedString *self, enum StringError *error, IntArray arr) {
    StringAppendString(self, error, CString("["));
    for (size_t i = 0; i < arr.length; i ++) {
        if (i != 0) {
            StringAppendString(self, error, CString(", "));
        }
        int v = arr.data[i];
        StringAppendInt(self, error, v);
    }
    StringAppendString(self, error, CString("]"));
    return self;
}
#endif

#endif //C_INTARRAY_H
