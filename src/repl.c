#include "repl.h"
#include "commands.h"
#include "wizard.h"
#include "input.h"
#include "output.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) *end-- = '\0';
    return s;
}

static void print_help(void) {
    const char *last_group = NULL;
    for (int i = 0; i < g_command_count; i++) {
        const command_spec_t *c = &g_commands[i];
        if (!last_group || strcmp(last_group, c->group) != 0) {
            printf("\n%s%s%s\n", C_BOLD, c->group, C_RESET);
            last_group = c->group;
        }
        printf("  %-16s%s\n", c->name, c->summary);
    }
    printf("\n%sMETA%s\n", C_BOLD, C_RESET);
    printf("  %-16s%s\n", "help", "Show this help");
    printf("  %-16s%s\n", "exit / quit", "Leave the program");
    printf("\n");
}

void repl_run(ss_state_t *state) {
    printf("Simple Social CLI (interactive)\n");
    printf("Type 'help' for a list of commands, 'exit' or 'quit' to leave.\n\n");

    char line[128];
    for (;;) {
        if (ss_state_is_logged_in(state))
            printf("simple-social (%s)> ", state->user.email);
        else
            printf("simple-social> ");
        fflush(stdout);

        int rc = input_read_line(line, sizeof(line));
        if (rc < 0) {
            printf("\n");
            break;
        }
        char *cmd_name = trim(line);
        if (cmd_name[0] == '\0') continue;

        if (strcmp(cmd_name, "exit") == 0 || strcmp(cmd_name, "quit") == 0) break;
        if (strcmp(cmd_name, "help") == 0) { print_help(); continue; }

        const command_spec_t *cmd = NULL;
        for (int i = 0; i < g_command_count; i++) {
            if (strcmp(cmd_name, g_commands[i].name) == 0) { cmd = &g_commands[i]; break; }
        }
        if (!cmd) {
            printf("Unknown command: %s\n", cmd_name);
            printf("Type 'help' for a list of commands.\n");
            continue;
        }

        if (cmd->requires_auth && !ss_state_is_logged_in(state)) {
            print_error("Not logged in. Run the 'login' command first.");
            continue;
        }

        wizard_answer_t answers[WIZARD_MAX_FIELDS];
        if (cmd->field_count > 0) {
            if (wizard_collect(cmd->fields, cmd->field_count, answers) != 0) {
                printf("Cancelled.\n");
                continue;
            }
        }
        cmd->handler(state, answers);
    }

    printf("Goodbye.\n");
}
