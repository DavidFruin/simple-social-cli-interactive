# Simple Social CLI — Interactive

A wizard-style interactive front end for the Simple Social API, built on the same
library as `simple-social-cli` and `simple-social-tui`. All three read and write the
same `~/.simple-social-cli/` session state, so logging in with any of them logs you
in everywhere.

`simple-social-cli` is vendored in as a git submodule under `vendor/`, not a sibling
checkout - this repo is self-contained. It never modifies the vendored copy.

## Build

Install a compiler and the libcurl runtime. On Debian or Ubuntu:

```
sudo apt install build-essential libcurl4 pkg-config
```

(On Ubuntu 24.04 and later the runtime package is `libcurl4t64`, and apt picks
it if you ask for `libcurl4`.) `pkg-config` is optional. libcurl's dev package
is not needed, because its headers are vendored.

Then clone and build somewhere you can write to, such as your home directory:

```
cd ~
git clone --recursive https://github.com/DavidFruin/simple-social-cli-interactive.git
cd simple-social-cli-interactive && make
```

Don't clone from `/` or another root-owned directory. `git clone` fails there
with `could not create work tree dir ... Permission denied`.

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
