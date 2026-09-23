#include "ss_state.h"
#include "ss_config.h"
#include "ss_api.h"
#include "output.h"
#include "repl.h"
#include <string.h>

static ss_state_t state;

// Persists an access token the library renewed on its own, so the session
// survives without ever asking for the password again.
static void on_token_refreshed(const char *jwt) {
    ss_state_set_jwt(&state, jwt);
    ss_state_save_jwt(&state);
}

int main(void) {
    ss_state_init(&state);

    // Its own app name, so this tool holds a session separate from the CLI
    // and the TUI and shows up as its own device.
    ss_state_set_app("wiz");
    api_set_user_agent("simple-social-cli-interactive");
    api_set_token_refreshed_cb(on_token_refreshed);

    config_t cfg;
    config_load(&cfg);
    api_init();
    output_init();

    if (ss_state_load_refresh(&state) == 0) api_set_refresh_token(state.refresh);

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
