#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/commons.h"

typedef struct {
    bool help;
    bool list_tasks;
    bool summary;
    char *create_task;
    Flag_List_Mut filters;
} cmdline_opts;

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

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./tasker <OPTIONS>\n");
    fprintf(stream, "OPTIONS:\n");
    fprintf(stream, "    help\n");
    fprintf(stream, "      Lists this help message\n");
    fprintf(stream, "    ls\n");
    fprintf(stream, "      Lists all tasks. Filters as strings can be passed to filter tasks by name, status and tags\n");
    fprintf(stream, "    summary\n");
    fprintf(stream, "      Summary of the different stats of all tasks available\n");
    fprintf(stream, "    create <title>\n");
    fprintf(stream, "      Creates a task\n");
}

void parse_options(int argc, char **argv, cmdline_opts *opts)
{
    while (argc) {
        char *flag = shift(argv, argc);
        if (strcmp(flag, "help") == 0) {
            opts->help = true;
            break;
        } else if (strcmp(flag, "ls") == 0) {
            opts->list_tasks = true;
            break;
        } else if (strcmp(flag, "summary") == 0) {
            opts->summary = true;
            break;
        } else if (strcmp(flag, "create") == 0) {
            opts->create_task = shift(argv, argc);
            break;
        }
    }

    if (opts->help) {
        usage(stderr);
        exit(0);
    }
}


// YYYYMMDD-HHMMSS
// It should probably be standardized to UTF+0
char *get_timestamp_uuid()
{
    setlocale(LC_TIME, "en_US.utf-8");

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char *buffer = NULL;
    size_t size = sizeof("YYYYMMSS-HHMMSS");
    buffer = calloc(size, sizeof(char));

    // Format time; output is UTF-8 encoded due to locale
    strftime(buffer, size, "%Y%m%d-%H%M%S", tm_info);
    return buffer;
}

int temp_sv_to_int(String_View sv)
{
    return atoi(nob_temp_sv_to_cstr(sv));
}

char *sv_to_cstr(String_View sv)
{
    return strdup((char *)nob_temp_sv_to_cstr(sv));
}

task_status cstr_to_task_status(const char *cstr)
{
    if (strcmp(cstr, "CLOSED") == 0)  return CLOSED;
    else if (strcmp(cstr, "OPEN") == 0) return OPEN;
    UNREACHABLE("task_status");
}

const char *task_status_to_cstr(task_status status)
{
    switch (status) {
        case CLOSED:
            return "CLOSED";
        case OPEN:
            return "OPEN";
    }
    UNREACHABLE("task_status");
}


void task_summary()
{
    TODO("task_summary");
}

void print_task(FILE *stream, task_t *task)
{
    TODO("print_task");
}

void print_tasks(const tasks_t *tasks, Flag_List_Mut *filters)
{
    TODO("print_tasks");
}


bool create_task(tasks_t *tasks, const char *path, const char *task_name)
{
    TODO("create_task");
}

bool parse_task(const char *path, const char *uuid, task_t *task)
{
    TODO("parse_task");
}

bool parse_tasks(const char *path, tasks_t *tasks)
{
    TODO("parse_tasks");
}

int main(int argc, char **argv)
{
    cmdline_opts opts = {0};
    tasks_t tasks = {0};
    parse_options(argc, argv, &opts);

    const char *tasks_folder = "./tasks/";

    if (!parse_tasks(tasks_folder, &tasks)) return 1;

    if (opts.list_tasks) {
        print_tasks(&tasks, &opts.filters);
    } else if (opts.create_task) {
        if (!create_task(&tasks, tasks_folder, opts.create_task)) return 1;
    } else if (opts.summary) {
        task_summary();
    }

    return 0;
}
