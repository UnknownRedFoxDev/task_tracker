#define HT_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"
#include "../lib/levenshtein.h"

static Ht(const char*, int) __g_stats = { .hasheq = ht_cstr_hasheq };

task_t *find_task(tasks_t *tasks, const char *uuid)
{
    task_t *result = NULL;
    da_foreach (task_t, task, tasks) {
        if (strcmp(task->uuid, uuid) == 0) {
            result = task;
            break;
        }
    }

    if (result == NULL)
        nob_log(WARNING, "TASK(%s) was not found", uuid);
    return result;
}

bool remove_task(task_t *task)
{
    if (!delete_directory_recursively(temp_sprintf("%s%s", task->path, task->uuid))) return false;
    return true;
}

bool remove_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid)
{
    da_foreach (task_t, task, tasks) {
        for (size_t i = 0; i < tasks_uuid->count; ++i) {
            if (strcmp(task->uuid, tasks_uuid->items[i]) == 0) {
                if (!remove_task(task)) return false;
                da_remove_unordered(tasks_uuid, i);
                break;
            }
        }
    }
    return true;
}

bool close_task(task_t *task)
{
    const char *task_md_path = temp_sprintf("%s%s/TASK.md", task->path, task->uuid);
    String_Builder sb = {0};
    String_Builder temp_sb = {0};
    bool result = true;

    if (!read_entire_file(task_md_path, &sb)) return_defer(false);

    size_t ite = 0;
    while (sb.items[ite++] != '\n'); // # <title>\n
    ite += 1; // \n
    while (sb.items[ite++] != '\n'); // # - STATUS: OPEN\n

    for (size_t i = ite; i < sb.count; ++i) {
        sb_append(&temp_sb, sb.items[i]);
    }

    sb.count -= sb.count - ite + 5;

    sb_appendf(&sb, "CLOSED\n");
    sb_append_buf(&sb, temp_sb.items, temp_sb.count);

    if (!write_entire_file(task_md_path, sb.items, sb.count)) return_defer(false);

defer:
    free(sb.items);
    free(temp_sb.items);

    return result;
}

bool close_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid)
{
    da_foreach (task_t, task, tasks) {
        for (size_t i = 0; i < tasks_uuid->count; ++i) {
            if (strcmp(task->uuid, tasks_uuid->items[i]) == 0) {
                if (!close_task(task)) return false;
                da_remove_unordered(tasks_uuid, i);
                break;
            }
        }
    }
    return true;
}

bool open_task(task_t *task)
{
    Cmd cmd = {0};
    bool result = true;
    const char *editor = getenv("EDITOR");
    if (editor != NULL) {
        cmd_append(&cmd, editor);
    } else {
        cmd_append(&cmd, "vim");
    }
    cmd_append(&cmd, temp_sprintf("%s%s/TASK.md", task->path, task->uuid));
    if (!cmd_run(&cmd)) result = false;

    free(cmd.items);
    return result;
}

