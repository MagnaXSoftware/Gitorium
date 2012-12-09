#include <stdio.h>
#include <stdlib.h>
#include "gitorium.h"

#define LINE_BUFFER_SIZE 50

static void get_line(char **linep) {
    char *line = malloc(LINE_BUFFER_SIZE);
    int len = LINE_BUFFER_SIZE, c;
    *linep = line;

    if(line == NULL)
        return;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF || c == '\n')
            break;

        if(--len == 0) {
            char *linen = realloc(*linep, sizeof *linep + LINE_BUFFER_SIZE);
            if(linen == NULL)
                exit(-2);

            len = LINE_BUFFER_SIZE;
            line = linen + (line - *linep);
            *linep = linen;
        }

        *line++ = c;
    }

    *line = '\0';
}

static void split_args(char ***args, char *str) {
    char **res = NULL, *p = strtok(str, " ");
    int n_spaces = 0;

    while (p) {
        res = realloc(res, sizeof (char*) * ++n_spaces);

        if (res == NULL)
            exit(-2);

        res[n_spaces-1] = p;
        p = strtok(NULL, " ");
    }

    res = realloc(res, sizeof(char*) * (n_spaces+1));
    res[n_spaces] = 0;
    *args = res;
}

static void run_shell(char *user) {
    int done = 0;

    do {
        char *line, **args;

        fprintf(stderr, "gitorium (%s)> ", user);
        get_line(&line);

        if (line[0] == '\0') {
            free(line);
            break;
        }

        split_args(&args, line);

        if (!strcmp(args[0], "quit") || !strcmp(args[0], "exit")) {
            done = 1;
//        } else if (is_remote_command_valid(args[0])) {
//            call_remote_command(user, args);
        } else
            fprintf(stderr, "The command does not exist.\n");

        free(line);
        free(args);
    } while (!done);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        // Someone tried to call us directly
	fprintf(stderr, "You must call this from a shell.\n");
        exit(-1);
    } else if (argc == 2) {
        // Running interactive
        run_shell(argv[1]);
    } else if (argc > 2) {
        // Running non-interactive
        fprintf(stderr, "Sorry, I can't handle this yet.\n");
    }
    return 0;
}
