#ifndef COMMANDS_H
#define COMMANDS_H

#include "wizard.h"
#include "ss_state.h"

typedef struct {
    const char *name;          /* word typed at the prompt */
    const char *summary;       /* one-line help text */
    const char *group;         /* help-listing section, e.g. "AUTH" */
    int requires_auth;         /* checked BEFORE running the wizard */
    field_spec_t fields[WIZARD_MAX_FIELDS];
    int field_count;
    void (*handler)(ss_state_t *state, const wizard_answer_t *answers);
} command_spec_t;

extern const command_spec_t g_commands[];
extern const int g_command_count;

#endif
