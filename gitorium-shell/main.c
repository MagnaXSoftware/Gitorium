#include <stdio.h>
#include <stdlib.h>
#include "../gitorium/gitorium.h"

#define LINE_BUFFER_SIZE 50

void getline(char **linep) {
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

void split_cmd(char **cmd, char **arg, char *str) {
    char *p, *args = malloc(1);

    *cmd = strtok(str, " ");

    while ((p = strtok(NULL, " "))) {
        char *argsn = realloc(args, sizeof args + 2 + sizeof p);

        if (argsn == NULL)
            exit(-2);

        args = argsn;
        strcat(args, p);
        strcat(args, " ");
    }

    if (strlen(args) > 0)
        args[strlen(args)-1] = '\0';

    *arg = args;
}

void split_args(char ***args, char *str) {
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

void run_shell(void) {
    int done = 0;

    do {
        char *line, *cmd, *argline, **args;

        fprintf(stderr, "gitorium> ");
        getline(&line);

        if (line[0] == '\0') {
            free(line);
            break;
        }

        split_cmd(&cmd, &argline, line);

        if (!strcmp(cmd, "quit") || !strcmp(cmd, "exit")) {
            done = 1;
        } else if (is_remote_command_valid(cmd)) {
            if (argline[0] != '\0')
                split_args(&args, argline);

            call_remote_command(cmd, args);
        } else
            fprintf(stderr, "The command does not exist.\n");

        free(line);
        if (argline != NULL)
            free(argline);
    } while (!done);
}

int main() {
    run_shell();
    return 0;
}
