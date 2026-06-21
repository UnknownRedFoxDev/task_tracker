#define NOB_IMPLEMENTATION
#define NOB_EXPERIMENTAL_DELETE_OLD
#define FLAG_IMPLEMENTATION
#include "lib/commons.h"

#define CXX             "gcc"
#define LIBPATH         BUILD_DIR"modules.a"
#define EXECUTABLE_NAME "tatr"
#define EXECUTABLE_PATH BIN_DIR EXECUTABLE_NAME

bool debug = false;

typedef struct {
    const char **items;
    size_t count;
    size_t capacity;
} submodules;

void add_standard_flags(Nob_Cmd *cmd)
{
    cmd_append(cmd, CXX);
    cmd_append(cmd, "-Wall");
    cmd_append(cmd, "-Wextra");
    cmd_append(cmd, "-Wno-missing-field-initializers");
    cmd_append(cmd, "-Wno-unused-parameter");
    cmd_append(cmd, "-Wno-unused-function");
    if (debug) {
        cmd_append(cmd, "-g");
        cmd_append(cmd, "-ggdb");
        cmd_append(cmd, "-fsanitize=address");
    }
}

void compile_command(Nob_Cmd *cmd, const char *input_path, const char *output_path, bool linking)
{
    add_standard_flags(cmd);
    cmd_append(cmd, "-o", output_path);
    if (!linking) cmd_append(cmd, "-c", input_path);
    else {
        cmd_append(cmd, input_path);
        if (file_exists(LIBPATH))
            cmd_append(cmd, LIBPATH);
    }
}

bool compile_submodules(submodules *modules, bool *needs_recompile)
{
    Cmd cmd = {0};
    Procs procs = {0};
    bool result = true;

    size_t mark = temp_save();
    for (size_t i = 0; i < modules->count; ++i) {
        nob_temp_rewind(mark);
        char *input = nob_temp_sprintf("%s%s.c", SRC_DIR, da_get(modules, i));
        char *output = nob_temp_sprintf("%s%s.o", BUILD_DIR, da_get(modules, i));
        if (nob_needs_rebuild1(output, input) || debug) {
            *needs_recompile = true;
            printf("-------------\n");
            printf("Input path: %s\nOutput path: %s\n", input, output);
            compile_command(&cmd, input, output, false); // No linking yet
            if (!cmd_run(&cmd, .async = &procs)) return_defer(false);
            printf("-------------\n");
        }
    }

    if (!procs_flush(&procs)) return_defer(false);

    if (*needs_recompile) {
        if (file_exists(LIBPATH)) delete_file(LIBPATH);

        nob_log(INFO, "Recreating the archive to hold the modules...");
        cmd_append(&cmd, "ar", "rcs");
        cmd_append(&cmd, LIBPATH);
        da_foreach(const char *, it, modules)
        {
            cmd_append(&cmd, temp_sprintf("%s%s.o", BUILD_DIR, *it));
        }
        if (!nob_cmd_run(&cmd)) return_defer(1);
    }

defer:
    free(cmd.items);
    free(procs.items);
    return result;
}

bool compile_main(Nob_Cmd *cmd, bool needs_recompile)
{
    if (needs_recompile && file_exists(EXECUTABLE_PATH))
        delete_file(EXECUTABLE_PATH);

    const char *main = SRC_DIR"main.c";

    if ((nob_needs_rebuild1(EXECUTABLE_PATH, main) || debug)) {
        compile_command(cmd, main, EXECUTABLE_PATH, true);
        if (!nob_cmd_run(cmd)) return false;
    }

    return true;
}


void usage(FILE *stream)
{
    fprintf(stream, "Usage: ./nob [OPTIONS]\n");
    fprintf(stream, "OPTIONS:\n");
    flag_print_options(stream);
}

void parse_flag(int argc, char **argv, bool *help, bool *rebuild, bool *run, bool *nobuild)
{
    flag_bool_var(&debug, "debug", false, "run in debug mode");
    flag_bool_var(help, "help", false, "Print this help");
    flag_bool_var(rebuild, "B", false, "Forces rebuilding the whole project");
    flag_bool_var(run, "run", false, "run the program with args");
    flag_bool_var(nobuild, "nobuild", false, "run the program without recompiling");

    if (!flag_parse(argc, argv)) {
        usage(stderr);
        flag_print_error(stderr);
        exit(1);
    }

    if (argc < 2) {
        usage(stderr);
        exit(1);
    }

    if (*help) {
        usage(stderr);
        exit(0);
    }
}

void initialise_directories()
{
    if (file_exists(BUILD_DIR)) {
        delete_directory_recursively(BUILD_DIR);
    }

    minimal_log_level = WARNING;
    nob_mkdir_if_not_exists(BUILD_DIR);
    nob_mkdir_if_not_exists(BIN_DIR);
    minimal_log_level = INFO;
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    bool help, rebuild, run, nobuild;
    bool needs_recompile = false;
    int result = 0;

    Nob_Cmd cmd = {0};
    parse_flag(argc, argv, &help, &rebuild, &run, &nobuild);

    if (rebuild) initialise_directories();
    if (!nobuild && !compile_main(&cmd, needs_recompile)) return_defer(1);

    if (run) {
        cmd_append(&cmd, EXECUTABLE_PATH);
        if (!nob_cmd_run(&cmd)) return_defer(1);
    }

defer:
    free(cmd.items);
    return result;
}
