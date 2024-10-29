#include <sys/wait.h> // waitpid()
#include <unistd.h> // chdir(), fork(), exec(), pid_t
#include <stdlib.h> // malloc(), realloc(), free(), exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <stdio.h> // fprintf(), printf(), stderr, getchar(), perror()
#include <string.h> // strcmp(), strtok()

// Can be implemented with getline() much easier, but wanted to show how it actually works
#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void) { // We need *, because we return a pointer to the memory where our input is stored
    int bufsize = LSH_RL_BUFSIZE; // Setting initial size of a buffer
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize); // Allocating memory for the buffer that can contain 1024 characters
    int c; // Variable for character. We use int instead of charm because we want to check for EOF. And EOF is integer

    if (!buffer) { // If we can't allocate memory with such size
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar(); // Read a character (as integer)

        if (c == EOF || c == '\n') { // If EOF (ctrl+d/ctrl+z) or ENTER we return current buffer (command we got so far).
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c; // Otherwise we add our character to out string
        }
        position++;

        if (position >= bufsize) { // If number of characters exceeds our bufersize we add another buffer and reallocate memory
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) { // If we can't allocate memory with such size
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) { // If can't execute
            perror("lsh"); //
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) { // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED); // Wait for the child proccess to change
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

// Built-in functions
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[] = { // Array of pointers to characters
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: excpected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char **args) {
    int i;
    printf("HManDEV's shell\n");
    printf("Created by Brennan's tutorial");
    printf("Built-in commands:\n");
    for (i = 0; i < lsh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for info on other commands");
    return 1;
}

int lsh_exit(char **args) {
    return 0;
}

int lsh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}

void lsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line(); // We read command
        args = lsh_split_line(line); // We read arguments of the command
        status = lsh_execute(args); // We execute and give status code

        free(line); // We free our command
        free(args); // We free all arguments
    } while (status); // Run until we have executed our command
}

int main(int argc, char **argv) {
    // Load config

    // Run command loop
    lsh_loop();

    // Shutdown

    return EXIT_SUCCESS;
}