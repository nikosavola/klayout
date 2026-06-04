# KLayout C++ Modernization

This directory tracks the staged C++11 → C++17/20/23/26 modernization effort.

## Guiding constraints

- **The default build must not break.** The Qt6 build is C++17, but the Qt5
  build still compiles at **C++11**, so any C++17+ feature in shared code is
  guarded and opt-in. Neither `build.sh` nor `src/klayout.pri` forces a `-std=`
  flag or emits any `HAVE_CPP*` define: the C++ standard is whatever the compiler
  / Qt mkspec is configured with, and a newer standard is selected purely through
  the compiler flags (e.g. `CXXFLAGS=-std=c++20`).
- **Central feature detection:** `src/tl/tl/tlCxxFeatures.h` infers everything
  from the compiler itself - `__cplusplus` for the standard level and the
  standard `__cpp_*` / `__cpp_lib_*` feature-test macros for individual features
  - with no separately injected `HAVE_CPP*` define. It exposes
  `TL_CXX20/23/26`, `TL_HAS_CONCEPTS/SPAN/SPACESHIP/STD_FORMAT/EXPECTED/`
  `MDSPAN/GENERATOR/STRING_VIEW`, and the helpers `TL_NODISCARD`,
  `TL_IF_CONSTEXPR`.
- **One commit per phase / sub-phase**, each verified by a targeted standalone
  compile (and `static_assert` / runtime checks where applicable), since a full
  qmake build is not available in the CI sandbox.
- Benchmarks use `uv`-runnable Python scripts that prefer `hyperfine`
  (`benchmarks/`).

## Status by phase

Each feature is first given an enabling helper (macro/alias) and then *applied*
at real call sites; the "Applied at" column records the concrete usage (verified
by per-TU compiles at C++11/17/20/23).

| Phase | Item | Helper | Applied at | Build impact |
| --- | --- | --- | --- | --- |
| 1.3 | tl::Variant vs std::variant | — | Assessment doc (keep tl::Variant) | doc only |
| 1.4 | std::string_view | `tl::string_view` + benchmark | `tl::edit_distance` args | C++17+ opt-in |
| 1.5 | structured bindings / if constexpr | `TL_IF_CONSTEXPR` | `tl::to_string` SFINAE→`if constexpr`; `long_uint::operator T` | C++11-safe |
| 2.1 | NULL → nullptr | — | tl library (105 sites) | C++11-safe |
| 2.2 | override | `.clang-tidy` | ShapeCollection hierarchy | C++11-safe |
| 2.3 | C-style → modern casts | — | tlGit.cc / tlHttpStreamCurl.cc | C++11-safe |
| 2.4 | [[nodiscard]] | `TL_NODISCARD` | pure factory/lookup fns in tlExpression.h | C++11-safe |
| 2.5 | smart pointers / RAII | — | DataMappingLookupTable arrays → std::vector | C++11-safe |
| 3.1 | range-based for | — | 9 loops in dbLayout.cc | C++11-safe |
| 3.2 | constexpr | — | tlMath.h integer helpers + epsilon | C++11-safe |
| 3.3 | move semantics | — | noexcept on slist / reuse_vector moves | C++11-safe |
| 3.4 | scoped_lock / shared_mutex | `Mutex::try_lock`, `tl::ScopedLock`, `tl::SharedMutex` | capability only — no multi-mutex site exists to convert (single-mutex MutexLocker is correct) | C++17+ opt-in |
| 4.1 | concepts | `tlConcepts.h` | `tl::gcd` / `tl::lcm` constrained to `tl::Coordinate` | C++20 opt-in |
| 4.2 | std::span | `tl::span` | `tl::to_string(span<…>)` byte-buffer overloads | C++20 opt-in |
| 4.3 | operator<=> | — | order-preserving `<=>` on db::point and db::vector | C++20 opt-in |
| 4.4 | coroutine iterators | — | Assessment (exploratory) | doc only |
| 4.5 | std::format | `tl::format` | octal escapes in to_quoted_string/escape_string | C++20 opt-in |
| 5.1 | std::expected | `tl::expected` | `tl::try_from_string<T>` parse wrapper | C++23 opt-in |
| 5.2 | std::mdspan | `tl::mdspan` | capability only — `<mdspan>` needs GCC14+/Clang17+, not in this toolchain so usage is unverifiable here | C++23 (GCC14+) |
| 5.3 | std::generator | `tl::generator` | capability only — `<generator>` needs GCC14+/Clang18+, unverifiable here | C++23 (GCC14+) |
| 5.4 | pattern matching | — | Monitor P2688 (no compiler support) | doc only |
| 5.5 | contracts | `tl_precondition` / `tl_postcondition` | 15 argument-check asserts in db::Layout | C++11-safe |

## Running the test suite

The plan calls for `unit_tests/` to be run after each phase. That requires a
configured qmake build (`./build.sh`), which is outside this sandbox; each
change here was instead verified with standalone `g++ -fsyntax-only` /
`static_assert` checks against the affected translation units at C++11/17/20/23.
Run the full `unit_tests` suite in a configured build before merging.
