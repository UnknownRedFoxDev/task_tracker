#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"

int main(int argc, char **argv)
{
    cmdline_opts opts = {0};
    tasks_t tasks = {0};
    parse_options(argc, argv, &opts);

    const char *tasks_folder = temp_sprintf("%s/tasks/", get_current_dir_temp());
    // const char *tasks_folder = "./tasks/";
    minimal_log_level = ERROR;
    mkdir_if_not_exists(tasks_folder);
    minimal_log_level = INFO;

    if (!parse_tasks(tasks_folder, &tasks)) return 1;

    if (opts.list_tasks) {
        print_tasks(&tasks, &opts.filters);
    } else if (opts.create_task) {
        if (!create_task(tasks_folder, opts.create_task)) return 1;
        const char *editor = getenv("EDITOR");
        minimal_log_level = ERROR;
        Cmd cmd = {0};
        if (editor != NULL) {
            cmd_append(&cmd, editor, tasks.items[tasks.count-1].path);
        } else { // default to vim
            cmd_append(&cmd, "vim", tasks.items[tasks.count-1].path);
        }
        if (!cmd_run(&cmd)) return 1;
        minimal_log_level = INFO;
    } else if (opts.summary) {
        task_summary();
    }

    free_tasks(&tasks);
    return 0;
}
