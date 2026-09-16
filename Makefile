CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../simple-social-cli/lib
LDFLAGS = -L../simple-social-cli/lib -lss -Wl,-rpath,'$$ORIGIN/../simple-social-cli/lib'

SRCS = src/main.c src/repl.c src/wizard.c src/commands.c src/output.c src/input.c
OBJS = $(SRCS:.c=.o)
BIN = simple-social-cli-interactive
LIBSS = ../simple-social-cli/lib/libss.so

all: check-lib $(BIN)

check-lib:
	@if [ ! -f $(LIBSS) ]; then \
		echo "error: $(LIBSS) not found."; \
		echo "Build the sibling project first: (cd ../simple-social-cli && make)"; \
		exit 1; \
	fi

$(BIN): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(BIN)

install: $(BIN)
	install -m 755 $(BIN) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(BIN)

.PHONY: all clean check-lib install uninstall
