#include "wizard.h"
#include "input.h"
#include <stdio.h>
#include <string.h>

int wizard_collect(const field_spec_t *fields, int field_count, wizard_answer_t *answers) {
    for (int i = 0; i < field_count; i++) {
        const field_spec_t *f = &fields[i];
        for (;;) {
            printf("%s%s%s: ", f->label, f->hint ? " " : "", f->hint ? f->hint : "");
            fflush(stdout);

            int rc = (f->type == FT_PASSWORD)
                ? input_read_line_hidden(answers[i].text, sizeof(answers[i].text))
                : input_read_line(answers[i].text, sizeof(answers[i].text));

            if (rc < 0) return -1; /* EOF mid-wizard -- caller cancels the command */

            if (answers[i].text[0] == '\0') {
                if (f->optional) {
                    answers[i].provided = 0;
                    break;
                }
                printf("This field is required.\n");
                continue; /* reprompt the same field */
            }

            answers[i].provided = 1;
            break;
        }
    }
    return 0;
}
