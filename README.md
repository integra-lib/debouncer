# debouncer

A sample-counting debouncer: set after enough agreeing samples, cleared as soon as they stop agreeing.

Part of [integra-lib](https://github.com/integra-lib) — architecture-independent C++20
components shared between firmware projects. Header-only,
no exceptions, no RTTI.

## Use it

```bash
git submodule add git@github.com:integra-lib/debouncer.git external/integra/debouncer
```

```cmake
add_subdirectory(external/integra/debouncer)
target_link_libraries(app PRIVATE Integra::debouncer)
```

```cpp
#include <integra/debouncer.hpp>
```

Each component carries its own include directory, so this header stays unreachable
until the component is linked: a forgotten dependency is a compile error rather than
a build that happens to work.

## What it does

```cpp
// Five readings over the limit before the alarm counts.
integra::Debouncer<> overLimit{5U};

if (overLimit.Update(reading > LIMIT))
{
    RaiseAlarm();
}
```

A saturating counter climbs by `INC` on every true sample and falls by `DEC` on every
false one, and the debouncer is set while the counter sits at the threshold.

It is not symmetric, and that is the point of it: it takes `threshold / INC` agreeing
samples to set, and with `DEC` equal to `INC` a single disagreeing one to clear. It
confirms a condition before acting on it and lets go the moment the condition fails. A
debouncer that should also resist clearing wants hysteresis, which this is not.

The two step sizes weigh the evidence: `Debouncer<1, 3>` sets slowly and clears
three times as fast.

It counts samples, not time. For a button on a pin, where the settle time is what
matters, [button-event](https://github.com/integra-lib/button-event) is the component
to reach for.

## Coming from a160-oto-screening's debouncer

`util::Debouncer` from a160-oto-screening's `lib/util`. The counting is unchanged. What
changed:

* **`Compare(Comparer)` became `Update(bool)`.** The callable was invoked exactly once,
  immediately, so it bought nothing over passing the value: `Compare([&] { return x; })`
  is `Update(x)`.
* **The conversion to `bool` is explicit.** The original's implicit one let
  `int n = debouncer;` compile. `if (debouncer)` still works; so does `IsSet()`.
* **The header includes what it uses.** The original used `std::min` and `std::size_t`
  with only `<type_traits>` included, and compiled because something before it in
  every including file happened to bring them in.
* **Everything is `constexpr`.**

## Versioning

Every component is released on its own, tagged `vX.Y.Z`. Pre-1.0, a minor release may
break the API, which is why dependants accept a single minor.

```bash
git -C external/integra/debouncer fetch --tags
git -C external/integra/debouncer checkout v0.2.0
git add external/integra/debouncer && git commit -m "build: bump debouncer to v0.2.0"
```

## In a consumer's CI

The component is an ordinary submodule, so the build needs it checked out. On GitLab
that means `GIT_SUBMODULE_STRATEGY: normal` (or `recursive`) on every job that builds —
not only on the ones that run unit tests.

## Develop it

```bash
git submodule update --init          # ci-shared, needed by pre-commit
cmake -S . -B build && cmake --build build -j && ctest --test-dir build
```

Tests are built only when this repository is the top-level project, so a consumer
never builds them and never fetches GoogleTest.

The style configs are symlinks into the `ci-shared` submodule, and the pipeline comes
from the same place. On GitHub this repository carries a self-contained build-and-test
workflow instead: a workflow token cannot read another private repository, so neither
a shared workflow nor the submodule is reachable there. The shared setup is what
GitLab will use.
