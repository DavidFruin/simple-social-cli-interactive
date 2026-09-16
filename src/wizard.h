#ifndef WIZARD_H
#define WIZARD_H

typedef enum { FT_TEXT, FT_PASSWORD, FT_INT } field_type_t;

typedef struct {
    const char *label;   /* e.g. "Email", "Limit" -- used in the prompt */
    field_type_t type;   /* TEXT / PASSWORD (hidden) / INT (numeric hint only) */
    int optional;         /* 1 => blank input allowed (skip / use default) */
    const char *hint;     /* shown after label, e.g. "(optional, default 25)", or NULL */
} field_spec_t;

#define WIZARD_MAX_FIELDS 4
#define WIZARD_ANSWER_MAX 5000 /* covers api_post_t.text / api_comment_t.text (5000) */

typedef struct {
    char text[WIZARD_ANSWER_MAX];
    int provided; /* 0 if optional field was left blank */
} wizard_answer_t;

/* Prompts for each field in order. Required fields reprompt on blank input.
 * Optional fields accept blank => provided=0, text[0]='\0'.
 * Returns 0 on success (answers[0..field_count-1] filled), -1 if the user
 * hit EOF (Ctrl+D) mid-collection (caller should abort the command, not the REPL). */
int wizard_collect(const field_spec_t *fields, int field_count, wizard_answer_t *answers);

#endif
