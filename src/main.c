#include "ss_state.h"
#include "ss_config.h"
#include "ss_api.h"
#include "output.h"
#include "repl.h"
#include <string.h>

int main(void) {
    ss_state_t state;
    ss_state_init(&state);

    config_t cfg;
    config_load(&cfg);
    api_init();
    output_init();

    if (ss_state_load_jwt(&state) == 0) {
        api_set_jwt(state.jwt);
        int id = 0;
        char email[256] = {0};
        char created[32] = {0};
        if (api_get_my_info(&id, email, sizeof(email), created, sizeof(created)) == 0) {
            ss_state_set_user(&state, id, email, created);
            api_set_user_id(id);
        } else {
            ss_state_clear(&state);
        }
    }

    repl_run(&state);

    api_cleanup();
    return 0;
}
