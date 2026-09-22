# Simple Social CLI — Interactive

A wizard-style interactive front end for the Simple Social API, built on the same
library as `simple-social-cli` and `simple-social-tui`. All three read and write the
same `~/.simple-social-cli/` session state, so logging in with any of them logs you
in everywhere.

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
