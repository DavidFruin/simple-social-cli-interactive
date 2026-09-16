#ifndef INPUT_H
#define INPUT_H
#include <stddef.h>

/* Reads one line of visible input into buf (trims trailing \n, NUL-terminates).
 * Returns line length (>=0) or -1 on EOF/error (caller treats as "cancel"). */
int input_read_line(char *buf, size_t bufsize);

/* Same, but disables terminal echo for the duration of the read (password entry).
 * Restores terminal state before returning, including on EOF, on read error,
 * and on SIGINT (Ctrl+C) received while reading. */
int input_read_line_hidden(char *buf, size_t bufsize);

#endif
