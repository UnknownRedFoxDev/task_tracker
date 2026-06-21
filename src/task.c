#include "../lib/task.h"
#include "../lib/helper.h"
#include "../lib/levenshtein.h"

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
    fprintf(stream, "] %s\n", task->name);
}

struct foo{
    size_t dist;
    task_t *task;
};

int cmp_tasks(const struct foo *t1, const struct foo *t2)
{
    return t2->task->priority - t1->task->priority;
}

int cmp_tasks_void(const void *t1, const void *t2)
{
    return cmp_tasks((const struct foo *)t1, (const struct foo *)t2);
}

int cmp_tasks_dist(const struct foo *t1, const struct foo *t2)
{
    return t2->dist - t1->dist;
}

int cmp_tasks_dist_void(const void *t1, const void *t2)
{
    return cmp_tasks_dist((const struct foo *)t1, (const struct foo *)t2);
}

void print_tasks(const tasks_t *tasks, Flag_List_Mut *filters)
{
    struct foo *distances = calloc(tasks->count, sizeof(struct foo));

    for (size_t i = 0; i < tasks->count; ++i) {
        task_t curr_task = tasks->items[i];
        size_t best_dist = 100;

        for (size_t j = 0; j < filters->count && best_dist; ++j) {
            // TAGS
            for (size_t k = 0; k < curr_task.tags.count && best_dist; ++k) {
                if (strstr(curr_task.tags.items[k], filters->items[j])) {
                    best_dist = 0;
                }
            }
            if (best_dist == 0) continue;

            // STATUS
            if (strstr(filters->items[j], task_status_to_cstr(curr_task.status))) {
                best_dist = 5;
            }

            // NAME
            size_t dist_name = levenshtein(filters->items[j], curr_task.name);
            if (strstr(curr_task.name, filters->items[j])) {
                best_dist = 0;
            } else if (dist_name < best_dist && dist_name != strlen(curr_task.name) && dist_name < strlen(curr_task.name)) {
                best_dist = dist_name;
            }
        }
        distances[i].dist = best_dist;
        distances[i].task = &tasks->items[i];
    }

    if (strcmp(filters->items[0], "") == 0) {
        qsort(distances, tasks->count, sizeof(struct foo), cmp_tasks_void);
    } else {
        qsort(distances, tasks->count, sizeof(struct foo), cmp_tasks_dist_void);
    }

    for (size_t i = 0; i < tasks->count; ++i) {
        print_task(stdout, distances[i].task);
    }
    free(distances);}


bool create_task(const char *path, const char *task_name)
{
    String_Builder sb = {0};
    bool result = true;

    sb_appendf(&sb, "# %s\n", task_name);
    sb_appendf(&sb, "\n");
    sb_appendf(&sb, "- STATUS: OPEN\n");
    sb_appendf(&sb, "- PRIORITY: 1\n");
    sb_appendf(&sb, "- TAGS: \n\n");

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

    // - TAGS: <str1>,<str2>,...
    String_View tags_line = sv_chop_by_delim(&sv, '\n');
    // It is possible that the "- TAGS: " becomes "- TAGS:" without a space at the end
    // leading to the prefix not being chopped. Ultimately, saying: "task is untagged"
    if (sv_chop_prefix(&tags_line, sv_from_cstr("- TAGS: "))) {
        while (tags_line.count) {
            char *tag = sv_to_cstr(sv_chop_by_delim(&tags_line, ','));
            da_append(&task->tags, tag);
        }
    }

defer:
    free(sb.items);
    return result;
}

void free_task(task_t *task)
{
    free(task->name);
}

bool parse_tasks(const char *path, tasks_t *tasks)
{
    File_Paths tasks_uuid = {0};
    read_entire_dir(path, &tasks_uuid);

    da_foreach (const char *, uuid, &tasks_uuid) {
        if (!sv_starts_with(sv_from_cstr(*uuid), sv_from_cstr("."))) {
            task_t task = {0};
            if (!parse_task(path, *uuid, &task)) return false;
            da_append(tasks, task);
        }
    }

    return true;
}

void free_tasks(tasks_t *tasks)
{
    da_foreach (task_t, task, tasks) {
        free_task(task);
    }
    free(tasks->items);
}
