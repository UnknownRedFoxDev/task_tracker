#include <stdlib.h>
#include "../lib/helper.h"

typedef struct {
    const char *name;
    const char *description[16];
} option_t;

void usage(FILE *stream, const char *program_name)
{
    option_t descriptions[] = {
        (option_t){
            .name = "help",
            .description = {
                "Lists this help message",
            },
        },
        (option_t){
            .name = "version",
            .description = {
                "Prints the git hash of the tooling",
            },
        },
        (option_t){
            .name = "init",
            .description = {
                "Initialise the \"tasks\" directory if not already present.",
            },
        },
        (option_t){
            .name = "summary",
            .description = {
                "Prints a summary of the tasks and their tags",
            }
        },
        (option_t){
            .name = "ls",
            .description = {
                "Lists all tasks. Filters as strings can be passed to filter tasks by name, status and tags",
            }
        },
        (option_t){
            .name = "ls-rev",
            .description = {
                "Same as `ls` but reverses the result",
            }
        },
        (option_t){
            .name = "new",
            .description = {
                "Creates a task",
                // "OPTIONS:",
                // "    -t <tags> : Add tags to the new task. Tags are comma seperated without space",
                // "    -p <priority> : Change priority from default 100 priority",
                // "    --no-editor : doesn't automatically open the task in your $EDITOR of choice",
            }
        },
        (option_t){
            // .name = "close <task-huid> [...]",
            .name = "close",
            .description = {
                "Closes the task(s) given",
            }
        },
        (option_t){
            // .name = "reopen <task-huid> [...]",
            .name = "reopen",
            .description = {
                "Re-open the task(s) given, if they were closed",
            }
        },
        (option_t){
            // .name = "find <task-huid>",
            .name = "find",
            .description = {
                "Finds the task specified and prints it to the output",
            }
        },
        (option_t){
            // .name = "cat <task-huid>",
            .name = "cat",
            .description = {
                "Prints the details of the task to the output",
            }
        },
        (option_t){
            // .name = "edit <task-huid>",
            .name = "edit",
            .description = {
                "Opens the task specified in your $EDITOR (or vim if $EDITOR is not set) of choice",
            }
        },
        (option_t){
            // .name = "del | rm [-last <int>] <task-huid> [...]",
            .name = "del",
            .description = {
                "Deletes the task(s) given",
                // "Using the `-last` flag lets you delete the last n tasks opened. Incompatible with giving huid"
            }
        },
        (option_t){
            // .name = "del | rm [-last <int>] <task-huid> [...]",
            .name = "rm",
            .description = {
                "Deletes the task(s) given",
                // "Using the `-last` flag lets you delete the last n tasks opened. Incompatible with giving huid"
            }
        },
        (option_t){
            // .name = "overwrite [-t [+|-]<tags>[,...]] [-p [+|-]<priority>] [-s <O[PEN] | C[LOSED]>] [title] <task-huid [...] | query>",
            .name = "overwrite",
            .description = {
                "Given a task-huid, or a query, you can modify the tasks' tags, priority, status and title",
            }
        },
    };
    fprintf(stream, "Usage: %s <OPTIONS>\n", program_name);
    fprintf(stream, "OPTIONS:\n");

    size_t opt_len = ARRAY_LEN(descriptions);
    for (size_t i = 0; i < opt_len; ++i) {
        fprintf(stream, "    %-*s- %s\n", 10, descriptions[i].name, descriptions[i].description[0]);
        // for (size_t j = 0; j < ARRAY_LEN((descriptions[i].description)); ++j) {
        //     if (descriptions[i].description[j]) {
        //         fprintf(stream, "        %s\n", descriptions[i].description[j]);
        //     }
        // }
        // fprintf(stream, "\n");
    }
}

