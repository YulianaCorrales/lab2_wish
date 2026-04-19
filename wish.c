#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>  // PARA PERROR

#define MAXLINE 1024
#define MAXARGS 100
#define MAXPATH 100
#define MAXCMDS 50

char *path_dirs[MAXPATH];
int path_count = 0;

char error_message[30] = "An error has occurred\n";

// ============================================================================
// DEBUG MACROS
// ============================================================================

#ifdef DEBUG
#define debug_print(fmt, ...) \
    do { \
        char buf[256]; \
        snprintf(buf, sizeof(buf), "[DEBUG] " fmt, ##__VA_ARGS__); \
        write(STDERR_FILENO, buf, strlen(buf)); \
    } while(0)
#else
#define debug_print(fmt, ...)
#endif

// ============================================================================
// PATH MANAGEMENT
// ============================================================================

void init_path(void) {
    path_dirs[0] = strdup("/bin");
    path_dirs[1] = NULL;
    path_count = 1;
    debug_print("Path inicializado: /bin\n");
}

void free_path(void) {
    for (int i = 0; i < path_count; i++) {
        if (path_dirs[i] != NULL) {
            free(path_dirs[i]);
            path_dirs[i] = NULL;
        }
    }
    path_count = 0;
}

char *find_executable(const char *command) {
    if (strchr(command, '/') != NULL) {
        if (access(command, X_OK) == 0) {
            debug_print("Encontrado ejecutable (ruta absoluta): %s\n", command);
            return strdup(command);
        }
        return NULL;
    }

    char full_path[MAXLINE];
    for (int i = 0; i < path_count; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path_dirs[i], command);
        debug_print("Buscando: %s\n", full_path);
        if (access(full_path, X_OK) == 0) {
            debug_print("Encontrado: %s\n", full_path);
            return strdup(full_path);
        }
    }

    return NULL;
}

// ============================================================================
// UTILS
// ============================================================================

void print_error(void) {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void free_args(char **args, int count) {
    for (int i = 0; i < count; i++) {
        if (args[i] != NULL) {
            free(args[i]);
            args[i] = NULL;
        }
    }
}

// ============================================================================
// TOKENIZER
// ============================================================================

int tokenize_line(char *line, char **args) {
    int arg_count = 0;

    debug_print("Tokenizando línea: '%s'\n", line);

    int len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' || 
           line[len - 1] == '\t' || line[len - 1] == ' ')) {
        line[len - 1] = '\0';
        len--;
    }

    debug_print("Después de limpiar: '%s'\n", line);

    int i = 0;
    while (line[i] != '\0') {
        while (line[i] != '\0' && isspace(line[i])) {
            i++;
        }

        if (line[i] == '\0') break;

        if (line[i] == '>' || line[i] == '&') {
            char op[2];
            op[0] = line[i];
            op[1] = '\0';
            args[arg_count++] = strdup(op);
            debug_print("Token operador: '%s'\n", op);
            i++;
            if (arg_count >= MAXARGS - 1) break;
            continue;
        }

        int start = i;
        while (line[i] != '\0' && !isspace(line[i]) && line[i] != '>' && line[i] != '&') {
            i++;
        }

        int token_len = i - start;
        char *token = (char *)malloc(token_len + 1);
        strncpy(token, &line[start], token_len);
        token[token_len] = '\0';

        char *trimmed = token;
        while (*trimmed && isspace(*trimmed)) trimmed++;
        if (trimmed != token) {
            memmove(token, trimmed, strlen(trimmed) + 1);
        }
        char *end = token + strlen(token) - 1;
        while (end > token && isspace(*end)) {
            *end = '\0';
            end--;
        }

        debug_print("Token argumento: '%s' (longitud: %zu)\n", token, strlen(token));
        args[arg_count++] = token;
        if (arg_count >= MAXARGS - 1) break;
    }

    args[arg_count] = NULL;
    debug_print("Total tokens: %d\n", arg_count);
    return arg_count;
}

// ============================================================================
// BUILT-IN COMMANDS
// ============================================================================

int builtin_exit(char **args, int arg_count) {
    debug_print("Ejecutando builtin: exit\n");
    if (arg_count != 1) {
        print_error();
        return 1;
    }
    exit(0);
}

