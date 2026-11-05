# AGENTS.md — FaustLive

Guide for coding agents (and humans) working on this repository.

## What FaustLive is

A Qt desktop application (C++, ~17k lines) to edit, compile and run [Faust](https://faust.grame.fr)
DSP programs on the fly. Each window holds one DSP, JIT-compiled by the embedded **libfaust + LLVM**
and played through an audio driver (CoreAudio on macOS, JACK on Linux, PortAudio on Windows).
Editing the source file triggers a recompilation and a crossfade to the new DSP without interrupting
the sound. Version: `version.txt` (2.5.20).

## Repository layout

| Path | Content |
|---|---|
| `src/MainStructure/` | `FLApp` (the `QApplication`: menus, windows, session), `FLWindow` (one DSP per window, its UIs and audio), `FLSessionManager` (compilation, SHA cache, snapshots), `FLSettings`/`FLWinSettings` (QSettings in the session folder), `FLFileWatcher`, `FLInterfaceManager`, component composer (`FLComponent*`, `FLNodes`) |
| `src/Audio/` | `AudioCreator` (singleton that picks the driver), `AudioFactory`/`AudioManager`/`AudioSettings` interfaces, `AudioFader_*` (crossfade); one subfolder per driver: `CA` CoreAudio, `JA` JACK, `PA` PortAudio, `NJ_Master`/`NJ_Slave` NetJack |
| `src/MenusAndDialogs/` | Preferences, error/help/presentation windows, toolbar, `FLExportManager` (remote compilation through the Faust web service), `SimpleParser` (parses its `/targets` JSON) |
| `src/Network/` | `FLServerHttp` (libmicrohttpd drop server, port 7777), `HTTPWindow` (QR code). `unused/` is not compiled |
| `src/Utilities/` | `main.cpp`, `utilities.cpp` (file and compile-option helpers), `TMutex.h`, `smartpointer.h` |
| `Resources/` | Qt resources: `application.qrc` (**generated**, see below), `Examples/*.dsp`, `Html/`, `Images/`, `Styles/`, and the `Libs/` submodule |
| `Resources/Libs/` | Git submodule → [grame-cncm/faustlibraries](https://github.com/grame-cncm/faustlibraries) |
| `Build/` | `CMakeLists.txt` (the real build), `Makefile` (wrapper), `Make.bat` (Windows), `rsrc/` (plist, rc, desktop file). `FaustLive.pro` is legacy qmake |
| `tests/` | Test suite (CTest), see below |
| `.github/workflows/all.yml` | Windows-only release build (manual or on tag). `.travis.yml` and `docker/` are obsolete |

## Building

Prerequisites: `faust` (with `libfaust.a` built **with the LLVM backend**, plus `libOSCFaust.a` and
`libHTTPDFaust.a`), `llvm-config` of the **same LLVM version as libfaust**, Qt 6 (or Qt 5 with
`-DQT6=off`), CMake, pkg-config, libmicrohttpd, libsndfile (and JACK on Linux).

```bash
cd Build
make                                  # submodule init + qrc + cmake + build → Build/bin/FaustLive.app
make test                             # build and run the tests (ctest)
make help
```

With CMake directly (any build folder; the app is always written to `Build/bin`):

```bash
cmake -S Build -B /tmp/fl-build -DCMAKE_PREFIX_PATH=$HOME/Qt6/6.x/macos
cmake --build /tmp/fl-build -j 8
ctest --test-dir /tmp/fl-build --output-on-failure
```

CMake options: `QT6` (default on), `TESTS` (default on, off on Windows), `REMOTE` (broken: `faust/dsp/remote-dsp.h`
no longer exists), `AUDIO=coreaudio|jack|portaudio|netjack` (the `alsa` branch points to a missing `src/Audio/AL`).

Expected build noise on macOS: linker warnings about objects built for a newer macOS than the
deployment target (`OSXTARGET` is 10.15 in `Build/CMakeLists.txt`) and libc++ "platform no longer supported"
warnings. They are not errors.

## Tests

`tests/` is included by `Build/CMakeLists.txt` when `TESTS=ON`. Run all with `make -C Build test`, or select by label:

```bash
ctest --test-dir <builddir> -L dsp      # libfaust/LLVM checks, no Qt, no audio (~130 tests, a few seconds with -j)
ctest --test-dir <builddir> -L unit     # Qt Test unit tests
ctest --test-dir <builddir> -L app      # starts FaustLive itself (needs an audio device)
ctest --test-dir <builddir> -LE app -j 8   # everything that runs headless (CI)
```

| Test | What it checks |
|---|---|
| `stage_session` (fixture) | `tests/stage_resources.cmake` copies `Libs/` and `Examples/` **as listed in `Resources/application.qrc`** (aliases included) into `<builddir>/tests/session`, like FaustLive does in its session folder |
| `example.<name>` | each bundled example compiles through `tests/faustlive_dsp_check` (same libfaust calls and `-I` order as `FLSessionManager::createFactory` in local mode), every imported `.lib` comes from the bundled Libs (not from a system Faust install), mono (and poly when the DSP is an instrument) instances produce finite samples, and the LLVM bitcode cache round trip gives the same output |
| `dsp.<name>` | same for `tests/dsp/*.dsp` (DX7 through `dx.`, old pre-2.0 libraries, smoke DSP) |
| `library.<name>` | each bundled library can be imported |
| `library.stdfaust_prefixes` | every `xx = library("…")` of `stdfaust.lib` resolves in the bundled Libs |
| `unit.utilities` | `tests/tst_utilities.cpp`: compile-option splitting, `isStringInt`, file helpers, `SimpleParser`. Known bugs are marked `QEXPECT_FAIL`: when you fix one, remove its `QEXPECT_FAIL` |
| `app.smoke` | `tests/app_smoke.cmake` launches the real app headless (`QT_QPA_PLATFORM=offscreen`) with **a temporary `HOME`**, checks it is still running after 10 s, that the session folder holds the flattened libraries, and that the DSP was compiled to `SHAFolder` |

Add a regression DSP by dropping a `.dsp` file into `tests/dsp/` (add `declare nvoices "4";` to also test it
polyphonically), then re-run cmake. Never run the app or tests against the real `~/.FaustLive-CurrentSession-*`
folder: always set `HOME` to a temporary directory.

## Updating the Faust libraries

```bash
git submodule update --remote --merge Resources/Libs
make -C Resources                      # regenerate application.qrc (never edit it by hand)
make -C Build && make -C Build test    # the dsp/library tests catch missing or broken libraries
git add Resources/Libs Resources/application.qrc
```

`Resources/Makefile` puts `Libs/*.lib`, `Libs/old/*.lib` and `Libs/dx7/*.lib` **flat** into `:/Libs`
(with qrc `alias`), like the Faust `install` script does in `faust --dspdir`. It excludes `doc.lib`, which is
only for generating the documentation. If upstream adds a library in a new subfolder, add the folder to
`LIBSUBDIRS` and check that `library.stdfaust_prefixes` passes. Then update `changelog.txt`.

## Runtime layout and caches (important when debugging)

- Session folder: `$HOME/.FaustLive-CurrentSession-<FAUSTLIVE_APP_VERSION>` (Windows:
  `%USERPROFILE%\FaustLive-CurrentSession-…` or `FAUSTLIVE_SESSIONDIR`). `FAUSTLIVE_APP_VERSION` is set in
  `Build/CMakeLists.txt` (currently `2.13`, independent from the app version).
- At startup `FLApp::create_Session_Hierarchy` copies `:/Libs`, `:/Examples`, `:/Html`… into it
  **only for files that are missing**. So a user who upgrades FaustLive keeps the old libraries unless
  `FAUSTLIVE_APP_VERSION` is raised. Raise it for a release that ships new libraries.
- `SHAFolder/<SHA>/`: source (`<SHA>.dsp`), bitcode (`<SHA>_bitcode_process.bc`, `_bitcode_effect.bc`),
  dependencies (`<SHA>.ini`). The SHA comes from `expandDSPFromString` and does not include the LLVM or
  libfaust version or the optimisation level.
- `Windows/FLW-<i>/`: per-window settings. `Settings.ini`: global preferences.

## Compilation pipeline (where to look)

`FLWindow::init_Window` / `update_Window` → `FLSessionManager::createFactory`:
`expandDSPFromString` (to get the SHA) → `createPolyDSPFactoryFromFile` with
`-I <source dir> <user options> -I <session>/Libs -I <session>/Examples` → `writePolyDSPFactoryToBitcodeFile`
→ `FLSessionManager::createDSP` (mono or `createPolyDSPInstance`, `dsp_sample_adapter` for `-double`,
`timed_dsp` for MIDI clock) → `AudioManager` crossfade (`init_FadeAudio` / `start_Fade` / `wait_EndFade`)
→ the old DSP and factory are deleted.

## Conventions

- C++ with Qt. Keep the existing style (4 spaces, `fMember` fields, `_Instance()` singletons, `#ifdef` per
  platform/driver). Both Qt 5 and Qt 6 must compile: use the `QT6` / `QTNEW*` macros of `src/MenusAndDialogs/QTDefs.h`.
- Faust architecture headers come from the installed Faust (`faust -includedir`). A translation unit that
  links Faust GUI code must define `GUI::fGuiList` and `GUI::gTimedZoneMap` once.
- Release: `./version x.y.z` (updates `version.txt`, plist, rc), update `changelog.txt` and
  `FAUSTLIVE_VERSION` in `.github/workflows/all.yml`, raise `FAUSTLIVE_APP_VERSION` if the session caches
  must be reset.
- Do not commit build outputs (`Build/bin`, `Build/builddir`, `*.dmg`, `Build/xcode`).

## Known issues (found during the 2026-11 review, not fixed)

- **Bitcode cache never used**: `FLSessionManager::createFactory` tests `QFileInfo(irFile).exists()`, but
  `writePolyDSPFactoryToBitcodeFile` writes `irFile + "_bitcode_process.bc"`. Every DSP is recompiled.
  If you enable the cache, also add the LLVM/libfaust version to the key (see the README "incorrect cache" crash).
- `utilities.cpp` `executeInstruction` passes a whole command line to `QProcess::start(program, {})`, so
  `touch`/`rm`/`tar` never run, and it always reports success (it is only called from dead helpers).
- `get_numberParameters` / `parse_compilationParams` disagree on options that do not start with `-`
  (`"foo -vec"` loses `-vec`). `isStringInt("")` is true. `readFile` decodes as Latin-1.
- Qt 6: `SIGNAL(error(QNetworkReply::NetworkError))` (FLExportManager.cpp, FLSessionManager.cpp) no longer
  exists (`errorOccurred`), so network errors are silently ignored.
- `FLComponentItem.cpp` calls `createDSP` with `NULL` settings, which `createDSP` dereferences.
- Audio fader flags are `volatile`, not atomic. The GUI thread busy-waits in `wait_EndFade`. The JACK fader
  allocates on the real-time thread.
- `FLApp::~FLApp` uses `delete` on arrays allocated with `new[]`. There are many leaks of `factory`/`argv`/`QSettings`.
- `Build/CMakeLists.txt`: `message(FATAL …)` should be `FATAL_ERROR`, and `CMAKE_CXX_STANDARD` is set to 17 then 11.
  Qt 6 forces C++17 anyway.
- NetJack and REMOTE builds are not maintained against current Faust headers.
