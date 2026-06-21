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
    fprintf(stream, "%s./tasks/%s/TASK.md%s:%s1%s: ", COLOR_RED, task->uuid, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
    fprintf(stream, "[PRIORITY: %-2d ", task->priority);
    if (task->tags.count) {
        fprintf(stream, ", TAGS: ");
        for (size_t i = 0; i < task->tags.count; ++i) {
            tag_t tag = task->tags.items[i];
            fprintf(stream, "%s%s", tag, (i != task->tags.count - 1) ? "," : "");
        }
    }
    fprintf(stream, "] %s\n", task->name);}

void print_tasks(const tasks_t *tasks, Flag_List_Mut *filters)
{
    TODO("print_tasks");
}


bool create_task(const char *path, const char *task_name)
{
    String_Builder sb = {0};
    bool result = true;

    sb_appendf(&sb, "# %s\n", task_name);
    sb_appendf(&sb, "\n");
    sb_appendf(&sb, "- STATUS: OPEN\n");
    sb_appendf(&sb, "- PRIORITY: 1\n\n");

    char *dir_name = get_timestamp_uuid();
    const char *task_path = temp_sprintf("%s%s", path, dir_name);
    const char *task_md = temp_sprintf("%s/TASK.md", task_path);

    minimal_log_level = ERROR;
    if (!mkdir_if_not_exists(task_path)) return_defer(false);
    minimal_log_level = INFO;
    if (!write_entire_file(task_md, sb.items, sb.count)) return_defer(false);

    nob_log(INFO, "Created task at: %s%s/TASK.md%s", COLOR_RED, task_path, COLOR_RESET);

defer:
    free(sb.items);
    free(dir_name);
    return result;}

bool parse_task(const char *path, const char *uuid, task_t *task)
{
    String_Builder sb = {0};
    String_View sv = {0};
    bool result = true;

    if (!read_entire_file(temp_sprintf("%s%s/TASK.md", path, uuid), &sb)) return_defer(false);
    sv = sb_to_sv(sb);

    task->path = path;
    task->uuid = uuid;

    // # Title
    String_View name = sv_chop_by_delim(&sv, '\n');
    sv_chop_by_delim(&name, ' ');
    task->name = sv_to_cstr(name);

    // Empty spacer
    sv_chop_by_delim(&sv, '\n');

    // - STATUS: CLOSED|OPEN
    String_View status = sv_chop_by_delim(&sv, '\n');
    sv_chop_prefix(&status, sv_from_cstr("- STATUS: "));
    const char *cstatus = temp_sv_to_cstr(status);
    task->status = cstr_to_task_status(cstatus);

    // - PRIORITY: UINT
    String_View priority = sv_chop_by_delim(&sv, '\n');
    sv_chop_prefix(&priority, sv_from_cstr("- PRIORITY: "));
    task->priority = temp_sv_to_int(priority);

    // TODO: Tags parsing

defer:
    free(sb.items);
    return result;
}

bool parse_tasks(const char *path, tasks_t *tasks)
{
    TODO("parse_tasks");
}

int main(int argc, char **argv)
{
    // cmdline_opts opts = {0};
    // tasks_t tasks = {0};
    // parse_options(argc, argv, &opts);

    const char *tasks_folder = "./tasks/";
    create_task(tasks_folder, "this is a test task");

#if 0
    if (!parse_tasks(tasks_folder, &tasks)) return 1;

    if (opts.list_tasks) {
        print_tasks(&tasks, &opts.filters);
    } else if (opts.create_task) {
        if (!create_task(&tasks, tasks_folder, opts.create_task)) return 1;
    } else if (opts.summary) {
        task_summary();
    }
#endif

    return 0;
}
