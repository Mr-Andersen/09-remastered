#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "database.h"
#include "split_space.h"
#include "my_string.h"

enum Flow { FlowExit, FlowContinue };
typedef enum Flow (*InputHandler)(SplitWordsIter_t, Database_t*);
struct PatternHandler {
    const char* pattern;
    const char* help_text;
    InputHandler handler;
};

enum Flow exit_handler(SplitWordsIter_t it, Database_t* database);
enum Flow help_handler(SplitWordsIter_t it, Database_t* database);
enum Flow add_handler(SplitWordsIter_t it, Database_t* database);

static const struct PatternHandler handlers[] = {
    { "exit", "exit -- close shell", exit_handler },
    { "help", "help [cmd] -- print help on `cmd` or general help", help_handler },
    { "add", "add <value1> ... -- add row to table, setting value of `column1` to `value1`", add_handler},
};
static const size_t handlers_num = sizeof(handlers) / sizeof(struct PatternHandler);


enum Flow exit_handler(SplitWordsIter_t it, Database_t* database) {
    String_t word = String_new();
    SplitWordsIter_next(&it, &word);
    if (word.str != NULL) {
        puts("`exit` doesn't accept arguments. Anyway, exiting");
        String_drop(&word);
    }
    return FlowExit;
}

enum Flow help_handler(SplitWordsIter_t it, Database_t* database) {
    String_t word = String_new();
    String_t temp = String_new();
    SplitWordsIter_next(&it, &word);
    SplitWordsIter_next(&it, &temp);
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

enum Flow add_handler(SplitWordsIter_t it, Database_t* database) {
    puts("add!");
    return FlowContinue;
}

#endif
