#ifndef COMMONS_H_
#define COMMONS_H_

#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "../thirdparty/nob.h"
#include "../thirdparty/flag.h"
#include "../thirdparty/ht.h"

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 1

#define STRINGIFY_HELPER(x) #x
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define VERSION STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH)

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

#define SRC_DIR         "src/"
#define BUILD_DIR       "./build/"
#define THIRDPARTY_DIR  "thirdparty/"
#define BIN_DIR         BUILD_DIR"bin/"

#define TASKS_DIR "./tasks/"

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef struct cmdline_opts_s {
    bool  help;
    bool  list_tasks;
    bool  list_tasks_reversed;
    bool  remove_tasks;
    bool  init_dir;
    bool  close_tasks;
    bool  summary;
    char *edit_task;
    char *find_task;
    char *create_task;
    char *create_task_tags;
    u16   create_task_priority;
    Flag_List_Mut filters;
} cmdline_opts_t;


#endif // COMMONS_H_

