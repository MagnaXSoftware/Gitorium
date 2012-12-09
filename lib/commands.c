#include "common.h"
#include "commands.h"

int is_remote_command_valid(char *cmd) {
    CMD_VALID("info")
    CMD_VALID("list")
    CMD_VALID("list-repos")
    CMD_VALID("list-commands")
    CMD_VALID("help")
    return 0;
}

void call_remote_command(char *cmd, char **argv) {
    CALL_CMD("info", command_info)
    CALL_CMD("list", command_list)
    CALL_CMD("list-repos", command_list_repos)
    CALL_CMD("list-commands", command_list_commands)
    CALL_CMD("help", command_help)
}

void command_help(char **argv) {
    HELP_PRINT("Gitorium help")
    if (argv == (char **)NULL) {
        HELP_BLANK
        HELP_PRINT("To see a list of the available commands, type 'list commands'.")
        HELP_PRINT("To see help about a specific command, type 'help ' followed by the command.")
        HELP_BLANK
        HELP_PRINT("Two part commands, e.g. 'list commands', can also be called as a one part command: 'list-commands'.")
    } else {
        CALL_HELP("info", info)
        CALL_HELP("list", list)
        CALL_HELP("list-repos", list_repos)
        CALL_HELP("list-commands", list_commands)
        HELP_BLANK
        HELP_PRINT("There is no help for the keyword you specified.")
    }
}

void command_info(char **argv) {
    puts("Gitorium version "GITORIUM_VER_FULL);
    puts("Copyright (c) AfroSoft & contributors");
}

void command_help_info(char **argv) {
    HELP_USAGE("info")
    HELP_BLANK
    HELP_PRINT("Give information about this gitorium version")
}

void command_list(char **argv) {
    if (argv == NULL) {
        fprintf(stderr, "You must indicate what type of list you wish.\n");
        fprintf(stderr, "For help, type 'help list'.\n");
        return;
    }
    CALL_ARGV(argv[0], "repos", command_list_repos)
    CALL_ARGV(argv[0], "commands", command_list_commands)
}

void command_help_list(char **argv) {
    HELP_USAGE("list item")
    HELP_BLANK
    HELP_PRINT("List the item")
    HELP_BLANK
    HELP_PRINT("Values of item:")
    HELP_PRINT("\trepos    - List repositories")
    HELP_PRINT("\tcommands - List commands")
}

void command_list_repos(char **argv) {
    puts("Here are all the repositories we know:");
}

void command_help_list_repos(char **argv) {
    HELP_USAGE("list repos OR list-repos")
    HELP_BLANK
    HELP_PRINT("List the repositories")
}

void command_list_commands(char **argv) {
    puts("Here are all the commands we know:");
}

void command_help_list_commands(char **argv) {
    HELP_USAGE("list commands OR list-commands")
    HELP_BLANK
    HELP_PRINT("List the commands")
}
