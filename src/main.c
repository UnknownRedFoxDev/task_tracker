#define FLAG_IMPLEMENTATION
#define NOB_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"
#include "../lib/parser.h"

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

    if (opts.init_dir) {
        init_directory(tasks_dir, opts.force_init_dir);
        goto defer;
    }

    initialise_tasks();
    if (!parse_tasks(tasks_dir, &tasks, NULL, NULL)) return_defer(1);


    String_Builder sb = {0};
    for (size_t i = 0; i < opts.filters.count; ++i) {
        sb_appendf(&sb, "%s%s", opts.filters.items[i], (i+1 == opts.filters.count)? "" : " ");
    }
    sb_append_null(&sb);
    printf("%s\n", sb.items);

    Lexer *lexer = init_lexer(sb.items);
    Parser *state = init_parser(lexer);

    Node_t *ast = parse_query(state);
    dump_ast(ast);

    clean_ast(ast);
    clean_parser(&state);

    // if (opts.list_tasks || opts.list_tasks_reversed) {
    //     print_tasks(&tasks, &opts.filters, opts.list_tasks_reversed);
    // }
    // else if (opts.create_task) {
    //     task_t *task = create_task(tasks_dir, opts.create_task, opts.no_editor);
    //     if (task->path != NULL && !opts.no_editor) {
    //         open_task(task);
    //     }
    //     free_task(task);
    // }
    // else if (opts.summary) {
    //     task_summary();
    // }
    // else if (opts.cat_task) {
    //     task_t *task = find_task(&tasks, opts.cat_task);
    //     if (!task) return_defer(1);
    //     cat_task(task);
    // }
    // else if (opts.edit_task) {
    //     task_t *task = find_task(&tasks, opts.edit_task);
    //     if (!task) return_defer(1);
    //     open_task(task);
    // }
    // else if (opts.find_task) {
    //     task_t *task = find_task(&tasks, opts.find_task);
    //     if (!task) return_defer(1);
    //     print_task(stdout, task, DEFAULT_ALIGNMENT);
    // }
    // else if (opts.remove_tasks) {
    //     remove_tasks(&tasks, &opts.filters);
    // }
    // else if (opts.close_tasks) {
    //     change_tasks_status(&tasks, &opts.filters, STATUS_CLOSED);
    // }
    // else if (opts.reopen_tasks) {
    //     change_tasks_status(&tasks, &opts.filters, STATUS_OPEN);
    // }
    // else if (opts.overwrite_task) {
    //     overwrite_task(&tasks, opts.overwrite_task);
    // }

defer:
    free_tasks(&tasks);
    free(opts.filters.items);
    free(tasks_dir);
    return result;
}