int builtin_cd(char **args, int arg_count) {
    debug_print("Ejecutando builtin: cd\n");
    
    if (arg_count != 2) {
        debug_print("Error: cd requiere exactamente 1 argumento, se recibieron %d\n", arg_count - 1);
        print_error();
        return 1;
    }
    
    debug_print("Argumento original: '%s'\n", args[1]);
    debug_print("Longitud original: %zu\n", strlen(args[1]));
    
    // Mostrar cada carácter del argumento (para detectar caracteres ocultos)
    debug_print("Caracteres del argumento: ");
    for (int i = 0; i < strlen(args[1]); i++) {
        debug_print("%c (0x%02x) ", args[1][i], (unsigned char)args[1][i]);
    }
    debug_print("\n");
    
    char *dir = args[1];
    
    while (*dir && isspace(*dir)) dir++;
    
    char *end = dir + strlen(dir) - 1;
    while (end > dir && isspace(*end)) {
        *end = '\0';
        end--;
    }
    
    debug_print("Directorio después de trim: '%s'\n", dir);
    debug_print("Longitud después de trim: %zu\n", strlen(dir));
    
    if (strlen(dir) == 0) {
        debug_print("Error: directorio vacío después de trim\n");
        print_error();
        return 1;
    }
    
    debug_print("Llamando a chdir con: '%s'\n", dir);
    
    if (chdir(dir) != 0) {
        debug_print("chdir falló con error: %s (errno=%d)\n", strerror(errno), errno);
        print_error();
        return 1;
    }
    
    debug_print("chdir exitoso\n");
    return 1;
}

int builtin_path(char **args, int arg_count) {
    debug_print("Ejecutando builtin: path\n");
    free_path();

    for (int i = 1; i < arg_count; i++) {
        path_dirs[path_count++] = strdup(args[i]);
        debug_print("Path agregado: %s\n", args[i]);
    }

    path_dirs[path_count] = NULL;
    return 1;
}

int execute_builtin(char **args, int arg_count) {
    if (arg_count == 0) return 0;

    if (strcmp(args[0], "exit") == 0) {
        return builtin_exit(args, arg_count);
    }

    if (strcmp(args[0], "cd") == 0) {
        return builtin_cd(args, arg_count);
    }

    if (strcmp(args[0], "path") == 0) {
        return builtin_path(args, arg_count);
    }

    return 0;
}

// ============================================================================
// REDIRECTION
// ============================================================================

int check_redirection(char **args, int arg_count, int *redir_index, char **outfile) {
    int count = 0;
    *redir_index = -1;
    *outfile = NULL;

    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], ">") == 0) {
            count++;
            *redir_index = i;
        }
    }

    if (count == 0) return 0;
    if (count > 1) return -1;

    if (*redir_index == arg_count - 1) return -1;
    if (*redir_index + 2 < arg_count) return -1;

    *outfile = args[*redir_index + 1];
    return 1;
}

// ============================================================================
// EXTERNAL COMMAND EXECUTION
// ============================================================================

void execute_external(char **args, char *outfile) {
    debug_print("Ejecutando comando externo: %s\n", args[0]);
    
    char *full_path = find_executable(args[0]);
    if (full_path == NULL) {
        print_error();
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        print_error();
        free(full_path);
        return;
    }

    if (pid == 0) {
        if (outfile != NULL) {
            debug_print("Redirigiendo salida a: %s\n", outfile);
            int fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                print_error();
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        execv(full_path, args);
        print_error();
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
    }

    free(full_path);
}

// ============================================================================
// PARALLEL COMMANDS
// ============================================================================

int contains_ampersand(char **args, int arg_count) {
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], "&") == 0) return 1;
    }
    return 0;
}

int split_commands(char **args, int arg_count, char ***cmds, int *cmd_lengths) {
    int cmd_count = 0;
    int start = 0;

    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], "&") == 0) {
            if (i == start) return -1;
            cmds[cmd_count] = &args[start];
            cmd_lengths[cmd_count] = i - start;
            cmd_count++;
            start = i + 1;
        }
    }

    if (start >= arg_count) return -1;

    cmds[cmd_count] = &args[start];
    cmd_lengths[cmd_count] = arg_count - start;
    cmd_count++;

    return cmd_count;
}