void parse_options(int argc, char **argv, cmdline_opts_t *opts, char **program_name)
{
    (*program_name) = shift(argv, argc);
    if (argc < 1) {
        usage(stderr, *program_name);
        fprintf(stderr, "No command provided\n");
        exit(1);
    }

    while (argc) {
        char *flag = shift(argv, argc);
        if (strstr(flag, "--") || strstr(flag, "-")) {
            fprintf(stderr, "No '--' or '-' is needed for the tool's options\n");
            usage(stderr, *program_name);
            exit(1);
        }

        if (strcmp(flag, "help") == 0) {
            opts->help = true;
            break;
        } else if (strcmp(flag, "summary") == 0 || strcmp(flag, "sum") == 0) {
            opts->summary = true;
            break;
        } else if (strcmp(flag, "version") == 0) {
            opts->version = true;
            break;
        } else if (strcmp(flag, "new") == 0) {
            opts->create_task = calloc(1, sizeof(task_info_t));
            assert(opts->create_task != NULL && "Failed to allocated space for overwrite's task info structure");

            opts->create_task->priority = NULL;
            opts->create_task->tags = NULL;
            while (argc > 0) {
                flag = shift(argv, argc);
                if (argc > 0) {
                    if (strcmp(flag, "-t") == 0) {
                        opts->create_task->tags = shift(argv, argc);
                    } else if (strcmp(flag, "-p") == 0) {
                        opts->create_task->priority = shift(argv, argc);
                    } else if (strcmp(flag, "--no-editor") == 0) {
                        opts->no_editor = true;
                    } else {
                        opts->create_task->title = flag;
                    }
                } else {
                    opts->create_task->title = flag;
                }
            }
            break;
        } else if (strcmp(flag, "ls") == 0) {
            opts->list_tasks = true;
            if (argc > 0) {
                flag = argv[0];
                if (strcmp(flag, "-id") == 0) {
                    opts->print_tasks_opts.byHUID = true;
                    shift(argv, argc);
                }
            }
            break;
        } else if (strcmp(flag, "ls-rev") == 0) {
            opts->list_tasks = true;
            opts->print_tasks_opts.reversed = true;
            if (argc > 0) {
                flag = argv[0];
                if (strcmp(flag, "-id") == 0) {
                    opts->print_tasks_opts.byHUID = true;
                    shift(argv, argc);
                }
            }
            break;
        } else if (strcmp(flag, "edit") == 0) {
            if (!argc) {
                nob_log(ERROR, "No task-id was provided");
                exit(1);
            }
            opts->edit_task = shift(argv, argc);
            break;
        } else if (strcmp(flag, "cat") == 0) {
            opts->cat_task = shift(argv, argc);
            break;
        } else if (strcmp(flag, "find") == 0) {
            opts->find_task = shift(argv, argc);
            break;
        } else if (strcmp(flag, "rm") == 0 || strcmp(flag, "del") == 0) {
            opts->last_n = 0;
            if (argc > 0) {
                flag = argv[0];
                if (strcmp(flag, "-last") == 0) {
                    opts->last_n = atoi(argv[1]);
                }
            }
            opts->remove_tasks = true;
            break;
        // TASK(20260805-161529): Change close and reopen cmdline options to use overwrite function rather than having its own thing
        } else if (strcmp(flag, "close") == 0) {
            opts->close_tasks = true;
            break;
        } else if (strcmp(flag, "reopen") == 0) {
            opts->reopen_tasks = true;
            break;
        } else if (strcmp(flag, "init") == 0) {
            opts->init_dir = true;
            if (argc > 0) {
                flag = shift(argv, argc);
                if (strcmp(flag, "y") == 0 || strcmp(flag, "Y") == 0) {
                    opts->force_init_dir = true;
                }
            }
            break;
        } else if (strcmp(flag, "overwrite") == 0) {
            opts->overwrite_task = calloc(1, sizeof(task_info_t));
            assert(opts->overwrite_task != NULL && "Failed to allocated space for overwrite's task info structure");

            if (argc <= 0) {
                usage(stderr, *program_name);
                exit(1);
            }

            while (argc > 0) {
                flag = shift(argv, argc);
                if (argc > 0) {
                    if (strcmp(flag, "-t") == 0) {
                        opts->overwrite_task->tags = shift(argv, argc);
                    } else if (strcmp(flag, "-p") == 0) {
                        opts->overwrite_task->priority = shift(argv, argc);
                    } else if (strcmp(flag, "-s") == 0) {
                        opts->overwrite_task->status = cstr_to_task_status(shift(argv, argc));
                    } else if (strcmp(flag, "-r") == 0) {
                        opts->overwrite_task->title = shift(argv, argc);
                    }
                } else {
                    // TASK(20260910-201839): allow `close`, `reopen`, `overwrite` and `rm` to "use" the query language
                    da_append(opts->overwrite_task, flag);
                }
            }
            break;
        }
    }

    if (argc) {
        while (argc) {
            char *filter = shift(argv, argc);
            da_append(&opts->filters, filter);
        }
    }

    if (opts->help) {
        usage(stderr, *program_name);
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
    u64 size = sizeof("YYYYMMSS-HHMMSS");
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
    if (strcmp(cstr, "CLOSED") == 0 || strcmp(cstr, "C") == 0)  return STATUS_CLOSED;
    else if (strcmp(cstr, "OPEN") == 0 || strcmp(cstr, "O") == 0) return STATUS_OPEN;
    else if (strcmp(cstr, "NONE") == 0 || strcmp(cstr, "N") == 0) return STATUS_NONE;
    UNREACHABLE("task_status");
}

const char *task_status_to_cstr(task_status status)
{
    switch (status) {
        case STATUS_CLOSED:
            return "CLOSED";
        case STATUS_OPEN:
            return "OPEN";
        case STATUS_NONE:
            return "NONE";
    }
    UNREACHABLE("task_status");
}

char *tasks_path_search(const char *cwd)
{
    File_Paths paths = {0};
    char *result = NULL;

    if (!read_entire_dir(cwd, &paths)) return NULL;

    da_foreach (const char *, path, &paths) {
        const char *full_path = temp_sprintf("%s/%s", cwd, *path);
        Nob_File_Type ft = nob_get_file_type(full_path);

        if (ft == NOB_FILE_DIRECTORY && strstr(*path, "tasks") != NULL) {
            result = temp_sprintf("%s", full_path);
        }

        if (result != NULL) break;
    }

    free(paths.items);
    return result;
}

char *recursive_descend_tasks_path_search(const char *cwd, int depth, const char *excluded_path)
{
    File_Paths paths = {0};
    char *result = NULL;

    // Search for the current directories inside the cwd
    result = tasks_path_search(cwd);

    if (result == NULL && depth > 0) {
        if (!read_entire_dir(cwd, &paths)) return_defer(NULL);

        da_foreach (const char *, path, &paths) {
            const char *full_path = temp_sprintf("%s/%s", cwd, *path);
            if (excluded_path != NULL && strcmp(full_path, excluded_path) == 0) {
                continue;
            }

            Nob_File_Type ft = nob_get_file_type(full_path);
            if (ft == NOB_FILE_DIRECTORY && *path[0] != '.') { // Exclude '.', '..', '.git', etc
                if ((result = recursive_descend_tasks_path_search(full_path, depth-1, excluded_path)) != NULL) break;
            }
        }
    }

defer:
    return result;
}

char *get_parent_dir(const char *cwd)
{
    String_View new_cwd = sv_from_cstr(cwd);
    while (new_cwd.items[new_cwd.count - 1] != '/') {
        sv_chop_right(&new_cwd, 1);
    }
    sv_chop_right(&new_cwd, 1);

    return strdup(temp_sv_to_cstr(new_cwd));
}

char *find_tasks_dir(const char *cwd)
{
    // Do an initial downward search from the cwd
    char *result = recursive_descend_tasks_path_search(cwd, 2, NULL);

    // If no tasks folder is found, go up
    // cwd: root/src/
    // parent: root/
    // inside parent: root/build, root/tasks, root/resources, root/src, ...
    //                                ^
    //                                We search for that directory
    //
    // That search goes up by at most 2 levels up. We won't try to search higher.
    if (result == NULL) {
        char *cwd_parent_dir = get_parent_dir(cwd);
        result = recursive_descend_tasks_path_search(cwd_parent_dir, 3, cwd);

        if (result == NULL) {
            char *parent_parent_dir = get_parent_dir(cwd_parent_dir);
            result = recursive_descend_tasks_path_search(parent_parent_dir, 4, parent_parent_dir);
            free(parent_parent_dir);
        }
        free(cwd_parent_dir);
    }

    if (result != NULL)
        return strdup(result);
    return result;
}

void print_tool_version(const char *program_name)
{
    nob_log(INFO, "%s - Task Tracker implemented by UnknownRedFoxDev", program_name);
#ifndef GIT_HASH
    nob_log(WARNING, "Git version is not defined. Please recompile the latest version.");
#else
    nob_log(INFO, "Git version: %s", GIT_HASH);
#endif // GIT_HASH
#ifndef COMPILER_VERSION
    nob_log(WARNING, "Compiler version is not defined. Please recompile the latest version.");
#else
    nob_log(INFO, "Compiler: %s", COMPILER_VERSION);
#endif // COMPILER_VERSION
#ifndef COMPILE_DATE
    nob_log(WARNING, "Compile date is not defined. Please recompile the latest version.");
#else
    nob_log(INFO, "Last compiled: %s", COMPILE_DATE);
#endif // COMPILE_DATE
}

