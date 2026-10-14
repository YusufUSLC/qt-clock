# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- The About dialog now shows the author's logo.

### Fixed
- The window icon was never loaded because of a wrong resource path (`:/icons/qtclock.png`).

## [1.0.0] - 2026-10-14

A complete rewrite of the original prototype.

### Added
- Resolution-independent vector rendering with a cached static dial layer; HiDPI aware. While the second
  hand sweeps, only the area it crosses is repainted.
- Five built-in themes (Forest, Midnight, Daylight, Sunset, Neon) and <kbd>Ctrl</kbd>+<kbd>T</kbd> to cycle them.
- Smooth sweeping second hand, with a classic ticking alternative.
- Time zone support: local time, a shortlist of popular cities and every IANA zone, with live UTC offsets.
- 12/24-hour display (defaults to the system locale) and a localised date line.
- Frameless, rounded, draggable desktop-widget mode; always-on-top option; full-screen mode.
- Zoom via shortcuts and <kbd>Ctrl</kbd>+mouse wheel.
- Custom logo support (loaded from disk at runtime).
- Persistent settings (`QSettings`), including window geometry.
- Context menu, menu bar, About dialog and `--help` / `--version`.
- CMake build for Qt 5.15 and Qt 6, plus a qmake project file.
- Beginner-friendly explanatory comments in every source file, written in Turkish (`// TR:`) and in
  plain English (`// EN:`).
- QtTest suite, GitHub Actions CI, README (English and Turkish), contributing guide, issue and pull
  request templates, MIT license, Linux desktop entry and application icon.

### Changed
- Source files moved to `src/`; identifiers are now in English.
- The hour hand now moves continuously with the minutes instead of jumping once per hour.
- The refresh timer follows the system clock instead of drifting with a fixed one-second interval.

### Fixed
- Two `QPainter` objects were active on the same widget at once, which is undefined behaviour and
  prints warnings on most platforms.
- The widget had a hard-coded 1300×350 size and could not be resized.
- The hour hand was dark teal on a dark green face and was hard to see.
- The digital clock used the Windows-only "OCR A Extended" font; it now uses the system fonts.
- `M_PI` was used without a portable definition (breaks MSVC).
- Rendering no longer continues while the window is hidden or minimised.

### Removed
- The bundled third-party logo image. Use **Display → Choose logo…** to show your own logo.
- The Qt Designer `.ui` file; the window is now built in code.

[Unreleased]: https://github.com/YusufUSLC/qt-clock/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/YusufUSLC/qt-clock/releases/tag/v1.0.0
