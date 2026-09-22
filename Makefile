CC = gcc

# Vendored as a git submodule rather than a sibling checkout, so this repo
# is self-contained: `git clone --recursive` + `make` is the whole story,
# no separate simple-social-cli clone required.
CLI_DIR = vendor/simple-social-cli
LIBSS   = $(CLI_DIR)/lib/libss.a

CFLAGS = -Wall -Wextra -O2 -I$(CLI_DIR)/lib
# Static archive linked directly (not -lss + rpath): the built binary ends
# up a single self-contained file that works wherever it's copied or
# symlinked, e.g. onto PATH via `make install`. -lcurl still needs
# vendor/simple-social-cli's own vendor/ dir on the search path, since
# that's where its vendor-links step puts the libcurl.so symlink dev
# packages don't always provide.
LDFLAGS = $(LIBSS) -L$(CLI_DIR)/vendor -lcurl

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

$(BIN): $(OBJS) $(LIBSS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(BIN)

install: $(BIN)
	install -m 755 $(BIN) /usr/local/bin/ssic

uninstall:
	rm -f /usr/local/bin/ssic

.PHONY: all clean check-lib install uninstall
