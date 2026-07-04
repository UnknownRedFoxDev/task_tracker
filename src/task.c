#include <ctype.h>
#define HT_IMPLEMENTATION
#include "../lib/task.h"
#include "../lib/helper.h"

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
    const char *task_md_path = temp_sprintf("%s/%s/TASK.md", task->path, task->uuid);
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

    nob_log(INFO, "Closed task(%s): %s", task->uuid, task->name);

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
    cmd_append(&cmd, temp_sprintf("%s/%s/TASK.md", task->path, task->uuid));
    if (!cmd_run(&cmd)) result = false;

    free(cmd.items);
    return result;
}

void print_task(FILE *stream, task_t *task)
{
    String_Builder sb = {0};
    sb_appendf(&sb, "%s%s/TASK.md%s:%s1%s: ", COLOR_RED, task->uuid, COLOR_RESET, COLOR_YELLOW, COLOR_RESET);
    sb_appendf(&sb, "[PRIORITY: %-2zu ", task->priority);
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

String_View see_next_token(String_View *sv) {
    size_t i = 0;
    while (i < sv->count && sv->data[i] != ' ') {
        i += 1;
    }
    String_View result = nob_sv_from_parts(sv->data, i);
    if (sv_starts_with(result, sv_from_cstr("."))) sv_chop_left(&result, 1);
    return result;
}

String_View get_next_token(String_View *sv)
{
    String_View result = sv_chop_by_delim(sv, ' ');
    if (sv_starts_with(result, sv_from_cstr("."))) sv_chop_left(&result, 1);
    return result;
}

void advance_to_next_token(String_View *sv, size_t i)
{
    if (sv->count > 0) {
        if (i < sv->count) {
            sv->count -= i + 1;
            sv->data  += i + 1;
        } else {
            sv->count -= i;
            sv->data  += i;
        }
    }
}

size_t eval_tag(const tasks_t *tasks, task_t **result, String_View tag, bool negate_mode)
{
    memset(result, 0, tasks->count * sizeof(task_t *));
    size_t result_ite = 0;

    nob_log(NOB_DEBUG, "-------------------------");
    nob_log(NOB_DEBUG, "tag: %s" SV_Fmt, (negate_mode)? "NOT " : "", SV_Arg(tag));

    da_foreach (task_t, task, tasks) {
        bool *found = ht_find(&task->tags, temp_sv_to_cstr(tag));
        if ((!found && negate_mode) || (found && !negate_mode)) {
            if (minimal_log_level == NOB_DEBUG) print_task(stdout, task);
            result[result_ite++] = task;
        }
    }

    nob_log(NOB_DEBUG, "-------------------------");

    return result_ite;
}

typedef Ht(char *, task_t *) tag_set;

void eval_and_put(const tasks_t *tasks, task_t **result, size_t result_ite, tag_set *ht_tasks_set,
                    boolean_keywords curr_mode, boolean_keywords prev_mode, String_View prev_token)
{
    nob_log(NOB_DEBUG, "Tasks fitting their tag(s) %s %s"SV_Fmt, boolean_keyword_to_string(curr_mode), (prev_mode == NOT)? "not " : "", SV_Arg(prev_token));
    for (size_t i = 0; i < result_ite; ++i) {
        task_t *task = result[i];
        bool *found = ht_find(&task->tags, temp_sv_to_cstr(prev_token));

        // If task has the tag from `prev_token` and `AND` mode is enabled. Otherwise just put it in the HTable
        if (curr_mode == OR || (((!found && prev_mode == NOT) || (found && prev_mode != NOT)) && curr_mode == AND)) {
            if (minimal_log_level == NOB_DEBUG) print_task(stdout, task);
            *ht_find_or_put(ht_tasks_set, task->uuid) = task;
        }
    }
    nob_log(NOB_DEBUG, "-------------------------");
}

char *parse_parenthesis(String_View *tokens)
{
    String_View sub_expr_tokens = {0};
    char *result = NULL;
    String_Builder sb = {0};
    int parenthesis_count = 0;

    do {
        String_View token = get_next_token(tokens);
        sb_appendf(&sb, SV_Fmt"%s", SV_Arg(token), (tokens->count > 0)? " " : "");
        nob_log(NOB_DEBUG, "token: "SV_Fmt, SV_Arg(token));
        if (sv_ends_with(token, sv_from_cstr(")"))) {
            while (sv_ends_with(token, sv_from_cstr(")"))) {
                sv_chop_right(&token, 1);
                parenthesis_count--;
            }
        } else if (sv_starts_with(token, sv_from_cstr("("))) {
            if (sv_starts_with(token, sv_from_cstr("(."))) sv_chop_left(&token, 2);
            else sv_chop_left(&token, 1);
            parenthesis_count++;
        }
    } while (parenthesis_count && tokens->count > 0);
    // parenthesis_count--; // Remove initial parenthesis count

    if (parenthesis_count > 0) {
        nob_log(ERROR, "Missing closing or opening parenthese(s)");
        exit(1); // Temporary
    }

    sub_expr_tokens = sb_to_sv(sb);
    sv_chop_left(&sub_expr_tokens, 1);
    if (sv_ends_with(sub_expr_tokens, sv_from_cstr(") "))) sv_chop_right(&sub_expr_tokens, 2);
    else sv_chop_right(&sub_expr_tokens, 1);
    nob_log(NOB_DEBUG, "-------------------------");
    nob_log(NOB_DEBUG, "sub expr: `"SV_Fmt"`", SV_Arg(sub_expr_tokens));
    nob_log(NOB_DEBUG, "-------------------------");
    result = sv_to_cstr(sub_expr_tokens);
    free(sb.items);
    return result;
}

/*
 * 20260621-110728:
 * When doing `not <tag>` or `<tag1> and <tag2>` or `<tag1> or <tag2>`, I'm not comparing tags, I'm comparing lists created from the tags and filtering the remainder
 * so when `<tag1> and (<tag2> or <tag3>)` I should be comparing list of tag1, with the last made of at least tag2 or tag3
 */
size_t eval_tokens(const tasks_t *tasks, String_View *tokens, task_t **result)
{
    tag_set ht_tasks_set = { .hasheq = ht_cstr_hasheq };

    size_t prev_inner_ite = 0;
    size_t result_ite = 0;

    String_View prev_token = {0};
    String_View curr_token = {0};
    String_View next_token = {0};
    boolean_keywords prev_mode = NONE;
    boolean_keywords curr_mode = NONE;
    boolean_keywords next_mode = NONE;

    nob_log(NOB_DEBUG, "Tokens: " SV_Fmt, SV_Arg(*tokens));

    while (tokens->count) {
        if (sv_starts_with(see_next_token(tokens), sv_from_cstr("("))) {
            char *temp_result =  parse_parenthesis(tokens);
            String_View sub_expr_tokens = sv_from_cstr(temp_result);
            result_ite = eval_tokens(tasks, &sub_expr_tokens, result);
            if (tokens->count > 0) {
                curr_token = get_next_token(tokens);
                next_token = see_next_token(tokens);

                if (sv_eq(curr_token, sv_from_cstr("and"))) curr_mode = AND;
                else if (sv_eq(curr_token, sv_from_cstr("or"))) curr_mode = OR;
            }
            eval_and_put(tasks, result, result_ite, &ht_tasks_set, curr_mode, prev_mode, next_token);
            free(temp_result);
            goto end;
        } else {
            curr_token = get_next_token(tokens);

            nob_log(NOB_DEBUG, "before modif: %s", boolean_keyword_to_string(curr_mode));
            nob_log(NOB_DEBUG, "prev: "SV_Fmt ", curr: "SV_Fmt ", next: "SV_Fmt, SV_Arg(prev_token), SV_Arg(curr_token), SV_Arg(next_token));
            nob_log(NOB_DEBUG, "-------------------------");

            if (sv_eq(curr_token, sv_from_cstr("not"))) {
                curr_mode = NOT;
                prev_token = curr_token;
                curr_token = get_next_token(tokens);
            } else if (sv_eq(curr_token, sv_from_cstr("and"))) {
                curr_mode = AND;
                next_token = see_next_token(tokens);
            } else if (sv_eq(curr_token, sv_from_cstr("or"))) {
                curr_mode = OR;
                next_token = see_next_token(tokens);
            }

            nob_log(NOB_DEBUG, "after modif: %s", boolean_keyword_to_string(curr_mode));
            nob_log(NOB_DEBUG, "prev: "SV_Fmt ", curr: "SV_Fmt ", next: "SV_Fmt, SV_Arg(prev_token), SV_Arg(curr_token), SV_Arg(next_token));
            nob_log(NOB_DEBUG, "-------------------------");
        }


        if (prev_token.count && prev_token.data[0] == '.') sv_chop_left(&prev_token, 1);
        if (curr_token.data[0] == '.') sv_chop_left(&curr_token, 1);
        if (next_token.count && next_token.data[0] == '.') sv_chop_left(&next_token, 1);
        switch (curr_mode) {
        case NONE: // Passthrough
        case NOT: {
            result_ite = eval_tag(tasks, result, curr_token, (curr_mode == NOT));
        } break;

        case AND: // Passthrough
        case OR: {
            if (sv_starts_with(next_token, sv_from_cstr("("))) {
                char *temp_result =  parse_parenthesis(tokens);
                String_View sub_expr_tokens = sv_from_cstr(temp_result);
                size_t result_ite = eval_tokens(tasks, &sub_expr_tokens, result);
                eval_and_put(tasks, result, result_ite, &ht_tasks_set, curr_mode, prev_mode, prev_token);
                free(temp_result);
            } else {
                if (sv_eq(next_token, sv_from_cstr("TAGGED"))) {
                    next_mode = NOT;
                    next_token = sv_from_cstr("UNTAGGED");
                } else if (sv_eq(next_token, sv_from_cstr("all"))) {
                    curr_mode = OR;
                    curr_token = sv_from_cstr("or");
                    next_token = sv_from_cstr("CLOSED");
                }

                if (sv_eq(next_token, sv_from_cstr("not"))) {
                    next_mode = NOT;
                    advance_to_next_token(tokens, next_token.count);
                    next_token = get_next_token(tokens);
                }

                nob_log(NOB_DEBUG, "previous tasks: %s" SV_Fmt, (prev_mode == NOT)? "not " : "", SV_Arg(prev_token));
                eval_and_put(tasks, result, prev_inner_ite, &ht_tasks_set, curr_mode, next_mode, next_token);
                nob_log(NOB_DEBUG, "-------------------------");

                // Parsing the next tasks having the tag from `next_token` once the previous tasks were tested
                nob_log(NOB_DEBUG, "next tasks: %s" SV_Fmt, (next_mode == NOT)? "not " : "", SV_Arg(next_token));
                result_ite = eval_tag(tasks, result, next_token, (next_mode == NOT));
                eval_and_put(tasks, result, result_ite, &ht_tasks_set, curr_mode, prev_mode, prev_token);
                nob_log(NOB_DEBUG, "-------------------------");
            }


            // Put the HTable's found tasks into the result array
            memset(result, 0, result_ite * sizeof(task_t *));
            result_ite = 0;
            ht_foreach(val, &ht_tasks_set) {
                result[result_ite++] = *val;
            }

            ht_reset(&ht_tasks_set);
            advance_to_next_token(tokens, next_token.count);
            curr_token = next_token;
        } break;
        default:
            UNREACHABLE("boolean_keywords: curr_token in print_tasks()");
        }

end:
        prev_mode = curr_mode;
        prev_inner_ite = result_ite;
        result_ite = 0;
        prev_token = curr_token;
    }

    nob_log(NOB_DEBUG, "Result: ");
    if (minimal_log_level == NOB_DEBUG) {
        for (size_t i = 0; i < prev_inner_ite; ++i) {
            print_task(stdout, result[i]);
        }
    }
    nob_log(NOB_DEBUG, "-------------------------");
    ht_free(&ht_tasks_set);

    return prev_inner_ite;
}

// pre-defined tags: .OPEN, .CLOSED, .UNTAGGED, .TAGGED (not .UNTAGGED)
// by default: .OPEN
bool print_tasks(const tasks_t *tasks, Flag_List_Mut *tokens, bool reversed)
{

    String_View sv = {0};
    String_Builder sb = {0};
    bool ignore_default = false;
    bool result = true;

    {
        String_Builder temp_sb = {0};
        for (size_t i = 0; i < tokens->count; ++i) {
            sb_appendf(&temp_sb, "%s ", tokens->items[i]);
        }

        if (temp_sb.count > 0 && (strstr(temp_sb.items, ".CLOSED") || strstr(temp_sb.items, "not .OPEN"))) {
            ignore_default = true;
        }

        free(temp_sb.items);
    }

    if (!ignore_default) {
        sb_appendf(&sb, ".OPEN");
        if (tokens->count > 0) sb_appendf(&sb, " and ");
        if (tokens->count > 1) sb_appendf(&sb, "(");
    }

    for (size_t i = 0; i < tokens->count; ++i) {
        sb_appendf(&sb, "%s%s", tokens->items[i], (i == tokens->count -1)? "" : " ");
    }

    if (!ignore_default && tokens->count > 1)
        sb_appendf(&sb, ")");

    sv = sb_to_sv(sb);


    // Size of tasks->count; The list may contain holes, or be incomplete due to the filtering
    task_t **list = calloc(tasks->count, sizeof(task_t *));
    size_t n = eval_tokens(tasks, &sv, list);
    task_t *ordered = NULL;
    if (!list) return_defer(false);

    if (n > 0) {
        ordered = calloc(n, sizeof(task_t));
        for (size_t i = 0; i < n; ++i)
            ordered[i] = *list[i];

        if (reversed) qsort(ordered, n, sizeof(task_t), cmp_tasks_rev_void);
        else qsort(ordered, n, sizeof(task_t), cmp_tasks_void);

        for (size_t i = 0; i < n; ++i) {
            print_task(stdout, &ordered[i]);
        }
    } else {
        nob_log(INFO, "No tasks fitting your query (\"%s\") were found", sb.items);
    }

defer:
    if (result) {
        free(list);
        free(ordered);
    }

    free(sb.items);
    return result;
}


task_t *create_task(const char *path, const char *task_name, cmdline_opts *opts)
{
    String_Builder sb = {0};
    task_t *result = calloc(1, sizeof(task_t));
    if (result == NULL) {
        nob_log(ERROR, "Failed to calloc a task_t");
        return NULL;
    }

    int priority = 100;
    if (opts->create_task_priority > 0) {
        priority = opts->create_task_priority;
    }

    char *tags = "";
    if (opts->create_task_tags != NULL) {
        tags = opts->create_task_tags;
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

    if (!read_entire_file(temp_sprintf("%s/%s/TASK.md", path, uuid), &sb)) {
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

    free(tasks_uuid.items);
    return true;
}

void free_task(task_t *task)
{
    free(task->name);
    free(task->uuid);
    free(task->path);
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

void init_directory(const char *tasks_dir)
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

    if (create_dir) {
        mkdir_if_not_exists(temp_sprintf("%s/tasks", cwd));
    }
    free(parent_tasks_dir);
}
