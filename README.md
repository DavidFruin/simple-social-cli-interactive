# Simple Social CLI — Interactive

A wizard-style interactive front end for the Simple Social API, built on the same
library as `simple-social-cli` and `simple-social-tui`. Each tool keeps its own
session under `~/.simple-social-cli/<tool>/` - logging in with one does not log
you in on the others.

`simple-social-cli` is vendored in as a git submodule under `vendor/`, not a sibling
checkout - this repo is self-contained. It never modifies the vendored copy.

## Build

```
git clone --recursive https://github.com/DavidFruin/simple-social-cli-interactive.git
cd simple-social-cli-interactive && make
```

`make` builds the vendored `simple-social-cli` library automatically if it isn't
already built. If you cloned without `--recursive`, run
`git submodule update --init --recursive` first.

The built binary is statically linked against the vendored library (no `.so` to
keep track of), so it works wherever it ends up - copied, symlinked, whatever.
`sudo make install` puts it on your PATH as `sswiz`; `sudo make uninstall`
removes it.

## Run

```
./simple-social-cli-interactive
```

Or, once installed: `sswiz`, from anywhere.

You'll get a prompt. Type a single command word (e.g. `login`, `feed`, `create`, `help`,
`exit`) and the tool will prompt you for each field it needs, one at a time. Password fields
are entered with the terminal echo hidden.

Type just the command word - this tool doesn't take arguments on the same line
(e.g. `login me@x.com` won't work; type `login` and answer the `Email:` /
`Password:` prompts that follow).

## Tests

```
TEST_EMAIL=you@example.com TEST_PASSWORD='...' python3 tests/test_wizard.py
```

Drives the built binary over a real pipe against a real account and asserts on
what it prints - the same experience typing into it by hand would give. No
default credentials, since a fallback account that silently stops existing
would make every test "pass" by failing to log in.

Defaults to `dev.davidfruin.com` (override with `TEST_BASE_URL`) since this
creates and deletes real posts. Runs the binary with `HOME` pointed at a
scratch directory, so it never touches your real
`~/.simple-social-cli/wiz/` or `~/.config/simple-social-cli/`.
