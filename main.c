#include <stdio.h>
#ifdef WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define USE_LIBC_ALLOC
#define USE_STDIO
#include "memAlloc.h"
#include "intArray.h"
#include "stringUtils.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Expected exactly 1 argument!");
        return 1;
    }

    char *filePath = argv[1];
    if (access(filePath, F_OK) == 1) {
        fprintf(stderr, "Given file \"%s\" does not exist!", filePath);
        return 1;
    }

    FILE *file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open file!");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    char *buf = malloc(size);
    if (buf == NULL) {
        fprintf(stderr, "Not enough memory for file!");
        return 1;
    }

    fgets(buf, (int) size, file);

    String lines = NewString(buf, size);

    IntArray lineBreaks;
    IntArrayInit(&lineBreaks, 10, LIBC_ALLOC, LIBC_ALLOC_STATE);

    LineIndices(&lineBreaks, lines);

    for (size_t i = 0; i < lineBreaks.length; i ++) {
        enum StringError error = STRINGUTILS_H_OK;
        String line = LineAt(&error, lines, lineBreaks.data[i]);
        if (error != STRINGUTILS_H_OK) {
            printf("%zu! ", i);
            String err = ErrorString(error);
            StringWriteTo(err, stdout);
            putchar('\n');
            continue;
        }
        printf("%zu: ", i);
        StringWriteTo(line, stdout);
        putchar('\n');
    }

    IntArrayClear(&lineBreaks);
    free(buf);

    return 0;
}