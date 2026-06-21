#include "../lib/task.h"
#include "../lib/helper.h"

void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./tasker <OPTIONS>\n");
    fprintf(stream, "OPTIONS:\n");
    fprintf(stream, "    help\n");
    fprintf(stream, "      Lists this help message\n");
    fprintf(stream, "\n");
    fprintf(stream, "    ls\n");
    fprintf(stream, "      Lists all tasks. Filters as strings can be passed to filter tasks by name, status and tags\n");
    fprintf(stream, "\n");
    fprintf(stream, "    summary\n");
    fprintf(stream, "      Summary of the different stats of all tasks available\n");
    fprintf(stream, "\n");
    fprintf(stream, "    create <title>\n");
    fprintf(stream, "      Creates a task\n");
    fprintf(stream, "\n");
    fprintf(stream, "    open <task-id>\n");
    fprintf(stream, "      Opens the task specified in your $EDITOR of choice. Default to vim if $EDITOR is not set.\n");
    fprintf(stream, "\n");
    fprintf(stream, "    find <task-id>\n");
    fprintf(stream, "      Finds the task specified and prints it to the output.\n");
    fprintf(stream, "\n");
    fprintf(stream, "    rm <task-id> [<task-id> [<task-id> [...] ] ]\n");
    fprintf(stream, "      Deletes the task(s) specified.\n");
    fprintf(stream, "\n");
    fprintf(stream, "    close <task-id> [<task-id> [<task-id> [...] ] ]\n");
    fprintf(stream, "      Closes the task(s) specified.\n");
}

void parse_options(int argc, char **argv, cmdline_opts *opts)
{
    // Just calling the program
    if (argc < 2) {
        opts->list_tasks = true;
        da_append(&opts->filters, "");
        return ;
    }

    char *program_name = shift(argv, argc);
    UNUSED(program_name);

    while (argc) {
        char *flag = shift(argv, argc);
        if (strcmp(flag, "help") == 0) {
            opts->help = true;
            break;
        } else if (strcmp(flag, "summary") == 0) {
            opts->summary = true;
            break;
        } else if (strcmp(flag, "create") == 0) {
            opts->create_task = shift(argv, argc);
            break;
        } else if (strcmp(flag, "ls") == 0) {
            opts->list_tasks = true;
            break;
        } else if (strcmp(flag, "open") == 0) {
            opts->open_task = shift(argv, argc);
            break;
        } else if (strcmp(flag, "find") == 0) {
            opts->find_task = shift(argv, argc);
            break;
        } else if (strcmp(flag, "rm") == 0) {
            opts->remove_tasks = true;
            break;
        } else if (strcmp(flag, "close") == 0) {
            opts->close_tasks = true;
            break;
        }
    }

    if (argc) {
        while (argc) {
            char *filter = shift(argv, argc);
            da_append(&opts->filters, filter);
        }
    } else {
        da_append(&opts->filters, "");
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
