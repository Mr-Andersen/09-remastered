#ifndef __PARSE_ARGS_H__
#define __PARSE_ARGS_H__

#include "iterator.h"
#include "my_string.h"
#include "utils.h"

// ParseArgs: ` arg1   "ar\"g\\2"` -> [`arg1`, `ar"g\2`]

typedef struct {
    StrSlice_t slice;
} ParseArgs_t;

ParseArgs_t ParseArgs_new(StrSlice_t slice) {
    ParseArgs_t res = { slice };
    return res;
}

IterRes ParseArgs_next(ParseArgs_t* self, String_t* res) {
    // skip all spaces
    while (*self->slice.str == ' ' && self->slice.size > 0) {
        ++self->slice.str;
        --self->slice.size;
    }
    if (self->slice.size == 0)
        return IterEnd;
    StrSlice_t part;
    switch (*self->slice.str) {
        case '"':
            res->size = 0;
            part.str = ++self->slice.str;
            --self->slice.size;
            for (; self->slice.size > 0; ++self->slice.str, --self->slice.size) {
                switch (*self->slice.str) {
                    case '"':
                        part.size = self->slice.str - part.str;
                        String_extend_with_StrSlice(res, part);
                        ++self->slice.str;
                        --self->slice.size;
                        return IterOk;
                    case '\\':
                        part.size = self->slice.str - part.str;
                        String_extend_with_StrSlice(res, part);
                        ++self->slice.str;
                        --self->slice.size;
                        part.str = self->slice.str;
                        switch (*self->slice.str) {
                            case '\\':
                            case '"':
                                break;
                            default:
                                ERR("Something aside from '\"' or '\\' appeared after '\\'");
                                return IterTotalErr;
                        }
                }
            }
            return IterTotalErr;
        default:
            part.str = self->slice.str;
            while (
                self->slice.size > 0
                && *self->slice.str != ' '
                && *self->slice.str != '"'
            ) {
                ++self->slice.str;
                --self->slice.size;
            }
            part.size = self->slice.str - part.str;
            if (self->slice.size > 0 && (*self->slice.str == ' ' || *self->slice.str == '"')) {
                ++self->slice.str;
                --self->slice.size;
            }
            StrSlice_to_String(part, res);
            return IterOk;
    }
}

#endif
