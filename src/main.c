#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"

char *tasks_path_search(const char *cwd)
{
    File_Paths paths = {0};
    char *result = NULL;

    if (!read_entire_dir(cwd, &paths)) return NULL;

    da_foreach (const char *, path, &paths) {
        const char *full_path = temp_sprintf("%s/%s", cwd, *path);
        Nob_File_Type ft = nob_get_file_type(full_path);

        if (ft == NOB_FILE_DIRECTORY && strstr(*path, "tasks") != NULL) {
            result = temp_sprintf("%s/", full_path);
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

    return strdup(result);
}

int main(int argc, char **argv)
{
    cmdline_opts opts = {0};
    tasks_t tasks = {0};
    int result = 0;
    parse_options(argc, argv, &opts);

    const char *cwd = get_current_dir_temp();
    char *tasks_dir = find_tasks_dir(cwd);
    if (tasks_dir == NULL) {
        nob_log(ERROR, "Failed to locate tasks folder");
        exit(1);
    }

    minimal_log_level = ERROR;
    mkdir_if_not_exists(tasks_dir);
#ifdef DEBUG
    minimal_log_level = NOB_DEBUG;
#else
    minimal_log_level = INFO;
#endif // DEBUG

    if (!parse_tasks(tasks_dir, &tasks)) return_defer(1);

    if (opts.list_tasks || opts.list_tasks_reversed) {
        print_tasks(&tasks, &opts.filters, opts.list_tasks_reversed);
    } else if (opts.create_task) {
        task_t *task = create_task(tasks_dir, opts.create_task, &opts);
        if (task->path != NULL) {
            open_task(task);
        }
        free_task(task);
    } else if (opts.summary) {
        task_summary();
    } else if (opts.edit_task) {
        task_t *task = find_task(&tasks, opts.edit_task);
        if (!task) return_defer(1);
        open_task(task);
    } else if (opts.find_task) {
        task_t *task = find_task(&tasks, opts.find_task);
        if (!task) return_defer(1);
        print_task(stdout, task);
    } else if (opts.remove_tasks) {
        remove_tasks(&tasks, &opts.filters);
    } else if (opts.close_tasks) {
        close_tasks(&tasks, &opts.filters);
    }

defer:
    free_tasks(&tasks);
    free(tasks_dir);
    return result;
}
