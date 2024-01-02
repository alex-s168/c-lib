#ifndef C_STRINGUTILS_H
#define C_STRINGUTILS_H

#include <stddef.h>
#include "memAlloc.h"

#ifndef C_INTARRAY_H
#warning IntArray.h should be included before this header
#endif
#include "intArray.h"

/**
 * A string.
 * "data" is the data of the string. This does not have to be null-terminated!
 * "len" is the string's length.
 */
typedef struct {
    size_t len;
    char *data;
} String;

enum StringError {
    STRINGUTILS_H_OK,
    STRINGUTILS_H_BOUNDS,
    STRINGUTILS_H_NOTFOUND
};

static size_t StringUtils_h_strlen(char *cstring) {
    size_t len = 0;
    while (*cstring != '\0') {
        len ++;
        cstring ++;
    }
    return len;
}

String NewString(char *str, size_t len) {
    return (String) { .len = len, .data = str };
}

String CString(char *str) {
    return (String) { .len = StringUtils_h_strlen(str), .data = str };
}

char *StringC(String str, Allocator allocator, void *allocatorState) {
    char *alloc = allocator.Alloc(allocatorState, str.len);

    MemCopy(alloc, str.data, str.len);

    return alloc;
}

#ifdef USE_STDIO
#include "stdio.h"
void StringWriteTo(String str, FILE *file) {
    char *end = str.data + str.len;
    char *curr = str.data;
    while (curr < end) {
        fputc(*curr, file);
        curr ++;
    }
}
#endif

size_t LineIndices(IntArray *arr, String string) {
    size_t oLen = arr->length;

    IntArrayAppend(arr, 0);

    for (size_t i = 0; i < string.len; i ++) {
        char c = string.data[i];

        if (c == '\n') {
            IntArrayAppend(arr, (int) i + 1);
        }
    }

    return arr->length - oLen;
}

String EmptyString() {
    return (String) { .len = 0 };
}

String SubStringSE(enum StringError *error, String str, size_t start, size_t end) {
    if (*error != STRINGUTILS_H_OK) {
        return EmptyString();
    }

    printf("start: %zu/%zu  end: %zu/%zu\n", start, str.len, end, str.len);

    if (start >= str.len || end >= str.len) {
        *error = STRINGUTILS_H_BOUNDS;
        return EmptyString();
    }

    if (start > end) {
        size_t t = start;
        start = end;
        end = t;
    }

    return (String) { .data = str.data + start, .len = end - start };
}

String SubStringS(enum StringError *error, String str, size_t start) {
    return SubStringSE(error, str, start, str.len - 1);
}

String SubStringE(enum StringError *error, String str, size_t end) {
    return SubStringSE(error, str, 0, end);
}

size_t StringFindFirst(enum StringError *error, String str, char c) {
    if (*error != STRINGUTILS_H_OK) {
        return 0;
    }

    for (size_t i = 0; i < str.len; i ++) {
        if (str.data[i] == c) {
            return i;
        }
    }

    *error = STRINGUTILS_H_NOTFOUND;
    return 0;
}

String LineAt(enum StringError *error, String str, size_t pos) {
    String cut = SubStringS(error, str, pos);
    enum StringError terr = STRINGUTILS_H_OK;
    size_t end = StringFindFirst(&terr, cut, '\n');
    if (terr != STRINGUTILS_H_OK) {
        end = cut.len - 1;
    }
    String res = SubStringE(error, cut, end);
    return res;
}

String ErrorString(enum StringError err) {
    switch (err) {
        default:
        case STRINGUTILS_H_OK:
            return EmptyString();
        case STRINGUTILS_H_BOUNDS:
            return CString("Out of bounds!");
        case STRINGUTILS_H_NOTFOUND:
            return CString("Not found!");
    }
}

#endif //C_STRINGUTILS_H
