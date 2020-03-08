#ifndef __PARSE_ARGS_H__
#define __PARSE_ARGS_H__

#include "iterator.h"
#include "my_string.h"

// ParseArgs: ` arg1   "ar\"g\\2"` -> [`arg1`, `ar"g\2`]

typedef struct {
    const char* str;
} ParseArgs_t;

ParseArgs_t ParseArgs_new(const char* str) {
    ParseArgs_t res = { str };
    return res;
}

IterRes ParseArgs_next(ParseArgs_t* self, String_t* res) {
    // skip all spaces
    while (*self->str == ' ')
        ++self->str;
    StrSlice_t slice;
    switch (*self->str) {
        case '\0':
            return IterEnd;
        case '"':
            res->size = 0;
            slice.str = ++self->str;
            for (; *self->str != '\0'; ++self->str) {
                switch (*self->str) {
                    case '"':
                        slice.size = self->str - slice.str;
                        String_extend_with_StrSlice(res, slice);
                        ++self->str;
                        return IterOk;
                    case '\\':
                        slice.size = self->str - slice.str;
                        String_extend_with_StrSlice(res, slice);
                        ++self->str;
                        slice.str = self->str;
                        switch (*self->str) {
                            case '\\':
                            case '"':
                                break;
                            default:
                                return IterTotalErr;
                        }
                }
            }
            return IterTotalErr;
        default:
            slice.str = self->str;
            while (
                *self->str != '\0'
                && *self->str != ' '
                && *self->str != '"'
            )
                ++self->str;
            slice.size = self->str - slice.str;
            if (*self->str == ' ' || *self->str == '"')
                ++self->str;
            StrSlice_to_String(slice, res);
            return IterOk;
    }
}

#endif
