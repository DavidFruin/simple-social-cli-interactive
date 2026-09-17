#include "commands.h"
#include "output.h"
#include "ss_api.h"
#include "ss_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int answer_int(const wizard_answer_t *a, int def) {
    return a->provided ? atoi(a->text) : def;
}

/* ---------------- Auth ---------------- */

static void cmd_login(ss_state_t *state, const wizard_answer_t *answers) {
    const char *email = answers[0].text;
    const char *password = answers[1].text;

    char jwt[1024] = {0};
    int user_id = 0;
    if (api_login(email, password, jwt, sizeof(jwt), &user_id) != 0) {
        print_error(api_get_last_error());
        return;
    }
    ss_state_set_jwt(state, jwt);
    api_set_jwt(jwt);
    api_set_user_id(user_id);
    ss_state_save_jwt(state);

    char email_out[256] = {0};
    char created[32] = {0};
    api_get_my_info(&user_id, email_out, sizeof(email_out), created, sizeof(created));
    ss_state_set_user(state, user_id, email_out, created);
    ss_state_save_user(state);

    char msg[512];
    snprintf(msg, sizeof(msg), "Logged in as %s (ID: %d)", email_out, user_id);
    print_success(msg);
}

static void cmd_logout(ss_state_t *state, const wizard_answer_t *answers) {
    (void)answers;
    ss_state_clear(state);
    char path[512];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(path, sizeof(path), "%s/.simple-social-cli/jwt.txt", home);
    unlink(path);
    snprintf(path, sizeof(path), "%s/.simple-social-cli/user.json", home);
    unlink(path);
    print_success("Logged out.");
}

static void cmd_whoami(ss_state_t *state, const wizard_answer_t *answers) {
    (void)answers;
    print_user_info(state->user.user_id, state->user.email, state->user.created_at);
}

