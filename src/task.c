#include <assert.h>
#include <ctype.h>
#include <string.h>
#define HT_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"
#include "../lib/parser.h"

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

bool remove_tasks(tasks_t *tasks, Flag_List_Mut *tasks_uuid)
{
    da_foreach (task_t, task, tasks) {
        for (u64 i = 0; i < tasks_uuid->count; ++i) {
            if (strcmp(task->uuid, tasks_uuid->items[i]) == 0) {
                if (!remove_task(task)) return false;
                da_remove_unordered(tasks_uuid, i);
                break;
            }
        }
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
                task_info_t info = {.task_id = task->uuid, .status = new_status};
                if (!overwrite_task(tasks, &info)) return false;
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
    u64 ite = 0;
    ht_foreach(value, &__g_stats) {
        const char *key = ht_key(&__g_stats, value);
        if (strcmp(key, "OPEN") && strcmp(key, "CLOSED") && strcmp(key, "UNTAGGED") && strcmp(key, "TOTAL")) {
            ordered_list[ite].key = key;
            ordered_list[ite++].value = *value;
        }
    }

    qsort(ordered_list, __g_stats.count, sizeof(struct keyval), cmp_keyval_void);

    for (u64 i = 0; i < ite; ++i) {
        printf("    %*s => %d\n", longest_tag_name, ordered_list[i].key, ordered_list[i].value);
    }
}

struct task_distance {
    u64 dist;
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

int cmp_tasks_rev(const task_t *t1, const task_t *t2)
{
    return t1->priority - t2->priority;
}

int cmp_tasks_rev_void(const void *t1, const void *t2)
{
    return cmp_tasks_rev((const task_t *)t1, (const task_t *)t2);
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


u32 filter_by_name(const tasks_t *tasks, String_View name, task_t **result)
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


typedef Ht(char *, task_t *) tag_set;

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

u32 eval_node(const tasks_t *tasks, Node_t *root, bool negated, task_t **result)
{
    tag_set ht_tasks_set = { .hasheq = ht_cstr_hasheq };
    u32 result_ite = 0;
    switch (root->kind) {
    case NODE_TAG: {
        result_ite = eval_tag(tasks, result, root->tag_name, negated);
        break;
    }
    case NODE_NOT: {
        assert(root->lhs && "not-node's lhs should not be NULL");
        result_ite = eval_node(tasks, root->lhs, true, result);
        break;
    }
    case NODE_AND: {
        assert(root->lhs && "and-node's lhs should not be NULL");
        task_t **lhs_result =  calloc(tasks->count, sizeof(task_t *));
        if (!lhs_result) {
            nob_log(ERROR, "Failed to allocate space for table of task_t pointers");
            exit(1);
        }

        u32 lhs_ite = 0;
        u32 rhs_ite = 0;
        lhs_ite = eval_node(tasks, root->lhs, negated, lhs_result);

        assert(root->lhs && "and-node's rhs should not be NULL");
        rhs_ite = eval_node(tasks, root->rhs, negated, result);

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

        break;
    }
    case NODE_OR: {
        assert(root->lhs && "and-node's lhs should not be NULL");
        result_ite = eval_node(tasks, root->lhs, negated, result);
        for (u32 i = 0; i < result_ite; ++i) {
            *ht_find_or_put(&ht_tasks_set, result[i]->uuid) = result[i];
        }

        assert(root->lhs && "and-node's rhs should not be NULL");
        memset(result, 0, result_ite * sizeof(task_t *));
        result_ite = eval_node(tasks, root->rhs, negated, result);
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

        break;
    }
    default:
        UNREACHABLE("Node_Kind");
    }

    return result_ite;
}

// pre-defined tags: .OPEN, .CLOSED, .UNTAGGED, .TAGGED (not .UNTAGGED)
// by default: .OPEN
bool print_tasks(const tasks_t *tasks, Flag_List_Mut *tokens, bool reversed)
{

    String_View sv = {0};
    String_Builder sb = {0};
    bool ignore_default = false;
    bool all = false;
    bool name_filtering = false;
    bool result = true;
    Flag_List_Mut save = *tokens;
    task_t *ordered = NULL;
    u32 n = 0;
    Lexer *l = NULL;
    Parser *s = NULL;
    Node_t *ast = NULL;

    task_t **list = NULL;
    list = calloc(tasks->count, sizeof(task_t *));
    if (!list) return_defer(false);

    if (tokens->count == 1) {
        for (char *c = tokens->items[0]; *c != '\0'; ++c) {
            if (*c == ' ') {
                name_filtering = true;
                break;
            }
        }
    }

    if (!name_filtering) {
        {
            String_Builder temp_sb = {0};
            for (u64 i = 0; i < tokens->count; ++i) {
                sb_appendf(&temp_sb, "%s ", tokens->items[i]);
            }

            if (temp_sb.count > 0 && (strstr(temp_sb.items, ".all"))) {
                all = true;
                shift(tokens->items, tokens->count);
            }

            if (temp_sb.count > 0 && (strstr(temp_sb.items, ".CLOSED") || strstr(temp_sb.items, "not .OPEN") || !strstr(temp_sb.items, "."))) {
                ignore_default = true;
            }

            free(temp_sb.items);
        }

        if (all) {
            sb_appendf(&sb, "(.OPEN or .CLOSED) ");
        } else if (!ignore_default) {
            sb_appendf(&sb, ".OPEN");
            if (tokens->count > 0) sb_appendf(&sb, " and ");
            if (tokens->count > 1) sb_appendf(&sb, "(");
        }

        for (u64 i = 0; i < tokens->count; ++i) {
            sb_appendf(&sb, "%s%s", tokens->items[i], (i == tokens->count -1)? "" : " ");
        }

        if (!ignore_default && tokens->count > 1 && !all)
            sb_appendf(&sb, ")");

        sb_append_null(&sb);
        sv = sb_to_sv(sb);
        *tokens = save;

        // nob_log(INFO, SV_Fmt, SV_Arg(sv));

        l   = init_lexer(sv.items);
        s   = init_parser(l);
        ast = parse_query(s);

        // Size of tasks->count; The list may contain holes, or be incomplete due to the filtering
        n = eval_node(tasks, ast, false, list);
    } else {
        n = filter_by_name(tasks, sv_from_cstr(tokens->items[0]), list);
    }

    if (n > 0) {
        ordered = calloc(n, sizeof(task_t));
        for (u32 i = 0; i < n; ++i)
            ordered[i] = *list[i];

        if (reversed) qsort(ordered, n, sizeof(task_t), cmp_tasks_rev_void);
        else qsort(ordered, n, sizeof(task_t), cmp_tasks_void);

        size_t alignment = find_best_alignment(ordered, n);

        for (u32 i = 0; i < n; ++i) {
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
    if (result) {
        free(list);
        free(ordered);
        clean_ast(ast);
        clean_parser(&s);
    }

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

    size_t checkpoint = temp_save();

    if (paths.count > 2) { // Each directory has the obligatory . and ..
        task->subtasks = calloc(1, sizeof(tasks_t));
        da_foreach (const char *, path, &paths) {
            temp_rewind(checkpoint);
            const char *full_path = temp_sprintf("%s%s", task_path, *path);
            Nob_File_Type ft = nob_get_file_type(full_path);
            if (ft == NOB_FILE_DIRECTORY && *path[0] != '.' && strstr(full_path, "tasks/")) { // Exclude '.', '..', '.git', etc
                parse_tasks(full_path, tasks, task, task->subtasks);
                break;
            }
        }
    }

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
            const char *tag = temp_sv_to_cstr(sv_chop_by_delim(&tags_line, ','));
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
    File_Paths tasks_uuid = {0};
    read_entire_dir(path, &tasks_uuid);

    da_foreach (const char *, uuid, &tasks_uuid) {
        const char *full_path = temp_sprintf("%s/%s", path, *uuid);
        Nob_File_Type ft = nob_get_file_type(full_path);
        if (ft == NOB_FILE_DIRECTORY) {
            if (!sv_starts_with(sv_from_cstr(*uuid), sv_from_cstr("."))) {
                    task_t task = {0};
                    parse_task(path, *uuid, &task, tasks);
                    da_append(tasks, task);

                if (parent != NULL && subtasks != NULL) {
                    task.parent = parent;
                    da_append(subtasks, task);
                }
            }
        }
    }

    free(tasks_uuid.items);
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

// 1 | # <title>\n
// 2 | \n
// 3 | - STATUS: <STATUS>\n
// 4 | - PRIORITY: <PRIORITY>\n
// 5 | - TAGS: <TAGS>\n
// 6 | \n
#define TITLE_LINE 1
#define STATUS_LINE 3
#define PRIORITY_LINE 4
#define TAGS_LINE 5

s32 set_attribut_line(const char *path, const char *new_attribut, String_Builder *previous_attribut, s32 line_number, String_Builder *sb, String_Builder *temp_sb) {
    if (!read_file_until_n_line(path, line_number, sb, temp_sb)) return false;

    size_t ite = sb->count;
    while (sb->items[ite] != '\n' && sb->items[ite] != '\0') {
        sb_appendf(previous_attribut, "%c", sb->items[ite]);
        ite += 1;
    }
    sb_append_null(previous_attribut);

    sb_appendf(sb, "%s\n", new_attribut);
    sb_append_buf(sb, temp_sb->items, temp_sb->count);
    if (!write_entire_file(path, sb->items, sb->count)) return false;
    return true;
}

// task(20260805-022652): Implement overwrite cmdline
bool overwrite_task(tasks_t *tasks, task_info_t *info)
{
    if (info->task_id == NULL) {
        nob_log(ERROR, "Failed to overwrite task: no task huid was provided");
        return false;
    }

    // task(20260805-162024): This task is used as a test subject for the overwrite command
    da_foreach (task_t, task, tasks) {
        if (strcmp(task->uuid, info->task_id) != 0) continue;

        const char *task_md_path = temp_sprintf("%s/%s/TASK.md", task->path, task->uuid);
        String_Builder sb = {0};
        String_Builder temp_sb = {0};
        bool result = true;

        // -=-=-=-=-=-=-=-=-=-=-= TITLE =-=-=-=-=-=-=-=-=-=-=-
        // TASK(20260813-001316): Overwriting title does not work
        if (info->title != NULL) {
            String_Builder previous_title = {0};
            if (!set_attribut_line(task_md_path, temp_sprintf("# %s", info->title), &previous_title, TITLE_LINE, &sb, &temp_sb)) return_defer(false);
            nob_log(INFO, "Title: changed from \"%*s\" to \"# %s\" for task(%s)", (int)previous_title.count-1, previous_title.items, info->title, task->uuid);
            free(previous_title.items);
            return_defer(true);
        }
        overwrite_mode priority_mode = OVERWRITE_SET;
        overwrite_mode tag_mode      = OVERWRITE_SET;

        // -=-=-=-=-=-=-=-=-=-=-= PRIORITY =-=-=-=-=-=-=-=-=-=-=-
        if (info->priority != NULL) {
            String_View a = sv_from_cstr(info->priority);
            if (sv_starts_with(a, sv_from_cstr("+"))) {
                priority_mode = OVERWRITE_ADD;
            } else if (sv_starts_with(a, sv_from_cstr("-"))) {
                priority_mode = OVERWRITE_SUB;
            }

            int new_priority = atoi(info->priority);
            int old_priority = task->priority;
            if (new_priority > 0) {
                switch (priority_mode) {
                    case OVERWRITE_SET:
                        task->priority = new_priority;
                        break;
                    case OVERWRITE_ADD:
                        task->priority += new_priority;
                        break;
                    case OVERWRITE_SUB:
                        task->priority -= new_priority;
                        break;
                    default:
                        UNREACHABLE("priority: overwrite_mode");
                }
                if (!read_file_until_n_line(task_md_path, PRIORITY_LINE, &sb, &temp_sb)) return_defer(false);

                sb_appendf(&sb, "%d\n", new_priority);
                sb_append_buf(&sb, temp_sb.items, temp_sb.count);

                if (!write_entire_file(task_md_path, sb.items, sb.count)) return_defer(false);

                nob_log(INFO, "Priority: changed from %d to %d for task(%s)", old_priority, new_priority, task->uuid);
                sb.count = 0;
                temp_sb.count = 0;
            }
        }

        // -=-=-=-=-=-=-=-=-=-=-= TAGS =-=-=-=-=-=-=-=-=-=-=-
        if (info->tags != NULL) {
            String_View a = sv_from_cstr(info->tags);
            // nob_log(INFO, "tags: "SV_Fmt, SV_Arg(a));

            while (a.count) {
                size_t i = 0;
                while (i < a.count && a.items[i] != ',') {
                    i += 1;
                }
                String_View tag_sv = sv_from_parts(a.items, i);

                if (sv_starts_with(tag_sv, sv_from_cstr("+"))) {
                    tag_mode = OVERWRITE_ADD;
                    sv_chop_left(&tag_sv, 1);
                } else if (sv_starts_with(tag_sv, sv_from_cstr("-"))) {
                    tag_mode = OVERWRITE_SUB;
                    sv_chop_left(&tag_sv, 1);
                }

                const char *tag = nob_temp_sv_to_cstr(tag_sv);
                bool tag_already_present = ht_find(&task->tags, tag);
                // nob_log(INFO, "tag mode: %s", (tag_mode == OVERWRITE_SET)? "set" : (tag_mode == OVERWRITE_ADD)? "add" : "sub");
                // nob_log(INFO, "current tag: %s", tag);

                if (tag_already_present) {
                    if (tag_mode == OVERWRITE_SUB) {
                        if (!read_file_until_n_line(task_md_path, TAGS_LINE, &sb, &temp_sb)) return_defer(false);
                        bool read_tag_found = false;
                        size_t total_count = 0;
                        String_Builder read_tag = {0};
                        size_t ite = 0;
                        s32 character_count = 0;

                        while (sb.items[sb.count++] != '\n') character_count++;
                        sb.count -= character_count + 1;
                        total_count = character_count;

                        do {
                            while (sb.items[sb.count + ite] != ',' && sb.items[sb.count + ite] != '\n') {
                                sb_appendf(&read_tag, "%c", sb.items[sb.count + ite]);
                                ite += 1;
                            }
                            sb_append_null(&read_tag);
                            character_count -= ite + 1; // +1 to account for the comma

                            // nob_log(INFO, "read tag: %s", read_tag.items);
                            if (strcmp(read_tag.items, tag) == 0) {
                                read_tag_found = true;
                                nob_log(INFO, "Tag: \"%s\" was removed from task(%s)", tag, task->uuid);
                                // TAGS: test,bug,cmdline-options\n < file
                                // TAGS: test,cmdline-options\n     < sb
                                if (sb.items[sb.count + ite + 1] == '\n') {
                                    sb.count -= 1; // If tag is at the end of the list, remove the last comma off it.
                                    character_count += 1; // In reverse, character count is probably negative, so cancel that out
                                }
                                sb_append_buf(&sb, sb.items + sb.count + ite + 1, character_count + 1);
                                break;
                            }

                            sb.count += ite+1;
                            read_tag.count = 0;
                            ite = 0;
                        } while (sb.items[sb.count] != '\n');

                        if (!read_tag_found) {
                            sb_append_buf(&sb, sb.items + sb.count - total_count - 1, total_count + 1);
                        }

                        sb_append_buf(&sb, temp_sb.items, temp_sb.count);
                        if (!write_entire_file(task_md_path, sb.items, sb.count)) return_defer(false);
                    } else if (tag_mode == OVERWRITE_ADD) {
                        nob_log(WARNING, "Tag: \"%s\" for task(%s) is already present. Addition cancelled", tag, task->uuid);
                    }
                } else {
                    if (tag_mode == OVERWRITE_SUB) {
                        nob_log(WARNING, "Tag: \"%s\" for task(%s) was not found. Deletion cancelled", tag, task->uuid);
                    } else if (tag_mode == OVERWRITE_ADD) {
                        if (!read_file_until_n_line(task_md_path, TAGS_LINE, &sb, &temp_sb)) return_defer(false);
                        while (sb.items[sb.count++] != '\n');

                        sb.count -= 1;

                        sb_appendf(&sb, ",%s\n", tag);
                        nob_log(INFO, "Tag: \"%s\" was added to task(%s)", tag, task->uuid);

                        sb_append_buf(&sb, temp_sb.items, temp_sb.count);
                        if (!write_entire_file(task_md_path, sb.items, sb.count)) return_defer(false);
                    } else if (tag_mode == OVERWRITE_SET) {
                        String_Builder previous_tags = {0};

                        if (!set_attribut_line(task_md_path, info->tags, &previous_tags, TAGS_LINE, &sb, &temp_sb)) return_defer(false);
                        nob_log(INFO, "Tag: changed from \"%*s\" to \"%s\" for task(%s)", (int)previous_tags.count-1, previous_tags.items, info->tags, task->uuid);

                        a.count = 0;
                        free(previous_tags.items);
                    }
                }

                if (i < a.count) {
                    a.count -= i + 1;
                    a.data  += i + 1;
                } else {
                    a.count -= i;
                    a.data  += i;
                }

                sb.count = 0;
                temp_sb.count = 0;
            }
        }

        if (info->status != task->status && info->status != STATUS_NONE) {
            if (!read_file_until_n_line(task_md_path, STATUS_LINE, &sb, &temp_sb)) return_defer(false);

            sb_appendf(&sb, "%s\n", task_status_to_cstr(info->status));
            sb_append_buf(&sb, temp_sb.items, temp_sb.count);

            if (!write_entire_file(task_md_path, sb.items, sb.count)) return_defer(false);

            nob_log(INFO, "%s task(%s): %s", (info->status == STATUS_CLOSED)? "Closed" : "Reopened", task->uuid, task->name);
            sb.count = 0;
            temp_sb.count = 0;
        }

defer:
        free(sb.items);
        free(temp_sb.items);
        return result;
    }
    return false;
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
