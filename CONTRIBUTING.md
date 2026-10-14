# Contributing to Qt Clock

Thanks for taking the time to improve Qt Clock! Bug reports, new themes, documentation fixes and
code contributions are all welcome.

## Quick start

```bash
git clone https://github.com/YusufUSLC/qt-clock.git
cd qt-clock
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
(cd build && ctest --output-on-failure)   # runs headless
./build/QtClock
```

You need a C++17 compiler, CMake 3.16+ and Qt 5.15 **or** Qt 6 (Widgets module). Both Qt versions are
supported and tested in CI, so please do not use APIs that exist in only one of them. When both are
installed, `-DQT_VERSION_MAJOR_OVERRIDE=5` or `=6` picks one.

Handy options:

| CMake option | Purpose |
| --- | --- |
| `-DQTCLOCK_WARNINGS_AS_ERRORS=ON` | Treat warnings as errors (CI does this on Linux) |
| `-DQTCLOCK_BUILD_TOOLS=ON` | Also build the README screenshot generator |
| `-DQTCLOCK_BUILD_TESTS=OFF` | Skip the tests |

## Adding a theme

Themes are plain data, so this is the easiest way to contribute.

1. Open [`src/clocktheme.cpp`](src/clocktheme.cpp) and copy one of the blocks in `buildThemes()`.
2. Give it a unique lower-case `id` (stored in the settings file, so never change it later) and a
   display `name` wrapped in `QObject::tr()`.
3. Fill in the colours. Keep hands and numerals clearly readable against the face — the test suite
   checks that their lightness differs from the face by a safe margin.
4. Run the tests, then regenerate the images (see below) and look at your theme.

No other file needs to change: the menu, the <kbd>Ctrl</kbd>+<kbd>T</kbd> cycling and the settings all pick
up new entries automatically.

## Regenerating the README images

The screenshots, the banner, the demo GIF and the application icon are rendered by the real widgets,
so they always match the code. After changing how the clock looks, run:

```bash
cmake -S . -B build -DQTCLOCK_BUILD_TOOLS=ON
cmake --build build
./tools/regenerate_assets.sh build/tools/screenshots
```

The GIF step needs Python 3 with [Pillow](https://pypi.org/project/pillow/).

## Code style

- C++17, Qt naming conventions (`camelCase` members/functions, `m_` prefix for data members).
- Format with `clang-format` using the checked-in [`.clang-format`](.clang-format) when you can.
- Keep user-visible strings in `tr()`.
- Comments are bilingual to help beginners: a `// TR:` line (Turkish) followed by a `// EN:` line
  (plain, B1-level English). Explain *what* a block does and *why*; skip the obvious. When you change
  code, update the matching comment. `clang-format` is set up not to re-wrap comments.
- Keep drawing code in the logical coordinate system (dial radius = 100 units). Only text sizes are
  converted to device pixels, in `drawLogicalText()`.
- Put anything that can be computed without a widget into [`clockmath.h`](src/clockmath.h) and test it.

## Tests

Please add or update a test in [`tests/tst_clock.cpp`](tests/tst_clock.cpp) for every behaviour change.
The suite uses QtTest and runs on the `offscreen` platform, so it works on CI machines without a
display. To run a single test: `./build/tests/tst_clock handAngles`.

## Pull requests

1. Fork the repository and create a topic branch from `main`.
2. Keep the change focused; unrelated clean-ups belong in a separate PR.
3. Make sure it builds and passes the tests with your Qt version, ideally with warnings as errors.
4. Describe *what* changed and *why*. Include a screenshot for anything visual.
5. Update [`CHANGELOG.md`](CHANGELOG.md) under "Unreleased".

## Reporting bugs

Please use the issue form and include your operating system, Qt version and — for display problems — a
screenshot. For crashes, a backtrace is very helpful.

## Licensing

By contributing you agree that your contribution is licensed under the [MIT License](LICENSE).
Please do not add third-party logos, trademarks or other artwork that you do not have the right to
redistribute.
