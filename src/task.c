#include <assert.h>
#include <ctype.h>
#include <string.h>
#define HT_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"

static tags_t __g_tags = {0};
static Ht(const char*, int) __g_stats = { .hasheq = ht_cstr_hasheq };

// By HUID
int cmp_tasks_by_huid(const task_t *t1, const task_t *t2)
{
    return strcmp(t1->uuid, t2->uuid);
}

int cmp_tasks_by_huid_reversed(const task_t *t1, const task_t *t2)
{
    return strcmp(t2->uuid, t1->uuid);
}

int cmp_tasks_by_huid_void(const void *t1, const void *t2)
{
    return cmp_tasks_by_huid((const task_t *)t1, (const task_t *)t2);
}

int cmp_tasks_by_huid_reversed_void(const void *t1, const void *t2)
{
    return cmp_tasks_by_huid_reversed((const task_t *)t1, (const task_t *)t2);
}

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

void cat_task(task_t *task)
{
    String_Builder sb = {0};
    if (!read_entire_file(temp_sprintf("%s/%s/TASK.md", task->path, task->uuid), &sb)) return ;
    sb_append_null(&sb);

    printf("%s", sb.items);
    free(sb.items);
}

bool remove_task(task_t *task)
{
    minimal_log_level = ERROR;
    if (!delete_directory_recursively(temp_sprintf("%s/%s", task->path, task->uuid))) return false;
#ifdef DEBUG
    minimal_log_level = DEBUG;
#else
    minimal_log_level = INFO;
#endif // DEBUG

    nob_log(INFO, "Deleted task(%s): %s", task->uuid, task->name);
    return true;
}

bool remove_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid, int last_n)
{
    if (last_n <= 0) {
        da_foreach (task_t, task, tasks) {
            for (u64 i = 0; i < tasks_uuid->count; ++i) {
                if (strcmp(task->uuid, tasks_uuid->items[i]) == 0) {
                    if (!remove_task(task)) return false;
                    da_remove_unordered(tasks_uuid, i);
                    break;
                }
            }
        }
    } else {
        task_t *ordered = calloc(tasks->count, sizeof(task_t));
        if (!ordered) {
            nob_log(ERROR, "remove_tasks(): Failed to create an ordered list of tasks");
            return false;
        }
        int len = 0;
        for (size_t i = 0; i < tasks->count; ++i) {
            // Only delete the recent opened tasks
            if (ht_find(&tasks->items[i].tags, "OPEN")) {
                ordered[len++] = tasks->items[i];
            }
        }

        qsort(ordered, len, sizeof(task_t), cmp_tasks_by_huid_reversed_void);

        for (int i = 0; i < last_n && i < len; ++i) {
            if (!remove_task(&ordered[i])) {
                return false;
            }
        }

        free(ordered);
    }
    return true;
}

bool read_file_until_n_line(const char *path, s32 n, String_Builder *file_buffer_reader, String_Builder *buffer_for_whatever_is_after)
{
    if (!read_entire_file(path, file_buffer_reader)) return false;
    u32 cursor = 0;

    if (file_buffer_reader->items != NULL) {
        for (s32 i = 0; i < n; ++i) {
            while (file_buffer_reader->items[cursor++] != '\n'); // # <title>\n
        }

        for (u32 i = cursor; i < file_buffer_reader->count; ++i) {
            sb_append(buffer_for_whatever_is_after, file_buffer_reader->items[i]);
        }

        file_buffer_reader->count -= file_buffer_reader->count - cursor; // remove the content after the cursor
        if (n > 2 && n < 6) {
            while (file_buffer_reader->items[file_buffer_reader->count--] != ' ');
            file_buffer_reader->count += 2; // STATUS -> STATUS:
        } else if (n == 1) {
            while (file_buffer_reader->items[file_buffer_reader->count--] != '#');
            file_buffer_reader->count += 1; // # -> #_
        }
    } else {
        return false;
    }
    return true;
}

