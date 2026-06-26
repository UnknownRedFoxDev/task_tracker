#ifndef COMMONS_H_
#define COMMONS_H_

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

#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#include "../thirdparty/nob.h"
#include "../thirdparty/flag.h"
#include "../thirdparty/ht.h"

typedef struct {
    bool help;
    bool list_tasks;
    bool remove_tasks;
    bool close_tasks;
    bool summary;
    char *edit_task;
    char *find_task;
    char *create_task;
    char *create_task_tags;
    int create_task_priority;
    Flag_List_Mut filters;
} cmdline_opts;

#endif // COMMONS_H_

