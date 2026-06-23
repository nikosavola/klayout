# Code style, linting and static analysis

This document describes the formatting, linting and static-analysis tooling
configured for KLayout and how to adopt it. The configuration is in place, but
**the existing source tree has not been mass-reformatted** — adoption is
deliberately incremental so it does not collide with in-flight work or produce
an unreviewable diff.

## Tooling at a glance

| Language | Formatter        | Linter            | Type / static analysis        | Config |
|----------|------------------|-------------------|-------------------------------|--------|
| C++      | `clang-format`   | `cppcheck`        | `clang-tidy`, CodeQL, sanitizers | `.clang-format`, `.clang-tidy` |
| Python   | `ruff format`    | `ruff check`      | `pyrefly`, CodeQL             | `pyproject.toml` |
| Ruby     | (rubocop -a)     | `rubocop`         | —                             | `.rubocop.yml` |
| All      | whitespace hooks | —                 | —                             | `.editorconfig`, `.pre-commit-config.yaml` |

## Quick start

```sh
pipx install pre-commit ruff          # or: pip install ...
pre-commit install                    # run hooks on staged files at commit time
pre-commit run --all-files            # one-off run over everything
```

C++ formatting (line-scoped, recommended while the tree is unformatted):

```sh
git clang-format            # format only the lines you changed, before commit
```

## C++ — `.clang-format`

The config codifies the existing hand-maintained house style rather than a
stock style. It was tuned **empirically** against `src/db`, `src/tl` and
`src/laybasic` to minimise churn: on representative core files it changes
~15–20% of lines, and most of that is normalising pre-existing
inconsistencies (missing spaces before parentheses, stray trailing
whitespace), not house-style changes.

These distinctive KLayout idioms are preserved by dedicated options
(`SpaceBeforeParens`, `SpaceAfterLogicalNot`, `SpaceBeforeSquareBrackets`,
`SpaceAfterCStyleCast`, `BreakBeforeBraces: Linux`):

```cpp
namespace tl
{

void foo (int *p)        // space before '(', pointer bound to name, brace own line
{
  if (! p) {             // space after '!', brace attached for control flow
    return arr [0];      // space before subscript bracket
  }
  bar ((int) x);         // space after C-style cast
}

}
```

Pin the **same clang-format major version** locally and in CI (currently 19);
output drifts across versions. `src/gsiqt` (machine-emitted Qt bindings) and
`testdata` are excluded via `.clang-format-ignore`.

## Python — ruff and pyrefly

* Minimum supported Python is **3.10** (the cibuildwheel `skip` drops 3.6–3.9).
* `ruff check` is the linter, `ruff format` the formatter (Black-compatible).
* Line length is relaxed (`E501` ignored) and the intentional star-import
  re-exports in the shipped `klayout` package `__init__` modules are
  per-file-ignored.
* `pyrefly` is the type checker. It is **informational**: the shipped package
  is thin Python over the compiled `pya`/`*core` C-extension, so the
  typed/untyped boundary is inherently noisy until the `*.pyi` stubs cover it.

## Ruby — rubocop

`.rubocop.yml` starts lax: correctness (`Lint/*`) and cheap whitespace
(`Layout/*`) cops are on; opinionated `Style/*` and `Metrics/*` cops are off so
the established DRC/LVS macro style is not flagged en masse. Tighten over time.

## Sanitizers

The Mac-only `MAC_USE_ASAN` switch has been generalised to a `SANITIZE` qmake
variable that works on Linux and macOS:

```sh
KLAYOUT_QMAKE_ARGS="SANITIZE=address SANITIZE+=undefined" ./build.sh -debug
KLAYOUT_QMAKE_ARGS="SANITIZE=thread"                      ./build.sh -debug
```

Accepted tokens: `address`, `undefined`, `thread`, `leak` (`thread` is
mutually exclusive with `address`/`leak`). The `MAC_USE_ASAN=1` environment
switch still works for backwards compatibility. The `Sanitizers` workflow
builds with ASan+UBSan and runs the test suite on demand / weekly.

## CI workflows

* `lint.yml` — **required**: C++ formatting on changed lines
  (`clang-format-diff`) and Python lint on changed files (`ruff check`).
  **advisory** (non-blocking): `ruff format --check`, `cppcheck`, `pyrefly`,
  `rubocop`. Scoping to changes means new code is enforced while legacy code
  is untouched.
* `codeql.yml` — CodeQL security/quality analysis for C++ and Python.
* `sanitizers.yml` — manual/scheduled ASan+UBSan build and test.

## Rollout plan

1. **Now (this change):** land all configs, pre-commit, CI (checks scoped to
   changed code / advisory), and the sanitizer build switch. No existing files
   are reformatted.
2. **Python format:** a dedicated commit running `ruff check --fix` and
   `ruff format` over the tracked Python (small, ~28 files); then make
   `ruff format --check` required.
3. **C++ incremental:** contributors run `git clang-format` on changed lines;
   the required `clang-format` check keeps new lines conformant.
4. **Static analysis:** triage `cppcheck`/CodeQL findings; stand up
   `clang-tidy` against a Bear-generated `compile_commands.json` on touched
   files.
5. **C++ flag day (optional, maintainer decision):** a single repo-wide
   `clang-format` commit, recorded in `.git-blame-ignore-revs` so `git blame`
   skips it. This is a large diff and is intentionally deferred.
6. **Ruby:** enable `rubocop` as required once the lax baseline is clean.
