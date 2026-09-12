#ifndef TASK_H_
#define TASK_H_

#include "commons.h"
#include "../lib/parser.h"

#define DISTANCE_THRESHOLD 5

typedef struct tasks_s tasks_t;

typedef struct {
    char *name;
    char *description;
} tag_t;

typedef struct {
    tag_t *items;
    size_t count;
    size_t capacity;
} tags_t;

typedef struct task_s {
    const struct task_s *parent;
    char *path;
    char *uuid;
    char *name;
    long priority;
    task_status status;
    Ht(const char *, bool) tags;
    tasks_t *subtasks;
} task_t;

struct tasks_s {
    task_t *items;
    u64 count;
    u64 capacity;
};

typedef Ht(char *, task_t *) tag_set;

// Initialisation functions
void initialise_tasks();
void init_directory(const char *tasks_dir, bool force_init);

// "Feature" functions
task_t *find_task(tasks_t *tasks, const char *uuid);
task_t *create_task(const char *path, task_info_t *info, bool no_editor);
bool overwrite_task(tasks_t *tasks, task_info_t *info);
void task_summary(const char *tasks_dir);
bool open_task(task_t *task);
bool remove_task(task_t *task);
void cat_task(task_t *task);
bool print_tasks(const tasks_t *tasks, Flag_List_Mut *tokens, print_tasks_opt opts);
bool remove_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid, int last_n);

// helper functions
bool parse_task(const char *path, const char *uuid, task_t *task, tasks_t *tasks);
bool parse_tasks(const char *path, tasks_t *tasks, const task_t *parent, tasks_t *subtasks);
void print_task(FILE *stream, task_t *task, int alignment);
bool parse_tags(const char *tasks_path);
bool change_tasks_status(tasks_t *tasks, Flag_List_Mut *tasks_uuid, task_status new_status);
size_t find_best_alignment(task_t *tasks, u32 tasks_len);
u32 retrieve_tasks_from_query(const tasks_t *tasks, Node_t *root, bool negated, task_t **result);

// memory management
void free_tags(tags_t *tags);
void free_task(task_t *task);
void free_tasks(tasks_t *tasks);

#endif // TASK_H_