void execute_parallel(char **args, int arg_count) {
    char **cmds[MAXCMDS];
    int cmd_lengths[MAXCMDS];

    int cmd_count = split_commands(args, arg_count, cmds, cmd_lengths);
    if (cmd_count < 0) {
        print_error();
        return;
    }

    char *full_paths[MAXCMDS];
    int redir_indices[MAXCMDS];
    char *outfiles[MAXCMDS];
    int valid = 1;
    
    for (int c = 0; c < cmd_count; c++) {
        full_paths[c] = NULL;
        cmds[c][cmd_lengths[c]] = NULL;

        if (strcmp(cmds[c][0], "exit") == 0 ||
            strcmp(cmds[c][0], "cd") == 0 ||
            strcmp(cmds[c][0], "path") == 0) {
            print_error();
            valid = 0;
            break;
        }

        int redir_status = check_redirection(cmds[c], cmd_lengths[c], 
                                              &redir_indices[c], &outfiles[c]);
        if (redir_status == -1) {
            print_error();
            valid = 0;
            break;
        }
        if (redir_status == 1) {
            cmds[c][redir_indices[c]] = NULL;
        }

        full_paths[c] = find_executable(cmds[c][0]);
        if (full_paths[c] == NULL) {
            print_error();
            valid = 0;
            break;
        }
    }

    if (!valid) {
        for (int c = 0; c < cmd_count; c++) {
            if (full_paths[c] != NULL) {
                free(full_paths[c]);
            }
        }
        return;
    }

    pid_t pids[MAXCMDS];
    for (int c = 0; c < cmd_count; c++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            print_error();
            for (int k = 0; k < c; k++) {
                waitpid(pids[k], NULL, 0);
            }
            for (int k = 0; k < cmd_count; k++) {
                if (full_paths[k] != NULL) {
                    free(full_paths[k]);
                }
            }
            return;
        }
        
        if (pid == 0) {
            if (outfiles[c] != NULL) {
                int fd = open(outfiles[c], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    print_error();
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
            
            execv(full_paths[c], cmds[c]);
            print_error();
            exit(1);
        } else {
            pids[c] = pid;
        }
    }

    for (int c = 0; c < cmd_count; c++) {
        waitpid(pids[c], NULL, 0);
        free(full_paths[c]);
    }
}

// ============================================================================
// MAIN COMMAND DISPATCH
// ============================================================================

void execute_command(char **args, int arg_count) {
    if (arg_count == 0) return;

    debug_print("Ejecutando comando: ");
    for (int i = 0; i < arg_count; i++) {
        debug_print("'%s' ", args[i]);
    }
    debug_print("\n");

    if (execute_builtin(args, arg_count)) {
        return;
    }

    if (contains_ampersand(args, arg_count)) {
        execute_parallel(args, arg_count);
        return;
    }

    int redir_index;
    char *outfile;

    int redir_status = check_redirection(args, arg_count, &redir_index, &outfile);

    if (redir_status == -1) {
        print_error();
        return;
    }

    if (redir_status == 1) {
        args[redir_index] = NULL;
        execute_external(args, outfile);
    } else {
        execute_external(args, NULL);
    }
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    int batch_mode = 0;

    debug_print("Wish shell iniciado\n");

    if (argc > 2) {
        print_error();
        exit(1);
    }

    if (argc == 2) {
        batch_mode = 1;
        debug_print("Modo batch: %s\n", argv[1]);
        input = fopen(argv[1], "r");
        if (input == NULL) {
            print_error();
            exit(1);
        }
    }

    init_path();

    char line[MAXLINE];
    char *args[MAXARGS];

    while (1) {
        if (!batch_mode) {
            printf("wish> ");
            fflush(stdout);
        }

        if (fgets(line, MAXLINE, input) == NULL) {
            debug_print("EOF encontrado, saliendo\n");
            exit(0);
        }

        int arg_count = tokenize_line(line, args);

        if (arg_count > 0) {
            execute_command(args, arg_count);
        }

        free_args(args, arg_count);
    }

    free_path();

    if (batch_mode) {
        fclose(input);
    }

    exit(0);
}