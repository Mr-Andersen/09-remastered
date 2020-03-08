#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "database.h"
#include "my_string.h"
#include "parse_args.h"
#include "split_space.h"
#include "utils.h"

enum Flow { FlowExit, FlowContinue };
typedef enum Flow (*InputHandler)(ParseArgs_t, Database_t*);
struct PatternHandler {
    const char* pattern;
    const char* help_text;
    InputHandler handler;
};

enum Flow exit_handler(ParseArgs_t it, Database_t* database);
enum Flow help_handler(ParseArgs_t it, Database_t* database);
enum Flow add_handler(ParseArgs_t it, Database_t* database);

static const struct PatternHandler handlers[] = {
    { "exit", "exit -- close shell", exit_handler },
    { "help", "help [cmd] -- print help on `cmd` or general help", help_handler },
    { "add", "add <value1> ... -- add row to table, setting value of `column1` to `value1`", add_handler},
};
static const size_t handlers_num = sizeof(handlers) / sizeof(struct PatternHandler);


enum Flow exit_handler(ParseArgs_t it, Database_t* database) {
    String_t word = String_new();
    ParseArgs_next(&it, &word);
    if (word.str != NULL) {
        puts("`exit` doesn't accept arguments. Anyway, exiting");
        String_drop(&word);
    }
    return FlowExit;
}

enum Flow help_handler(ParseArgs_t it, Database_t* database) {
    String_t word = String_new();
    String_t temp = String_new();
    ParseArgs_next(&it, &word);
    ParseArgs_next(&it, &temp);
    if (temp.str != NULL) {
        puts("`help` accepts either one or no arguments");
        puts(handlers[0].help_text);
        goto wipeout;
    }
    if (word.str == NULL) {
        puts("General syntax: <cmd> <args...>\n");
        for (size_t i = 0; i < handlers_num; ++i)
            puts(handlers[i].help_text);
    } else {
        size_t i = 0;
        for (; i < handlers_num; ++i)
            if (String_eq_str(word, handlers[i].pattern)) {
                puts(handlers[i].help_text);
                break;
            }
        if (i == handlers_num) {
            fputs("Unknown command `", stdout);
            String_fput(word, stdout);
            puts("`. Use `help` for list of commands");
        }
    }
    wipeout:
    String_drop(&word);
    String_drop(&temp);
    return FlowContinue;
}

enum Flow add_handler(ParseArgs_t it, Database_t* database) {
    String_t owned[database->col_num];

    bool die = false;
    for (size_t i = 0; i < database->col_num; ++i) {
        owned[i] = String_new();
        switch (ParseArgs_next(&it, &owned[i])) {
            case IterOk:
                break;
            case IterTotalErr:
                ERR("Invalid arguments");
                die = true;
                break;
            case IterEnd:
                fprintf(stderr, "Not enough arguments: received %zu, expected %zu\n", i, database->col_num);
                die = true;
                break;
            default:
                ; // unreachable
        }
        if (die) {
            while (i != 0)
                String_drop(&owned[--i]);
            break;
        }
    }

    StrSlice_t refs[database->col_num];

    if (ParseArgs_next(&it, &owned[0]) != IterEnd) {
        fprintf(stderr, "Too many arguments: expected %zu", database->col_num);
        goto wipeout;
    }

    for (size_t i = 0; i < database->col_num; ++i)
        refs[i] = String_as_ref(&owned[i]);

    if (Database_add(database, refs) == AddFieldOverflow)
        ERR("Value is too long");

    wipeout:
    for (size_t i = 0; i < database->col_num; ++i)
        String_drop(&owned[i]);
    return FlowContinue;
}

#endif