bool change_tasks_status(tasks_t *tasks, Flag_List_Mut *tasks_uuid, task_status new_status)
{
    da_foreach (task_t, task, tasks) {
        for (u64 i = 0; i < tasks_uuid->count; ++i) {
            if (strcmp(task->uuid, tasks_uuid->items[i]) == 0) {
                task_info_t info = {0};
                da_append(&info, task->uuid);
                info.status = new_status;
                if (!overwrite_task(tasks, &info)) return false;
                free(info.items);
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
    cmd_append(&cmd, temp_sprintf("%s/%s/TASK.md", task->path, task->uuid));
    if (!cmd_run(&cmd)) result = false;

    free(cmd.items);
    return result;
}

void print_task(FILE *stream, task_t *task, int alignment)
{
    String_Builder sb = {0};
    sb_appendf(&sb, "%s./tasks/%s/TASK.md%s:%s1%s: ", COLOR_RED, task->uuid, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
    sb_appendf(&sb, "[PRIORITY: %-*zu ", alignment, task->priority);
    size_t tag_count = 0;
    if (!ht_find(&task->tags, "UNTAGGED")) {
        sb_appendf(&sb, ", TAGS: ");
        ht_foreach (val, &task->tags) {
            const char *key = ht_key(&task->tags, val);
            tag_count += 1;
            if (strcmp(key, "OPEN") && strcmp(key, "CLOSED") && strcmp(key, "UNTAGGED"))
                sb_appendf(&sb, "%s%s", key, (tag_count == task->tags.count)? "" : ",");
        }
    }
        sb_append_cstr(&sb, "]");
    sb_appendf(&sb, " %s%s%s\n", COLOR_BOLD, task->name, COLOR_RESET);
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

bool is_special_tag(const char *key)
{
    return !strcmp(key, "OPEN") || !strcmp(key, "CLOSED") || !strcmp(key, "UNTAGGED") || !strcmp(key, "TOTAL");
}

void task_summary(const char *tasks_dir)
{
    parse_tags(tasks_dir);

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
    u64 ite = 0;
    ht_foreach(value, &__g_stats) {
        const char *key = ht_key(&__g_stats, value);
        if (!is_special_tag(key)) {
            ordered_list[ite].key = key;
            ordered_list[ite++].value = *value;
        }
    }

    qsort(ordered_list, __g_stats.count, sizeof(struct keyval), cmp_keyval_void);

    for (u64 i = 0; i < ite; ++i) {
        const char *desc = NULL;
        da_foreach (tag_t, tag, &__g_tags) {
            if (strcmp(tag->name, ordered_list[i].key) == 0) {
                desc = tag->description;
            }
        }
        printf("    %*s => %-3d", longest_tag_name, ordered_list[i].key, ordered_list[i].value);
        if (desc != NULL) {
            printf(" - %s", desc);
        }
        printf("\n");
    }

    free_tags(&__g_tags);
    free(ordered_list);
}

struct task_distance {
    u64 dist;
    task_t *task;
};

// By Task's Priority
int cmp_tasks(const task_t *t1, const task_t *t2)
{
    return t2->priority - t1->priority;
}

int cmp_tasks_void(const void *t1, const void *t2)
{
    return cmp_tasks((const task_t *)t1, (const task_t *)t2);
}

int cmp_tasks_rev(const task_t *t1, const task_t *t2)
{
    return t1->priority - t2->priority;
}

int cmp_tasks_rev_void(const void *t1, const void *t2)
{
    return cmp_tasks_rev((const task_t *)t1, (const task_t *)t2);
}


char *str_to_lower(const char *cstr)
{
    char *result = strdup(cstr);
    u32 i = 0;
    while (result[i]) {
        result[i] = tolower(result[i]);
        i++;
    }
    return result;
}


u32 retrieve_tasks_from_name(const tasks_t *tasks, String_View name, task_t **result)
{
    nob_log(NOB_DEBUG, "Starting filtering by name");

    String_Builder sb = {0};
    sb_appendf(&sb, SV_Fmt, SV_Arg(name));
    char *name_cstr = str_to_lower(sb.items);
    u32 result_ite = 0;

    da_foreach (task_t, task, tasks) {
        char *task_name = str_to_lower(task->name);
        if (strstr(task_name, name_cstr)) {
            result[result_ite++] = task;
        }
        free(task_name);
    }

    free(name_cstr);
    free(sb.items);
    return result_ite;
}

 void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

char *itoa(int a)
{
    char *result = calloc(16, sizeof(char));
    int i = 0;
    do  {
        result[i++] = a % 10 + '0';
    } while ((a /= 10) > 0 && i < 16);
    reverse(result);
    return result;
}

size_t find_best_alignment(task_t *tasks, u32 tasks_len)
{
    size_t alignment = DEFAULT_ALIGNMENT;
    for (u32 i = 0; i < tasks_len; ++i) {
        char *priority_cstr = itoa(tasks[i].priority);
        size_t len = strlen(priority_cstr);
        if (len > alignment) {
            alignment = len;
        }

        free(priority_cstr);
    }
    return alignment;
}

u32 eval_tag(const tasks_t *tasks, task_t **result, char *tag, bool negated)
{
    memset(result, 0, tasks->count * sizeof(task_t *));
    u32 result_ite = 0;

    nob_log(NOB_DEBUG, "-------------------------");
    nob_log(NOB_DEBUG, "tag: %s %s", (negated)? "NOT " : "", tag);

    da_foreach (task_t, task, tasks) {
        bool *found = ht_find(&task->tags, tag);
        if ((!found && negated) || (found && !negated)) {
            if (minimal_log_level == NOB_DEBUG) print_task(stdout, task, DEFAULT_ALIGNMENT);
            result[result_ite++] = task;
        }
    }

    nob_log(NOB_DEBUG, "-------------------------");

    return result_ite;
}

#define case_compare(node_kind, op, tasks, result, result_ite) \
    case (node_kind): { \
        da_foreach (task_t, task, tasks) { \
            if (task->priority op) { \
                result[result_ite++] = task; \
            } \
        } \
    } break

u32 retrieve_tasks_from_query(const tasks_t *tasks, Node_t *root, bool negated, task_t **result)
{
    tag_set ht_tasks_set = { .hasheq = ht_cstr_hasheq };
    u32 result_ite = 0;
    switch (root->kind) {
    case NODE_TAG: {
        result_ite = eval_tag(tasks, result, root->as.tag_name, negated);
        break;
    }
    case NODE_NOT: {
        assert(root->lhs && "not-node's lhs should not be NULL");
        result_ite = retrieve_tasks_from_query(tasks, root->lhs, true, result);
        break;
    }
    case NODE_AND: {
        assert(root->lhs && "and-node's lhs should not be NULL");
        task_t **lhs_result = calloc(tasks->count, sizeof(task_t *));
        if (!lhs_result) {
            nob_log(ERROR, "Failed to allocate space for table of task_t pointers");
            exit(1);
        }

        u32 lhs_ite = 0;
        u32 rhs_ite = 0;
        lhs_ite = retrieve_tasks_from_query(tasks, root->lhs, negated, lhs_result);

        assert(root->lhs && "and-node's rhs should not be NULL");
        rhs_ite = retrieve_tasks_from_query(tasks, root->rhs, negated, result);

        // TASK(20260823-234429): Optimize NODE_AND eval_node() case when accumulating both results array
        for (u32 i = 0; i < lhs_ite; ++i) {
            task_t *lhs_task = lhs_result[i];
            for (u32 j = 0; j < rhs_ite; ++j) {
                task_t *rhs_task = result[j];
                if (lhs_task == rhs_task) {
                    *ht_find_or_put(&ht_tasks_set, rhs_task->uuid) = rhs_task;
                    break;
                }
            }
        }

        // Reset result table
        memset(result, 0, result_ite * sizeof(task_t *));
        result_ite = 0;

        // Fill it back with the result of both lhs and rhs tags
        ht_foreach(val, &ht_tasks_set) {
            result[result_ite++] = *val;
        }

        ht_free(&ht_tasks_set);
        free(lhs_result);
        break;
    }
    case NODE_OR: {
        assert(root->lhs && "and-node's lhs should not be NULL");
        result_ite = retrieve_tasks_from_query(tasks, root->lhs, negated, result);
        for (u32 i = 0; i < result_ite; ++i) {
            *ht_find_or_put(&ht_tasks_set, result[i]->uuid) = result[i];
        }

        assert(root->lhs && "and-node's rhs should not be NULL");
        memset(result, 0, result_ite * sizeof(task_t *));
        result_ite = retrieve_tasks_from_query(tasks, root->rhs, negated, result);
        for (u32 i = 0; i < result_ite; ++i) {
            *ht_find_or_put(&ht_tasks_set, result[i]->uuid) = result[i];
        }

        // Reset result table
        memset(result, 0, result_ite * sizeof(task_t *));
        result_ite = 0;

        // Fill it back with the result of both lhs and rhs tags
        ht_foreach(val, &ht_tasks_set) {
            result[result_ite++] = *val;
        }

        ht_free(&ht_tasks_set);
        break;
    }
    case_compare(NODE_LT,  <  root->rhs->as.integer, tasks, result, result_ite);
    case_compare(NODE_LTE, <= root->rhs->as.integer, tasks, result, result_ite);
    case_compare(NODE_GT,  >  root->rhs->as.integer, tasks, result, result_ite);
    case_compare(NODE_GTE, <= root->rhs->as.integer, tasks, result, result_ite);
    default:
        UNREACHABLE("Node_Kind");
    }

    return result_ite;
}

void args_to_query_string(String_Builder *dst, Flag_List_Mut *src)
{
    for (size_t i = 0; i < src->count; ++i) {
        if (i != 0) {
            sb_append_cstr(dst, " ");
        }
        sb_appendf(dst, "%s", src->items[i]);
    }
}

bool is_task_name(Flag_List_Mut *tokens)
{
    if (tokens->count >= 1) {
        String_View token = sv_from_cstr(tokens->items[0]);
        if (!sv_starts_with(token, SVLIT("."))) {
            return true;
        }
    }

    return false;
}

u32 get_tasks(const tasks_t *tasks, String_View token_str, task_t **list)
{
    String_Builder sb = {0};
    String_View sv = {0};
    bool ignore_default = false;
    u32 result = 0;
    Lexer *l = NULL;
    Parser *s = NULL;
    Node_t *ast = NULL;

    if (token_str.count > 0
        && (strstr(token_str.items, ".CLOSED")
        || strstr(token_str.items, ".OPEN")
        || strstr(token_str.items, ".all"))) {
        ignore_default = true;
    }

    if (!ignore_default) {
        sb_appendf(&sb, ".OPEN");
        if (token_str.count > 0) sb_appendf(&sb, " and ");
        if (token_str.count > 1) sb_appendf(&sb, "[");
    }

    sb_append_buf(&sb, token_str.items, token_str.count);

    if (!ignore_default && token_str.count > 1) {
        sb_appendf(&sb, "]");
    }

    sb_append_null(&sb);
    sv = sb_to_sv(sb);

    l   = init_lexer(sv.items);
    s   = init_parser(l);
    ast = parse_query(s);

    result = retrieve_tasks_from_query(tasks, ast, false, list);

    clean_ast(ast);
    clean_parser(&s);
    free(sb.items);

    return result;
}

// pre-defined tags: .OPEN, .CLOSED, .UNTAGGED, .TAGGED (not .UNTAGGED)
// by default: .OPEN
bool print_tasks(const tasks_t *tasks, Flag_List_Mut *tokens, print_tasks_opt opts)
{
    bool name_filtering = is_task_name(tokens);
    bool result = true;
    u32 task_count = 0;
    task_t *ordered = NULL;
    task_t **task_list = NULL;

    String_Builder sb = {0};
    args_to_query_string(&sb, tokens);

    if (name_filtering) {
        task_count = retrieve_tasks_from_name(tasks, sv_from_cstr(tokens->items[0]), task_list);
    } else {
        task_list = calloc(tasks->count, sizeof(task_t *));
        if (!task_list) {
            return_defer(false);
        }
        String_View sv = sb_to_sv(sb);
        task_count = get_tasks(tasks, sv, task_list);
    }

    if (task_count > 0) {
        ordered = calloc(task_count, sizeof(task_t));
        for (u32 i = 0; i < task_count; ++i)
            ordered[i] = *task_list[i];

        // TASK(20260824-192224): allow displaying and sorting task by their huid
        if (opts.byHUID) {
            if (opts.reversed) qsort(ordered, task_count, sizeof(task_t), cmp_tasks_by_huid_reversed_void);
            else qsort(ordered, task_count, sizeof(task_t), cmp_tasks_by_huid_void);
        } else {
            if (opts.reversed) qsort(ordered, task_count, sizeof(task_t), cmp_tasks_rev_void);
            else qsort(ordered, task_count, sizeof(task_t), cmp_tasks_void);
        }

        size_t alignment = find_best_alignment(ordered, task_count);

        for (u32 i = 0; i < task_count; ++i) {
            print_task(stdout, &ordered[i], alignment);
        }
    } else {
        if (name_filtering) {
            nob_log(INFO, "No tasks were found having \"%s\" in their name", tokens->items[0]);
        } else {
            nob_log(INFO, "No tasks fitting your query (\"%s\") were found", sb.items);
        }
    }

defer:

    free(ordered);
    free(task_list);
    free(sb.items);
    return result;
}


task_t *create_task(const char *path, task_info_t *info, bool no_editor)
{
    String_Builder sb = {0};
    task_t *result = calloc(1, sizeof(task_t));
    if (result == NULL) {
        nob_log(ERROR, "Failed to calloc a task_t");
        return NULL;
    }
    char *task_name = info->title;

    int priority = 100;
    if (info->priority != NULL) {
        if (atoi(info->priority) > 0) {
            priority = atoi(info->priority);
        }
    }

    char *tags = "";
    if (info->tags != NULL) {
        tags = info->tags;
    }

    sb_appendf(&sb, "# %s\n", task_name);
    sb_appendf(&sb, "\n");
    sb_appendf(&sb, "- STATUS: OPEN\n");
    sb_appendf(&sb, "- PRIORITY: %d\n", priority);
    sb_appendf(&sb, "- TAGS: %s\n\n", tags);

    char *dir_name = get_timestamp_uuid();
    const char *task_path = temp_sprintf("%s/%s", path, dir_name);
    const char *task_md = temp_sprintf("%s/TASK.md", task_path);

    minimal_log_level = ERROR;
    if (!mkdir_if_not_exists(task_path)) goto defer;
#ifdef DEBUG
    minimal_log_level = NOB_DEBUG;
#else
    minimal_log_level = INFO;
#endif // DEBUG
    if (!write_entire_file(task_md, sb.items, sb.count)) goto defer;

    result->name = strdup(task_name);
    result->path = strdup(path);
    result->uuid = strdup(dir_name);
    result->priority = 1;
    result->status = STATUS_OPEN;
    result->tags.hasheq = ht_cstr_hasheq;

    if (!no_editor) {
        nob_log(INFO, "Created task at: %s%s/TASK.md%s", COLOR_RED, task_path, COLOR_RESET);
        Nob_Cmd cmd = {0};
        cmd_append(&cmd, "wl-copy");
        cmd_append(&cmd, "-n");
        cmd_append(&cmd, temp_sprintf("TASK(%s): %s", dir_name, task_name));
        minimal_log_level = WARNING;
        if (!cmd_run(&cmd)) {
            nob_log(WARNING, "Failed to copy HUID to clipboard. Is wl-copy installed?");
        }

        free(cmd.items);
#ifdef DEBUG
        minimal_log_level = DEBUG;
#else
        minimal_log_level = INFO;
#endif // DEBUG
    } else {
        nob_log(INFO, "Created task at: %s/TASK.md", task_path);
    }

defer:
    free(sb.items);
    free(dir_name);
    return result;
}

void initialise_tasks()
{
    *ht_put(&__g_stats, "OPEN") = 0;
    *ht_put(&__g_stats, "CLOSED") = 0;
    *ht_put(&__g_stats, "TOTAL") = 0;
    *ht_put(&__g_stats, "UNTAGGED") = 0;
}

// bool parse_subtasks(const char *path, subtasks_t *subtasks, const task_t *parent)
// {
//     File_Paths tasks_uuid = {0};
//     read_entire_dir(path, &tasks_uuid);
//
//     da_foreach (const char *, uuid, &tasks_uuid) {
//         if (!sv_starts_with(sv_from_cstr(*uuid), sv_from_cstr("."))) {
//             subtask_t subtask = {0};
//             parse_task(path, *uuid, subtask.self);
//             subtask.parent = parent;
//             da_append(subtasks, subtask);
//         }
//     }
//
//     free(tasks_uuid.items);
//     return true;
// }

bool parse_task(const char *path, const char *uuid, task_t *task, tasks_t *tasks)
{
    String_Builder sb = {0};
    String_View sv = {0};
    bool result = true;

    const char *task_path = temp_sprintf("%s/%s/", path, uuid);

    File_Paths paths = {0};
    if (!read_entire_dir(task_path, &paths)) {
        nob_log(ERROR, "Task(%s) directory was not found", uuid);
        return_defer(false);
    }

    // size_t checkpoint = temp_save();
    //
    // if (paths.count > 2) { // Each directory has the obligatory . and ..
    //     task->subtasks = calloc(1, sizeof(tasks_t));
    //     da_foreach (const char *, path, &paths) {
    //         temp_rewind(checkpoint);
    //         const char *full_path = temp_sprintf("%s%s", task_path, *path);
    //         Nob_File_Type ft = nob_get_file_type(full_path);
    //         if (ft == NOB_FILE_DIRECTORY && *path[0] != '.' && strstr(full_path, "tasks/")) { // Exclude '.', '..', '.git', etc
    //             parse_tasks(full_path, tasks, task, task->subtasks);
    //             break;
    //         }
    //     }
    // }

    free(paths.items);

    if (!read_entire_file(temp_sprintf("%sTASK.md", task_path), &sb)) {
        nob_log(WARNING, "Task(%s) directory was found, but no TASK.md was found inside.", uuid);
        return_defer(false);
    }
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
    sv_chop_left(&status, sizeof("- STATUS: ") - 1);
    const char *cstatus = temp_sv_to_cstr(status);
    *ht_find_or_put(&__g_stats, cstatus) += 1;
    *ht_put(&task->tags, cstatus) = true;
    task->status = cstr_to_task_status(cstatus);

    // - PRIORITY: UINT
    String_View priority = sv_chop_by_delim(&sv, '\n');
    sv_chop_left(&priority, sizeof("- PRIORITY: ") - 1);
    task->priority = temp_sv_to_int(priority);

    // - TAGS: <tag1>,<tag2>,<tag3>,...
    String_View tags_line = sv_chop_by_delim(&sv, '\n');
    if (sv_chop_prefix(&tags_line, sv_from_cstr("- TAGS: ")) && tags_line.count > 0) {
        while (tags_line.count) {
            size_t i = 0;
            while (i < tags_line.count && (tags_line.data[i] != ',' && tags_line.data[i] != ' ')) {
                i += 1;
            }

            Nob_String_View tag_sv = nob_sv_from_parts(tags_line.data, i);

            if (i < tags_line.count) {
                tags_line.count -= i + 1;
                tags_line.data  += i + 1;
            } else {
                tags_line.count -= i;
                tags_line.data  += i;
            }

            if (tag_sv.count == 0) continue;
            const char *tag = temp_sv_to_cstr(sv_trim(tag_sv));
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

bool parse_tasks(const char *path, tasks_t *tasks, const task_t *parent, tasks_t *subtasks)
{
    File_Paths tasks_dirs = {0};
    read_entire_dir(path, &tasks_dirs);

    da_foreach (const char *, dirs, &tasks_dirs) {
        const char *full_path = temp_sprintf("%s/%s", path, *dirs);
        Nob_File_Type ft = nob_get_file_type(full_path);
        if (ft == NOB_FILE_DIRECTORY) {
            if (!sv_starts_with(sv_from_cstr(*dirs), sv_from_cstr(".")) && strcmp(*dirs, "tags.md")) {
                    task_t task = {0};
                    parse_task(path, *dirs, &task, tasks);
                    da_append(tasks, task);

                if (parent != NULL && subtasks != NULL) {
                    task.parent = parent;
                    da_append(subtasks, task);
                }
            }
        }
    }

    free(tasks_dirs.items);
    return true;
}

void free_task(task_t *task)
{
    free(task->name);
    task->name = NULL;
    free(task->uuid);
    task->uuid = NULL;
    free(task->path);
    task->path = NULL;

    free(task->subtasks);
    task->subtasks = NULL;

    ht_free(&task->tags);
}

void free_tasks(tasks_t *tasks)
{
    da_foreach (task_t, task, tasks) {
        free_task(task);
    }
    free(tasks->items);
    ht_free(&__g_stats);
}

void init_directory(const char *tasks_dir, bool force_init)
{
    bool create_dir = true;
    const char *cwd = get_current_dir_temp();
    char *parent_tasks_dir = NULL;

    if (tasks_dir) {
        create_dir = false;
        parent_tasks_dir = get_parent_dir(tasks_dir);
        if (strcmp(cwd, parent_tasks_dir) == 0) {
            nob_log(ERROR, "tasks/ directory was already found in the current working directory.");
            free(parent_tasks_dir);
            return ;
        }

        if (!force_init) {
            char user_choice = 'n';
            nob_log(WARNING, "A tasks/ directory has been found at: %s/", tasks_dir);
            do {
                printf("[INFO] Do you still wish to initialize here? (y/N) : ");
                scanf("%c", &user_choice);
                if (user_choice == '\n') user_choice = 'n';
                user_choice = tolower(user_choice);
            } while (user_choice != 'y' && user_choice != 'n');

            switch (user_choice) {
                case 'n': {
                    nob_log(INFO, "Initialization procedure was cancelled");
                } break;
                case 'y': {
                    create_dir = true;
                } break;
            }
        }
    }

    if (create_dir || force_init) {
        mkdir_if_not_exists(temp_sprintf("%s/tasks", cwd));
    }
    free(parent_tasks_dir);
}

typedef enum {
    OVERWRITE_SET,
    OVERWRITE_ADD,
    OVERWRITE_SUB,
    __overwrite_mode_count
} overwrite_mode;

void change_task_priority(task_t *task, const char *new_priority)
{
    overwrite_mode priority_mode = OVERWRITE_SET;

    String_View a = sv_from_cstr(new_priority);
    if (sv_starts_with(a, sv_from_cstr("+"))) {
        priority_mode = OVERWRITE_ADD;
    } else if (sv_starts_with(a, sv_from_cstr("-"))) {
        priority_mode = OVERWRITE_SUB;
    }

    int priority = atoi(new_priority);
    int old_priority = task->priority;
    if (priority > 0) {
        switch (priority_mode) {
            case OVERWRITE_SET:
                task->priority = priority;
                break;
            case OVERWRITE_ADD:
                task->priority += priority;
                break;
            case OVERWRITE_SUB:
                task->priority -= priority;
                break;
            default:
                UNREACHABLE("priority: overwrite_mode");
        }
        nob_log(INFO, "Priority: changed from %d to %d for task(%s)", old_priority, priority, task->uuid);
    }
}

void change_task_tags(task_t *task, const char *tags)
{
    String_View tags_sv = sv_from_cstr(tags);
    bool was_reset = false;

    while (tags_sv.count) {
        String_View tag = sv_chop_by_delim(&tags_sv, ',');

        // The behavior is for the tags to either be added/remove or be overriden entirely.
        // Meaning that overridding the tags automatically disables the adding/removing feature by default.
        // Such that: -t test,testing,tested,+add,-remove would not be a "valid" overwrite command
        // But this : -t +add1,+add2,-remove1,+add3,-remove2 is valid
        // And this : -t +add1,+add2,set1,+add3,-remove1 is not valid
        if (sv_starts_with(tag, SVLIT("+"))) {
            if (was_reset) continue;
            sv_chop_left(&tag, 1);
            const char *tag_cstr = temp_sv_to_cstr(tag);
            bool found = *ht_find(&task->tags, tag_cstr);
            if (found) {
                nob_log(WARNING, "Tag \""SV_Fmt"\" already exists for task(%s)", SV_Arg(tag), task->uuid);
            } else {
                *ht_put(&task->tags, tag_cstr) = true;
                nob_log(INFO, "Tag \""SV_Fmt"\" was added to task(%s)", SV_Arg(tag), task->uuid);
            }
        } else if (sv_starts_with(tag, SVLIT("-"))) {
            if (was_reset) continue;
            sv_chop_left(&tag, 1);
            const char *tag_cstr = temp_sv_to_cstr(tag);
            bool found = *ht_find(&task->tags, tag_cstr);
            if (!found) {
                nob_log(WARNING, "Tag \""SV_Fmt"\" is not present for task(%s)", SV_Arg(tag), task->uuid);
            } else {
                ht_find_and_delete(&task->tags, tag_cstr);
                nob_log(INFO, "Tag \""SV_Fmt"\" was removed from task(%s)", SV_Arg(tag), task->uuid);
            }
        } else {
            const char *tag_cstr = temp_sv_to_cstr(tag);
            if (!was_reset) {
                ht_reset(&task->tags);
                was_reset = true;
            }
            *ht_put(&task->tags, tag_cstr) = true;
        }
    }
}

task_t *find_task_by_uuid(const tasks_t *tasks, const char *uuid)
{
    da_foreach (task_t, task, tasks) {
        if (strcmp(task->uuid, uuid) != 0) continue;
        return task;
    }

    return NULL;
}


// task(20260805-162024): This task is used as a test subject for the overwrite command
bool overwrite_task(tasks_t *tasks, task_info_t *info)
{
    bool result = true;
    tasks_t target_task = {0};

    if (info->items == NULL) {
        nob_log(ERROR, "Failed to overwrite task: no task huid was provided");
        return false;
    }

    bool no_changes = true;

    da_foreach (char *, item, info) {
        if ((*item)[0] != '.') {
            task_t *task = find_task_by_uuid(tasks, *item);
            da_append(&target_task, *task);
        } else {
            TODO("implement using query rather than task huid");
        }
    }

    da_foreach (task_t, task, &target_task) {
        // TODO("Figure out what to do with tasks when overwriting something");
        no_changes = true;
        const char *task_md_path = temp_sprintf("%s/%s/TASK.md", task->path, task->uuid);

        // 1 | # <title>\n
        if (info->title != NULL) {
            nob_log(INFO, "Title: renamed from \"%s\" to \"# %s\" for task(%s)", task->name, info->title, task->uuid);
            task->name = info->title;
            no_changes = false;
        }
        // 2 | \n

        // 3 | - STATUS: <STATUS>\n
        if (info->status != task->status && info->status != STATUS_NONE) {
            nob_log(INFO, "%s task(%s): %s", (info->status == STATUS_CLOSED)? "Closed" : "Reopened", task->uuid, task->name);
            task->status = info->status;
            no_changes = false;
        }

        // 4 | - PRIORITY: <PRIORITY>\n
        if (info->priority != NULL) {
            change_task_priority(task, info->priority);
            no_changes = false;
        }

        // 5 | - TAGS: <TAGS>\n
        if (info->tags != NULL) {
            change_task_tags(task, info->tags);
            no_changes = false;
        }
        // 6 | \n

        if (!no_changes) render_task_md(task_md_path, task);
    }

    free(target_task.items);
    return result;
}

bool parse_tags(const char *tasks_path)
{
    const char *tags_path = temp_sprintf("%s/tags.md", tasks_path);
    String_Builder sb = {0};
    bool result = true;
    String_View sv = {0};

    if (!file_exists(tags_path)) {
        // Silently exit, not every user would have a tags.md to describe there tags
        return_defer(false);
    }

    // File structure:
    // <tag>: <description>
    if (!read_entire_file(tags_path, &sb)) {
        return_defer(false);
    }

    sv = sb_to_sv(sb);
    while (sv.count > 0) {
        String_View line = sv_chop_by_delim(&sv, '\n');
        String_View tag_sv = sv_chop_by_delim(&line, ' ');
        sv_chop_right(&tag_sv, 1); // remove the ':'

        tag_t tag = {
            .name = sv_to_cstr(tag_sv),
            .description = sv_to_cstr(line)
        };

        da_append(&__g_tags, tag);
    }

defer:
    free(sb.items);
    return result;
}

void free_tag(tag_t *tag)
{
    free(tag->description);
    tag->description = NULL;
    free(tag->name);
    tag->name = NULL;
}

void free_tags(tags_t *tags)
{
    da_foreach (tag_t, tag, tags) {
        free_tag(tag);
    }

    free(tags->items);
}

bool render_task_md(const char *task_md_path, task_t *task)
{
    // 0 | # <title>\n
    // 1 | \n
    // 2 | - STATUS: <STATUS>\n
    // 3 | - PRIORITY: <PRIORITY>\n
    // 4 | - TAGS: <TAGS>\n
    // 5 | \n
    String_Builder file_reader = {0};
    String_Builder sb = {0};
    bool result = true;
    if (!read_entire_file(task_md_path, &file_reader)) return_defer(false);

    // Ready the rest of the file to fill the sb once it's been reconstructed
    String_View sv = sb_to_sv(file_reader);
    for (int i = 0; i < 5; ++i) sv_chop_by_delim(&sv, '\n');


    sb_appendf(&sb, "# %s\n", task->name);
    sb_appendf(&sb, "\n");
    sb_appendf(&sb, "- STATUS: %s\n", task_status_to_cstr(task->status));
    sb_appendf(&sb, "- PRIORITY: %ld\n", task->priority);
    sb_appendf(&sb, "- TAGS: ");

    int i = 0;
    ht_foreach(tag, &task->tags) {
        const char *tag_name = ht_key(&task->tags, tag);
        if (is_special_tag(tag_name)) continue;
        if (i != 0) {
            sb_appendf(&sb, ",");
        }
        sb_appendf(&sb, "%s", tag_name);
        i += 1;
    }
    sb_appendf(&sb, "\n");

    sb_append_buf(&sb, sv.items, sv.count);

    if (!write_entire_file(task_md_path, sb.items, sb.count)) return_defer(false);

defer:
    free(sb.items);
    free(file_reader.items);
    return result;
}

/*
 * ├ ─ │ └
 *
 * your-project/
 * ├── tasks/
 * │   ├── 20260331-144635/
 * │   │   └── TASK.md
 * │   ├── 20260330-202358/
 * │   │   └── TASK.md
 * │   └── 20260329-123700/
 * │       └── TASK.md
 * └── ...
 */