static void cmd_register(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *email = answers[0].text;
    const char *otp = answers[1].text;
    const char *password = answers[2].text;
    const char *confirm = answers[3].text;

    if (api_register_verify_otp(email, otp) != 0) {
        print_error(api_get_last_error());
        return;
    }
    if (api_register_finish(email, password, confirm) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_success("Registration complete. You can now login.");
}

static void cmd_send_otp(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *email = answers[0].text;
    if (api_register_send_otp(email) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "OTP sent to %s", email);
    print_success(msg);
}

static void cmd_reset_password(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *email = answers[0].text;
    const char *otp = answers[1].text;
    const char *password = answers[2].text;
    const char *confirm = answers[3].text;

    if (api_verify_otp(email, otp) != 0) {
        print_error(api_get_last_error());
        return;
    }
    if (api_reset_password(email, password, confirm) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_success("Password reset. You can now login.");
}

/* ---------------- Posts ---------------- */

static void cmd_feed(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    int limit = answer_int(&answers[0], 25);
    int offset = answer_int(&answers[1], 0);
    api_posts_result_t result;
    if (api_fetch_followed_posts(offset, limit, &result) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_posts(&result);
}

static void cmd_posts(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    int limit = answer_int(&answers[1], 25);
    int offset = answer_int(&answers[2], 0);
    api_posts_result_t result;
    int rc;
    if (answers[0].provided) rc = api_get_user_posts(atoi(answers[0].text), offset, limit, &result);
    else rc = api_get_my_posts(offset, limit, &result);
    if (rc != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_posts(&result);
}

static void cmd_post(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    api_post_t post;
    if (api_get_post_by_id(post_id, &post) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_post(&post);

    api_comments_result_t comments;
    if (api_get_post_comments(post_id, 0, 25, &comments) == 0 && comments.count > 0) {
        printf("\n%sComments (%d):%s\n", C_BOLD, comments.count, C_RESET);
        print_comments(&comments);
    }
}

static void cmd_create(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *text = answers[0].text;
    char media_url[512] = {0};
    int media_id = 0;
    const char *media_url_ptr = NULL;

    if (answers[1].provided) {
        if (api_upload_media_with_id(answers[1].text, media_url, sizeof(media_url), &media_id) != 0) {
            print_error(api_get_last_error());
            return;
        }
        media_url_ptr = media_url;
    }

    char post_id[64] = {0};
    if (api_create_post(text, media_url_ptr, post_id, sizeof(post_id)) != 0) {
        if (media_id) api_delete_media(media_id);
        print_error(api_get_last_error());
        return;
    }

    char msg[600];
    if (media_url_ptr) snprintf(msg, sizeof(msg), "Created post %s (media: %s)", post_id, media_url);
    else snprintf(msg, sizeof(msg), "Created post %s", post_id);
    print_success(msg);
}

static void cmd_delete_post(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    if (api_delete_post(post_id) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Deleted post %s", post_id);
    print_success(msg);
}

static void cmd_like(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    if (api_like_post(post_id) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Liked post %s", post_id);
    print_success(msg);
}

static void cmd_unlike(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    if (api_unlike_post(post_id) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Unliked post %s", post_id);
    print_success(msg);
}

static void cmd_likes(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    char out[16384] = {0};
    if (api_get_post_likes(post_id, out, sizeof(out)) != 0) {
        print_error(api_get_last_error());
        return;
    }
    api_users_result_t res;
    res.count = 0;
    const char *arr_start, *arr_end;
    if (json_get_array(out, "likes", &arr_start, &arr_end) == 0 ||
        json_get_array(out, "users", &arr_start, &arr_end) == 0) {
        int len = json_array_len(arr_start, arr_end);
        for (int i = 0; i < len && i < 256; i++) {
            const char *is, *ie;
            if (json_array_get_item(arr_start, arr_end, i, &is, &ie) != 0) break;
            int l = (int)(ie - is);
            char item[4096];
            if (l >= (int)sizeof(item)) l = sizeof(item) - 1;
            memcpy(item, is, l);
            item[l] = '\0';
            api_user_t *u = &res.users[res.count];
            if (json_get_int(item, "userId", &u->id) != 0) json_get_int(item, "id", &u->id);
            json_get_string(item, "email", u->email, sizeof(u->email));
            if (u->email[0] == '\0') json_get_string(item, "userEmail", u->email, sizeof(u->email));
            json_get_string(item, "created_at", u->created_at, sizeof(u->created_at));
            res.count++;
        }
        print_users(&res);
        return;
    }
    printf("%s\n", out);
}

/* ---------------- Comments ---------------- */

static void cmd_comments(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    int offset = answer_int(&answers[1], 0);
    api_comments_result_t result;
    if (api_get_post_comments(post_id, offset, 25, &result) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_comments(&result);
}

static void cmd_comment(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *post_id = answers[0].text;
    const char *text = answers[1].text;
    int comment_id = 0;
    if (api_create_comment(post_id, text, &comment_id) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Comment %d added to %s", comment_id, post_id);
    print_success(msg);
}

static void cmd_delete_comment(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *comment_id = answers[0].text;
    if (api_delete_comment(atoi(comment_id)) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Deleted comment %s", comment_id);
    print_success(msg);
}

/* ---------------- Users / follow ---------------- */

static void cmd_users(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    (void)answers;
    api_users_result_t result;
    if (api_get_users(&result) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_users(&result);
}

static void cmd_profile(ss_state_t *state, const wizard_answer_t *answers) {
    int user_id = answers[0].provided ? atoi(answers[0].text) : state->user.user_id;

    char email[256] = {0};
    char created[32] = {0};
    if (api_get_user_info(user_id, email, sizeof(email), created, sizeof(created)) != 0) {
        print_error(api_get_last_error());
        return;
    }

    api_users_result_t followers, following;
    api_get_my_followers(user_id, &followers);
    api_get_my_follows(user_id, &following);
    print_profile(user_id, email, created, followers.count, following.count);

    if (user_id != state->user.user_id) {
        int is_following = 0;
        api_is_following(user_id, &is_following);
        printf("%sFollowing:%s  %s\n", C_BOLD, C_RESET, is_following ? "Yes" : "No");
    }
}

static void cmd_follow(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *user_id = answers[0].text;
    if (api_follow_user(atoi(user_id)) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Followed user %s", user_id);
    print_success(msg);
}

static void cmd_unfollow(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    const char *user_id = answers[0].text;
    if (api_unfollow_user(atoi(user_id)) != 0) {
        print_error(api_get_last_error());
        return;
    }
    char msg[5100];
    snprintf(msg, sizeof(msg), "Unfollowed user %s", user_id);
    print_success(msg);
}

static void cmd_followers(ss_state_t *state, const wizard_answer_t *answers) {
    int user_id = answers[0].provided ? atoi(answers[0].text) : state->user.user_id;
    api_users_result_t result;
    if (api_get_my_followers(user_id, &result) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_users(&result);
}

static void cmd_following(ss_state_t *state, const wizard_answer_t *answers) {
    int user_id = answers[0].provided ? atoi(answers[0].text) : state->user.user_id;
    api_users_result_t result;
    if (api_get_my_follows(user_id, &result) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_users(&result);
}

/* ---------------- Notifications ---------------- */

static void cmd_notifications(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    int offset = answer_int(&answers[0], 0);
    api_notifications_result_t result;
    if (api_get_notifications(offset, &result) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_notifications(&result);
}

static void cmd_notify_count(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    (void)answers;
    int count = 0;
    if (api_get_unseen_notification_count(&count) != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_count(count);
}

static void cmd_mark_seen(ss_state_t *state, const wizard_answer_t *answers) {
    (void)state;
    (void)answers;
    if (api_mark_notifications_seen() != 0) {
        print_error(api_get_last_error());
        return;
    }
    print_success("Notifications marked as seen.");
}

/* ---------------- Account ---------------- */

static void cmd_delete_account(ss_state_t *state, const wizard_answer_t *answers) {
    const char *password = answers[0].text;
    if (api_delete_account(password) != 0) {
        print_error(api_get_last_error());
        return;
    }
    ss_state_clear(state);
    print_success("Account deleted.");
}

/* ---------------- Command table ---------------- */

const command_spec_t g_commands[] = {
    {"login", "Login", "AUTH", 0,
     {{"Email", FT_TEXT, 0, NULL}, {"Password", FT_PASSWORD, 0, NULL}}, 2, cmd_login},
    {"logout", "Logout", "AUTH", 0, {{0}}, 0, cmd_logout},
    {"whoami", "Show current user", "AUTH", 1, {{0}}, 0, cmd_whoami},
    {"register", "Register with OTP", "AUTH", 0,
     {{"Email", FT_TEXT, 0, NULL}, {"OTP", FT_TEXT, 0, NULL},
      {"Password", FT_PASSWORD, 0, NULL}, {"Confirm Password", FT_PASSWORD, 0, NULL}},
     4, cmd_register},
    {"send-otp", "Send registration OTP", "AUTH", 0,
     {{"Email", FT_TEXT, 0, NULL}}, 1, cmd_send_otp},
    {"reset-password", "Reset password via OTP", "AUTH", 0,
     {{"Email", FT_TEXT, 0, NULL}, {"OTP", FT_TEXT, 0, NULL},
      {"Password", FT_PASSWORD, 0, NULL}, {"Confirm Password", FT_PASSWORD, 0, NULL}},
     4, cmd_reset_password},

    {"feed", "Show feed", "POSTS", 1,
     {{"Limit", FT_INT, 1, "(optional, default 25)"}, {"Offset", FT_INT, 1, "(optional, default 0)"}},
     2, cmd_feed},
    {"posts", "List user posts", "POSTS", 1,
     {{"User ID", FT_INT, 1, "(optional, default: yourself)"},
      {"Limit", FT_INT, 1, "(optional, default 25)"}, {"Offset", FT_INT, 1, "(optional, default 0)"}},
     3, cmd_posts},
    {"post", "Show post + comments", "POSTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}}, 1, cmd_post},
    {"create", "Create post", "POSTS", 1,
     {{"Text", FT_TEXT, 0, NULL}, {"Media file path", FT_TEXT, 1, "(optional)"}}, 2, cmd_create},
    {"delete", "Delete post", "POSTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}}, 1, cmd_delete_post},
    {"like", "Like post", "POSTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}}, 1, cmd_like},
    {"unlike", "Unlike post", "POSTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}}, 1, cmd_unlike},
    {"likes", "List likes for post", "POSTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}}, 1, cmd_likes},

    {"comments", "Show comments", "COMMENTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}, {"Offset", FT_INT, 1, "(optional, default 0)"}}, 2, cmd_comments},
    {"comment", "Add comment", "COMMENTS", 1,
     {{"Post ID", FT_TEXT, 0, NULL}, {"Text", FT_TEXT, 0, NULL}}, 2, cmd_comment},
    {"delete-comment", "Delete comment", "COMMENTS", 1,
     {{"Comment ID", FT_INT, 0, NULL}}, 1, cmd_delete_comment},

    {"users", "List all users", "USERS", 1, {{0}}, 0, cmd_users},
    {"profile", "Show profile", "USERS", 1,
     {{"User ID", FT_INT, 1, "(optional, default: yourself)"}}, 1, cmd_profile},
    {"follow", "Follow user", "USERS", 1,
     {{"User ID", FT_INT, 0, NULL}}, 1, cmd_follow},
    {"unfollow", "Unfollow user", "USERS", 1,
     {{"User ID", FT_INT, 0, NULL}}, 1, cmd_unfollow},
    {"followers", "List followers", "USERS", 1,
     {{"User ID", FT_INT, 1, "(optional, default: yourself)"}}, 1, cmd_followers},
    {"following", "List following", "USERS", 1,
     {{"User ID", FT_INT, 1, "(optional, default: yourself)"}}, 1, cmd_following},

    {"notifications", "Show notifications", "NOTIFICATIONS", 1,
     {{"Offset", FT_INT, 1, "(optional, default 0)"}}, 1, cmd_notifications},
    {"notify-count", "Unseen notification count", "NOTIFICATIONS", 1, {{0}}, 0, cmd_notify_count},
    {"mark-seen", "Mark notifications seen", "NOTIFICATIONS", 1, {{0}}, 0, cmd_mark_seen},

    {"delete-account", "Delete your account", "ACCOUNT", 1,
     {{"Password", FT_PASSWORD, 0, NULL}}, 1, cmd_delete_account},
};

const int g_command_count = sizeof(g_commands) / sizeof(g_commands[0]);
