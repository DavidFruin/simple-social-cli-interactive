CC = gcc

# Vendored as a git submodule rather than a sibling checkout, so this repo
# is self-contained: `git clone --recursive` + `make` is the whole story,
# no separate simple-social-cli clone required.
CLI_DIR = vendor/simple-social-cli
LIBSS   = $(CLI_DIR)/lib/libss.so

CFLAGS  = -Wall -Wextra -O2 -I$(CLI_DIR)/lib
LDFLAGS = -L$(CLI_DIR)/lib -lss -Wl,-rpath,'$$ORIGIN/$(CLI_DIR)/lib'

SRCS = src/main.c src/repl.c src/wizard.c src/commands.c src/output.c src/input.c
OBJS = $(SRCS:.c=.o)
BIN = simple-social-cli-interactive

all: check-lib $(BIN)

check-lib:
	@if [ ! -f $(CLI_DIR)/Makefile ]; then \
		echo "error: $(CLI_DIR) is empty - the submodule wasn't checked out."; \
		echo "Run: git submodule update --init --recursive"; \
		exit 1; \
	fi
	@if [ ! -f $(LIBSS) ]; then \
		echo "Building vendored simple-social-cli library..."; \
		$(MAKE) -C $(CLI_DIR); \
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
