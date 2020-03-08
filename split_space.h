#ifndef __SPLIT_SPACE_H__
#define __SPLIT_SPACE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iterator.h"
#include "my_string.h"

typedef struct {
    const char* str;
} SplitSpaceIter_t;

IterRes SplitSpaceIter_next(SplitSpaceIter_t* self, StrSlice_t* slice) {
    while (*self->str == ' ')
        ++self->str;
    if (*self->str == '\0') {
        slice->str = NULL;
        slice->size = 0;
        return IterEnd;
    }
    slice->str = self->str;
    while (*self->str != '\0' && *self->str != ' ')
        ++self->str;
    slice->size = self->str - slice->str;
    return IterOk;
}

typedef struct {
    SplitSpaceIter_t ss_iter;
} SplitWordsIter_t;

SplitWordsIter_t SplitWordsIter_new(const char* str) {
    SplitWordsIter_t res = { { str } };
    return res;
}

IterRes SplitWordsIter_next(
    SplitWordsIter_t* self,
    String_t* res
) {
    StrSlice_t slice;
    if (SplitSpaceIter_next(&self->ss_iter, &slice) != IterOk)
        return IterEnd;
    StrSlice_to_String(slice, res);
    while (slice.str[slice.size - 1] == '\\') {
        fputs("last slice: '", stdout);
        StrSlice_fput(slice, stdout);
        puts("'");
        printf("adding: '%s'\n", res->str);
        if (SplitSpaceIter_next(&self->ss_iter, &slice) != IterOk)
            return IterTotalErr;
        res->str[res->size - 1] = ' ';
        String_extend_with_StrSlice(res, slice);
    }
    return IterOk;
}

#endif
