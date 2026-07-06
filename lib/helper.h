#ifndef HELPER_H_
#define HELPER_H_

#include "commons.h"
#include "task.h"

void usage(FILE *stream);
void parse_options(int argc, char **argv, cmdline_opts_t *opts);

char *get_timestamp_uuid();
int temp_sv_to_int(String_View sv);
char *sv_to_cstr(String_View sv);
char *tasks_path_search(const char *cwd);
char *get_parent_dir(const char *cwd);
char *find_tasks_dir(const char *cwd);
task_status cstr_to_task_status(const char *cstr);
const char *task_status_to_cstr(task_status status);
char *recursive_descend_tasks_path_search(const char *cwd, int depth, const char *excluded_path);


#endif // HELPER_H_
