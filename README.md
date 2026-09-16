# Simple Social CLI — Interactive

A wizard-style interactive front end for the Simple Social API. Sibling project to
`simple-social-cli`: it links against that repo's already-built `lib/libss.so` and shares
login/session state with it via `~/.simple-social-cli/` — logging in with one tool is visible
to the other.

This repo never modifies `simple-social-cli`.

## Build

```
cd ../simple-social-cli && make   # build the shared library first, if not already built
cd ../simple-social-cli-interactive && make
```

## Run

```
./simple-social-cli-interactive
```

You'll get a prompt. Type a single command word (e.g. `login`, `feed`, `create`, `help`,
`exit`) and the tool will prompt you for each field it needs, one at a time. Password fields
are entered with the terminal echo hidden.
