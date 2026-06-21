#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/helper.h"
#include "../lib/task.h"

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
        if (!create_task(tasks_folder, opts.create_task)) return 1;
    } else if (opts.summary) {
        task_summary();
    }

    free_tasks(&tasks);
    return 0;
}
