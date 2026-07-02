#ifndef TASK_H_
#define TASK_H_

#include "commons.h"

#define DISTANCE_THRESHOLD 5

typedef enum {
    CLOSED,
    OPEN,
} task_status;

typedef struct {
    char *path;
    char *uuid;
    char *name;
    uint8_t priority;
    task_status status;
    Ht(const char *, bool) tags;
} task_t;

typedef struct {
    task_t *items;
    size_t count;
    size_t capacity;
} tasks_t;

void task_summary();
void print_task(FILE *stream, task_t *task);
bool print_tasks(const tasks_t *tasks, Flag_List_Mut *tokens, bool reversed);
task_t *create_task(const char *path, const char *task_name, cmdline_opts *opts);
bool parse_task(const char *path, const char *uuid, task_t *task);
bool parse_tasks(const char *path, tasks_t *tasks);
void free_task(task_t *task);
void free_tasks(tasks_t *tasks);
task_t *find_task(tasks_t *tasks, const char *uuid);
bool close_task(task_t *task);
bool close_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid);
bool remove_task(task_t *task);
bool remove_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid);
bool open_task(task_t *task);
void init_directory(const char *tasks_dir);

#endif // TASK_H_
