#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"

int main(int argc, char **argv)
{
    cmdline_opts_t opts = {0};
    tasks_t tasks = {0};
    int result = 0; parse_options(argc, argv, &opts);

    const char *cwd = get_current_dir_temp();
    char *tasks_dir = find_tasks_dir(cwd);
    if (tasks_dir == NULL && !opts.init_dir) {
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
    }
    else if (opts.create_task) {
        task_t *task = create_task(tasks_dir, opts.create_task, &opts);
        if (task->path != NULL) {
            open_task(task);
        }
        free_task(task);
    }
    else if (opts.summary) {
        task_summary();
    }
    else if (opts.cat_task) {
        task_t *task = find_task(&tasks, opts.cat_task);
        if (!task) return_defer(1);
        cat_task(task);
    }
    else if (opts.edit_task) {
        task_t *task = find_task(&tasks, opts.edit_task);
        if (!task) return_defer(1);
        open_task(task);
    }
    else if (opts.find_task) {
        task_t *task = find_task(&tasks, opts.find_task);
        if (!task) return_defer(1);
        print_task(stdout, task);
    }
    else if (opts.remove_tasks) {
        remove_tasks(&tasks, &opts.filters);
    }
    else if (opts.close_tasks) {
        change_tasks_status(&tasks, &opts.filters, CLOSED);
    }
    else if (opts.reopen_tasks) {
        change_tasks_status(&tasks, &opts.filters, OPEN);
    }
    else if (opts.init_dir) {
        init_directory(tasks_dir);
    }

defer:
    free_tasks(&tasks);
    free(opts.filters.items);
    free(tasks_dir);
    return result;
}
