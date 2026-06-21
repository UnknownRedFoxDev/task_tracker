#ifndef TASK_H_
#define TASK_H_

#include "commons.h"

#define DISTANCE_THRESHOLD 5

typedef enum {
    CLOSED,
    OPEN,
} task_status;

typedef char * tag_t;

typedef struct {
    tag_t *items;
    size_t count;
    size_t capacity;
} tags_t;

typedef struct {
    const char *path;
    const char *uuid;
    char *name;
    uint8_t priority;
    task_status status;
    tags_t tags;
} task_t;

typedef struct {
    task_t *items;
    size_t count;
    size_t capacity;
} tasks_t;

void task_summary();
void print_task(FILE *stream, task_t *task);
void print_tasks(const tasks_t *tasks, Flag_List_Mut *filters);
bool create_task(const char *path, const char *task_name);
bool parse_task(const char *path, const char *uuid, task_t *task);
bool parse_tasks(const char *path, tasks_t *tasks);
void free_task(task_t *task);
void free_tasks(tasks_t *tasks);

#endif // TASK_H_
