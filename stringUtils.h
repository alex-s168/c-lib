#ifndef C_STRINGUTILS_H
#define C_STRINGUTILS_H

#include <stddef.h>
#include "memAlloc.h"

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
    STRINGUTILS_H_NOTFOUND,
    STRINGUTILS_H_NOMEM,
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

/**
 * @param str
 * @param allocator
 * @param allocatorState
 * @param count The maximum amount of chars to convert.
 * 0 means all. Allocates @param count + 1 bytes
 * @return
 */
char *StringC(String str, Allocator allocator, void *allocatorState, size_t count) {
    if (count == 0 || count > str.len) {
        count = str.len;
    }

    char *alloc = allocator.Alloc(allocatorState, count + 1);

    MemCopy(alloc, str.data, count);
    alloc[count] = '\0';

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

String EmptyString() {
    return (String) { .len = 0 };
}

String SubStringSE(enum StringError *error, String str, size_t start, size_t end) {
    if (*error != STRINGUTILS_H_OK) {
        return EmptyString();
    }

    if (start >= str.len || end >= str.len) {
        *error = STRINGUTILS_H_BOUNDS;
        return EmptyString();
    }

    if (start > end) {
        size_t t = start;
        start = end;
        end = t;
    }

    return (String) { .data = str.data + start, .len = end - start + 1 };
}

String SubStringS(enum StringError *error, String str, size_t start) {
    return SubStringSE(error, str, start, str.len - 1);
}

String SubStringE(enum StringError *error, String str, size_t end) {
    return SubStringSE(error, str, 0, end);
}

size_t StringFindFirst(enum StringError *error, String str, char c, size_t instead) {
    if (*error != STRINGUTILS_H_OK) {
        return instead;
    }

    for (size_t i = 0; i < str.len; i ++) {
        if (str.data[i] == c) {
            return i;
        }
    }

    *error = STRINGUTILS_H_NOTFOUND;
    return instead;
}

String LineAt(enum StringError *error, String str, size_t pos) {
    String cut = SubStringS(error, str, pos);
    enum StringError terr = STRINGUTILS_H_OK; // ignore
    size_t end = StringFindFirst(&terr, cut, '\n', cut.len) - 1;
    return SubStringE(error, cut, end);
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

typedef struct {
    String str;
    size_t allocSize;
    Allocator alloc;
    void *allocState;
} OwnedString;

OwnedString AllocString(enum StringError *error, Allocator alloc, void *allocState, size_t length) {
    char *strAlloc = alloc.Alloc(allocState, length);
    if (strAlloc == NULL) {
        *error = STRINGUTILS_H_NOMEM;
    }
    String str = (String) { .len = 0, .data = strAlloc };
    return (OwnedString) {
        .str = str,
        .allocState = allocState,
        .alloc = alloc,
        .allocSize = length
    };
}

void ClearString(OwnedString *str) {
    str->str.len = 0;
    str->alloc.Free(str->allocState, str->str.data, str->allocSize);
    str->allocSize = 0;
    str->str.data = NULL;
}

OwnedString *StringReserve(enum StringError *error, OwnedString *str, size_t additional) {
    size_t strLen = str->str.len;
    size_t strAllocSize = str->allocSize;
    size_t extraAvail = strAllocSize - strLen;
    if (extraAvail >= additional) {
        return str;
    }
    size_t newSize = strLen + additional;
    char *newAlloc = ReAlloc(str->alloc, str->allocState,
                             str->str.data, newSize, strAllocSize);
    if (newAlloc == NULL) {
        *error = STRINGUTILS_H_NOMEM;
        return str;
    }
    str->str.data = newAlloc;
    str->allocSize = newSize;
    return str;
}

OwnedString *StringAppendString(OwnedString *self, enum StringError *error, String str) {
    enum StringError tempErr = STRINGUTILS_H_OK;
    StringReserve(&tempErr, self, str.len);
    if (tempErr != STRINGUTILS_H_OK) {
        *error = tempErr;
        return self;
    }

    MemCopy(self->str.data + self->str.len, str.data, str.len);
    self->str.len += str.len;

    return self;
}

OwnedString *StringAppendInt(OwnedString *self, enum StringError *error, signed int i) {
    enum StringError tempErr = STRINGUTILS_H_OK;
    StringReserve(&tempErr, self, 11);
    if (tempErr != STRINGUTILS_H_OK) {
        *error = tempErr;
        return self;
    }

    size_t count = sprintf(self->str.data + self->str.len, "%i", i);
    self->str.len += count;

    return self;
}

#include "intArray.h"

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

#endif //C_STRINGUTILS_H
