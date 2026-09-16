#include "input.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

static void drain_rest_of_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

/* Strips a trailing '\n' (and preceding '\r', if present) from buf. */
static void strip_newline(char *buf) {
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[--len] = '\0';
        if (len > 0 && buf[len - 1] == '\r') buf[--len] = '\0';
    }
}

int input_read_line(char *buf, size_t bufsize) {
    if (bufsize == 0) return -1;
    if (fgets(buf, (int)bufsize, stdin) == NULL) {
        buf[0] = '\0';
        return -1;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] != '\n' && !feof(stdin)) {
        /* line was longer than the buffer -- discard the remainder */
        drain_rest_of_line();
    }
    strip_newline(buf);
    return (int)strlen(buf);
}

static struct termios g_saved_termios;
static volatile sig_atomic_t g_hidden_read_active = 0;
static struct sigaction g_prev_sigint;

static void hidden_read_sigint_handler(int sig) {
    (void)sig;
    if (g_hidden_read_active) {
        tcsetattr(STDIN_FILENO, TCSANOW, &g_saved_termios);
        write(STDOUT_FILENO, "\n", 1);
    }
    _exit(130);
}

int input_read_line_hidden(char *buf, size_t bufsize) {
    if (bufsize == 0) return -1;

    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &g_saved_termios) != 0) {
        /* not a real terminal (e.g. piped input) -- fall back to visible read */
        return input_read_line(buf, bufsize);
    }
    raw = g_saved_termios;
    raw.c_lflag &= ~ECHO; /* keep ICANON so backspace/line-editing still work */

    struct sigaction sa;
    sa.sa_handler = hidden_read_sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, &g_prev_sigint);

    g_hidden_read_active = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    int rc;
    if (fgets(buf, (int)bufsize, stdin) == NULL) {
        buf[0] = '\0';
        rc = -1;
    } else {
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] != '\n' && !feof(stdin)) {
            drain_rest_of_line();
        }
        strip_newline(buf);
        rc = (int)strlen(buf);
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_saved_termios);
    g_hidden_read_active = 0;
    sigaction(SIGINT, &g_prev_sigint, NULL);

    printf("\n"); /* Enter's newline isn't echoed with ECHO off */
    fflush(stdout);

    return rc;
}