void print_task(FILE *stream, task_t *task)
{
    String_Builder sb = {0};
    sb_appendf(&sb, "%s%s/TASK.md%s:%s1%s: ", COLOR_RED, task->uuid, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
    sb_appendf(&sb, "[PRIORITY: %-2d ", task->priority);
    if (task->tags.count) {
        sb_appendf(&sb, ", TAGS: ");
        ht_foreach (val, &task->tags) {
            const char *key = ht_key(&task->tags, val);
            if (strcmp(key, "OPEN") && strcmp(key, "CLOSED") && strcmp(key, "UNTAGGED"))
                sb_appendf(&sb, "%s,", key);
        }
        sb.items[sb.count-1] = ']';
    }
    sb_appendf(&sb, " %s\n", task->name);
sb_append_null(&sb);
    fprintf(stream, "%s", sb.items);
    free(sb.items);
    sb.items = NULL;
}

struct keyval {
    const char *key;
    int value;
};

int cmp_keyval(const struct keyval *t1, const struct keyval *t2)
{
    return t2->value - t1->value;
}

int cmp_keyval_void(const void *t1, const void *t2)
{
    return cmp_keyval((const struct keyval *)t1, (const struct keyval *)t2);
}

void task_summary()
{
    nob_log(INFO, "Summary of tasks:");
    printf("OPEN:     %2d\n", *ht_find(&__g_stats, "OPEN"));
    printf("CLOSED:   %2d\n", *ht_find(&__g_stats, "CLOSED"));
    printf("TOTAL:    %2d\n", *ht_find(&__g_stats, "TOTAL"));
    printf("UNTAGGED: %2d\n", *ht_find(&__g_stats, "UNTAGGED"));
    printf("TAGGED:\n");

    int longest_tag_name = 0;

    ht_foreach(value, &__g_stats) {
        const char *key = ht_key(&__g_stats, value);
        if (strcmp(key, "OPEN") && strcmp(key, "UNTAGGED")) {
            int len = strlen(key);
            if (len > longest_tag_name) longest_tag_name = len;
        }
    }

    struct keyval *ordered_list = calloc(__g_stats.count, sizeof(struct keyval));
    size_t ite = 0;
    ht_foreach(value, &__g_stats) {
        const char *key = ht_key(&__g_stats, value);
        if (strcmp(key, "OPEN") && strcmp(key, "CLOSED") && strcmp(key, "UNTAGGED") && strcmp(key, "TOTAL")) {
            ordered_list[ite].key = key;
            ordered_list[ite++].value = *value;
        }
    }

    qsort(ordered_list, __g_stats.count, sizeof(struct keyval), cmp_keyval_void);

    for (size_t i = 0; i < ite; ++i) {
        printf("    %*s => %d\n", longest_tag_name, ordered_list[i].key, ordered_list[i].value);
    }
}

struct task_distance {
    size_t dist;
    task_t *task;
};

int cmp_tasks(const task_t *t1, const task_t *t2)
{
    return t2->priority - t1->priority;
}

int cmp_tasks_void(const void *t1, const void *t2)
{
    return cmp_tasks((const task_t *)t1, (const task_t *)t2);
}

int cmp_tasks_a(const struct task_distance *t1, const struct task_distance *t2)
{
    return t2->task->priority - t1->task->priority;
}

int cmp_tasks_void_a(const void *t1, const void *t2)
{
    return cmp_tasks_a((const struct task_distance *)t1, (const struct task_distance *)t2);
}

int cmp_tasks_dist(const struct task_distance *t1, const struct task_distance *t2)
{
    return t2->dist - t1->dist;
}

int cmp_tasks_dist_void(const void *t1, const void *t2)
{
    return cmp_tasks_dist((const struct task_distance *)t1, (const struct task_distance *)t2);
}

typedef enum {
    NONE,
    NOT,
    OR,
    AND
} boolean_keywords;

const char *boolean_keyword_to_string(boolean_keywords key)
{
    switch (key) {
        case NONE: return "none";
        case NOT: return "not";
        case OR: return "or";
        case AND: return "and";
        default:
            UNREACHABLE("boolean_keyword");
    }
}

String_View get_next_token(String_View *sv)
{
    String_View result = sv_chop_by_delim(sv, ' ');
    if (sv_starts_with(result, sv_from_cstr("."))) sv_chop_left(&result, 1);
    return result;
}

/*
 * 20260621-110728:
 * When doing `not <tag>` or `<tag1> and <tag2>` or `<tag1> or <tag2>`, I'm not comparing tags, I'm comparing lists created from the tags and filtering the remainder
 * so when `<tag1> and (<tag2> or <tag3>)` I should be comparing list of tag1, with the last made of at least tag2 or tag3
 */
task_t *eval_tokens(const tasks_t *tasks, String_View *tokens)
{
    task_t *result = calloc(tasks->count, sizeof(task_t));

    int parentheses_count = 0;
    String_View prev_token = {0};
    String_View curr_token = {0};
    String_View next_token = {0};
    tasks_t temp_list = {0};
    boolean_keywords curr_modif = NONE;
    curr_modif = NONE;

    nob_log(INFO, SV_Fmt, SV_Arg(*tokens));

    while (tokens->count) {
        nob_log(INFO, "-------------------------");
        nob_log(INFO, "prev: "SV_Fmt ", curr: "SV_Fmt ", next: "SV_Fmt, SV_Arg(prev_token), SV_Arg(curr_token), SV_Arg(next_token));
        nob_log(INFO, "before modif: %s", boolean_keyword_to_string(curr_modif));

        nob_log(INFO, "prev: "SV_Fmt ", curr: "SV_Fmt ", next: "SV_Fmt, SV_Arg(prev_token), SV_Arg(curr_token), SV_Arg(next_token));
        nob_log(INFO, "after modif: %s", boolean_keyword_to_string(curr_modif));
        nob_log(INFO, "-------------------------");

        da_foreach (task_t, task, tasks) {
            switch (curr_modif) {
            case NONE: {
                TODO("none");
                // if (ht_find(&task->tags, temp_sv_to_cstr(curr_token)) != NULL) {
                // }
            } break;
            case NOT: {
                TODO("not");
                // if (ht_find(&task->tags, temp_sv_to_cstr(next_token)) == NULL) {
                //     da_append(&temp_list, *task);
                // }
            } break;
            case OR: {
                TODO("or");
                // if (ht_find(&task->tags, temp_sv_to_cstr(next_token)) != NULL ||
                //     ht_find(&task->tags, temp_sv_to_cstr(prev_token)) != NULL) {
                //     da_append(&temp_list, *task);
                // }
            } break;
            case AND: {
                TODO("and");
                // if (ht_find(&task->tags, temp_sv_to_cstr(next_token)) != NULL &&
                //     ht_find(&task->tags, temp_sv_to_cstr(prev_token)) != NULL) {
                //     da_append(&temp_list, *task);
                // }
            } break;
            default:
                UNREACHABLE("boolean_keywords: curr_token in print_tasks()");
            }
        }
        prev_token = next_token;
    }

    if (parentheses_count != 0) {
        nob_log(ERROR, "failed to parse filters, uneven amount of paratheses");
        return NULL;
    }

    return result;
}

bool print_tasks(const tasks_t *tasks, Flag_List_Mut *tokens)
{
    // TODO: rewrite it to take into account the new nomenclature: .<tag>, and, or, not
    //       "pre-defined tags: .OPEN, .CLOSED (not .OPEN), .TAGGED, .UNTAGGED (not .TAGGED)
    //       "by default: .OPEN and (.TAGGED or .UNTAGGED)

    String_View sv = {0};
    String_Builder sb = {0};
    bool ignore_default = false;
    bool result = true;

    if (tokens->count > 0) {
        sv = sv_from_cstr(tokens->items[0]);

        String_View temp_sv = sv_from_cstr(tokens->items[0]);
        while (temp_sv.count) {
            String_View tok = sv_chop_by_delim(&temp_sv, ' ');
            if (sv_eq(tok, sv_from_cstr(".CLOSED"))) {
                ignore_default = true;
                break;
            }
        }
    }

    if (sv.count == 0 || !ignore_default) {
        nob_log(INFO, "loading default");
        sb_appendf(&sb, ".OPEN");
        if (sv.count > 0) sb_appendf(&sb, " and ");
        sb_append_sv(&sb, sv);
        sv = sb_to_sv(sb);
    }

    // Size of tasks->count; The list may contain holes, or be incomplete due to the filtering
    task_t *list = eval_tokens(tasks, &sv);
    if (!list) return_defer(false);

defer:
    qsort(list, tasks->count, sizeof(task_t), cmp_tasks_void);
    for (size_t i = 0; i < tasks->count; ++i) {
        print_task(stdout, &list[i]);
    }

    if (result) {
        free(list);
    }

    free(sb.items);
    return result;
#if 0
    return result;
    struct task_distance *distances = calloc(tasks->count, sizeof(struct task_distance));
    bool exclude_opened_tasks = false;
    bool all = false;

    for (size_t i = 0; i < filters->count; ++i) {
        if (strcmp(filters->items[i], "CLOSED") == 0) {
            exclude_opened_tasks = true;
        } if (strcmp(filters->items[i], "all") == 0) {
            da_remove_unordered(filters, i);
            if (filters->count == 0)
                da_append(filters, "");
            all = true;
        }
    }

    size_t min_dist = 100;
    for (size_t i = 0; i < tasks->count; ++i) {
        task_t curr_task = tasks->items[i];
        size_t best_dist = 100;

        if (all || (exclude_opened_tasks && curr_task.status == CLOSED) || (!exclude_opened_tasks && curr_task.status == OPEN)) {
            for (size_t j = 0; j < filters->count && best_dist; ++j) {
                // TAGS
                for (size_t k = 0; k < curr_task.tags.count && best_dist; ++k) {
                    if (strstr(curr_task.tags.items[k], filters->items[j])) {
                        best_dist = 0;
                    }
                }
                if (best_dist < min_dist)
                    min_dist = best_dist;
                if (best_dist == 0) continue;

                // STATUS
                if (strstr(filters->items[j], task_status_to_cstr(curr_task.status))) {
                    best_dist = 5;
                }
                if (best_dist < min_dist)
                    min_dist = best_dist;

                // NAME
                size_t dist_name = levenshtein(filters->items[j], curr_task.name);
                if (strstr(curr_task.name, filters->items[j])) {
                    best_dist = 0;
                } else if (dist_name < best_dist && dist_name != strlen(curr_task.name) && dist_name < strlen(curr_task.name)) {
                    best_dist = dist_name;
                }
                if (best_dist < min_dist)
                    min_dist = best_dist;
            }
        }
        distances[i].dist = best_dist;
        distances[i].task = &tasks->items[i];
    }

    if (strcmp(filters->items[0], "") == 0) {
        qsort(distances, tasks->count, sizeof(struct task_distance), cmp_tasks_void);
    } else {
        qsort(distances, tasks->count, sizeof(struct task_distance), cmp_tasks_dist_void);
    }

    if (min_dist > DISTANCE_THRESHOLD)
        min_dist = DISTANCE_THRESHOLD;

    for (size_t i = 0; i < tasks->count; ++i) {
        if (distances[i].dist == min_dist) {
            print_task(stdout, distances[i].task);
        }
    }
    free(distances);
#endif
}


task_t *create_task(const char *path, const char *task_name)
{
    String_Builder sb = {0};
    task_t *result = calloc(1, sizeof(task_t));
    if (result == NULL) {
        nob_log(ERROR, "Failed to calloc a task_t");
        return NULL;
    }

    sb_appendf(&sb, "# %s\n", task_name);
    sb_appendf(&sb, "\n");
    sb_appendf(&sb, "- STATUS: OPEN\n");
    sb_appendf(&sb, "- PRIORITY: 1\n");
    sb_appendf(&sb, "- TAGS: \n\n");

    char *dir_name = get_timestamp_uuid();
    const char *task_path = temp_sprintf("%s%s", path, dir_name);
    const char *task_md = temp_sprintf("%s/TASK.md", task_path);

    minimal_log_level = ERROR;
    if (!mkdir_if_not_exists(task_path)) goto defer;
    minimal_log_level = INFO;
    if (!write_entire_file(task_md, sb.items, sb.count)) goto defer;

    result->name = strdup(task_name);
    result->path = strdup(path);
    result->uuid = strdup(dir_name);
    result->priority = 1;
    result->status = OPEN;
    result->tags.hasheq = ht_cstr_hasheq;

    nob_log(INFO, "Created task at: %s%s/TASK.md%s", COLOR_RED, task_path, COLOR_RESET);

defer:
    free(sb.items);
    free(dir_name);
    return result;
}

bool parse_task(const char *path, const char *uuid, task_t *task)
{
    String_Builder sb = {0};
    String_View sv = {0};
    bool result = true;

    if (!read_entire_file(temp_sprintf("%s%s/TASK.md", path, uuid), &sb)) return_defer(false);
    sv = sb_to_sv(sb);

    task->path = strdup(path);
    task->uuid = strdup(uuid);
    task->tags.hasheq = ht_cstr_hasheq;

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
    *ht_find_or_put(&__g_stats, cstatus) += 1;
    *ht_put(&task->tags, cstatus) = true;
    task->status = cstr_to_task_status(cstatus);

    // - PRIORITY: UINT
    String_View priority = sv_chop_by_delim(&sv, '\n');
    sv_chop_prefix(&priority, sv_from_cstr("- PRIORITY: "));
    task->priority = temp_sv_to_int(priority);

    // - TAGS: <tag1>,<tag2>,<tag3>,...
    String_View tags_line = sv_chop_by_delim(&sv, '\n');
    if (sv_chop_prefix(&tags_line, sv_from_cstr("- TAGS: "))) {
        while (tags_line.count) {
            char *tag = sv_to_cstr(sv_chop_by_delim(&tags_line, ','));
            *ht_find_or_put(&__g_stats, tag) += 1;
            *ht_put(&task->tags, tag) = true;
        }
    } else {
        *ht_find_or_put(&__g_stats, "UNTAGGED") += 1;
        *ht_put(&task->tags, "UNTAGGED") = true;
    }

    *ht_find_or_put(&__g_stats, "TOTAL") += 1;
defer:
    free(sb.items);
    return result;
}

bool parse_tasks(const char *path, tasks_t *tasks)
{
    File_Paths tasks_uuid = {0};
    read_entire_dir(path, &tasks_uuid);
    *ht_put(&__g_stats, "OPEN") = 0;
    *ht_put(&__g_stats, "CLOSED") = 0;
    *ht_put(&__g_stats, "TOTAL") = 0;
    *ht_put(&__g_stats, "UNTAGGED") = 0;

    da_foreach (const char *, uuid, &tasks_uuid) {
        if (!sv_starts_with(sv_from_cstr(*uuid), sv_from_cstr("."))) {
            task_t task = {0};
            parse_task(path, *uuid, &task);
            da_append(tasks, task);
        }
    }

    return true;
}

void free_task(task_t *task)
{
    free(task->name);
    free(task->uuid);
    free(task->path);
}

void free_tasks(tasks_t *tasks)
{
    da_foreach (task_t, task, tasks) {
        free_task(task);
    }
    free(tasks->items);
}
