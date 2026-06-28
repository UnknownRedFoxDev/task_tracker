#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"

const char *find_tasks_dir()
{
    const char *cwd = get_current_dir_temp();
    File_Paths paths = {0};

    if (!read_entire_dir(cwd, &paths)) return NULL;

    da_foreach (const char *, path, &paths) {
        Nob_File_Type ft = nob_get_file_type(*path);
        if (strstr(*path, "tasks") != NULL) {
            return temp_sprintf("./%s/", *path);
        } else if (ft == NOB_FILE_DIRECTORY && *path[0] != '.') { // Exclude '.', '..', '.git', etc
            File_Paths childrens = {0};
            if (!read_entire_dir(*path, &childrens)) return NULL;
            da_foreach(const char *, sub_path, &childrens) {
                if (strstr(*sub_path, "tasks") != NULL) {
                    return temp_sprintf("./%s/%s/", *path, *sub_path);
                }
            }
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    cmdline_opts opts = {0};
    tasks_t tasks = {0};
    int result = 0;
    parse_options(argc, argv, &opts);

    const char *tasks_folder = find_tasks_dir();
    minimal_log_level = ERROR;
    mkdir_if_not_exists(tasks_folder);
#ifdef DEBUG
    minimal_log_level = NOB_DEBUG;
#else
    minimal_log_level = INFO;
#endif // DEBUG

    if (!parse_tasks(tasks_folder, &tasks)) return_defer(1);

    if (opts.list_tasks || opts.list_tasks_reversed) {
        print_tasks(&tasks, &opts.filters, opts.list_tasks_reversed);
    } else if (opts.create_task) {
        task_t *task = create_task(tasks_folder, opts.create_task, &opts);
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
    return result;
}
