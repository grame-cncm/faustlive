# AGENTS.md — FaustLive

Guide for coding agents (and humans) working on this repository.

## What FaustLive is

A Qt desktop application (C++, ~17k lines) to edit, compile and run [Faust](https://faust.grame.fr)
DSP programs on the fly. Each window holds one DSP, JIT-compiled by the embedded **libfaust + LLVM**
and played through an audio driver (CoreAudio on macOS, JACK on Linux, PortAudio on Windows).
Editing the source file triggers a recompilation and a crossfade to the new DSP without interrupting
the sound. Version: `version.txt` (2.5.21).

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
| `.github/` | CI: `workflows/all.yml` (macOS arm64/x86_64, Linux x86_64/aarch64 and Windows binaries, see "Continuous integration"), `actions/fetch-llvm` and `llvm/` (copied from Faust), `scripts/build-deps.sh`, `scripts/package-linux.sh`. `.travis.yml` and `docker/` are obsolete |

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
deployment target (`OSXTARGET`: `-DOSXTARGET=x.y`, else `$MACOSX_DEPLOYMENT_TARGET`, else 12.0 with Qt 6) and libc++ "platform no longer supported"
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
| `example.<name>` | each bundled example compiles through `tests/faustlive_dsp_check` (same libfaust calls and `-I` order as `FLSessionManager::createFactory` in local mode), every imported `.lib` comes from the bundled Libs (not from a system Faust install), mono (and poly when the DSP is an instrument) instances produce finite samples, and the LLVM bitcode cache round trip gives the same output and keeps the compile options, library list and include pathnames |
| `dsp.<name>` | same for `tests/dsp/*.dsp` (DX7 through `dx.`, old pre-2.0 libraries, smoke DSP) |
| `library.<name>` | each bundled library can be imported |
| `library.stdfaust_prefixes` | every `xx = library("…")` of `stdfaust.lib` resolves in the bundled Libs |
| `unit.utilities` | `tests/tst_utilities.cpp`: compile-option splitting, `isStringInt`, file helpers, external commands (`executeInstruction`, tar round trip, `deleteDirectoryAndContent`, `touchFolder`), `FJUI` connection renaming, audio crossfade gains (`AudioFader_Implementation`), `SimpleParser`. Known bugs are marked `QEXPECT_FAIL`: when you fix one, remove its `QEXPECT_FAIL` |
| `app.smoke` | `tests/app_smoke.cmake` launches the real app headless (`QT_QPA_PLATFORM=offscreen`) with **a temporary `HOME`**, checks it is still running after 10 s, that the session folder holds the flattened libraries, and that the DSP was compiled to `SHAFolder`; then starts it again on the same session and checks that the bitcode cache is read (file not rewritten) |

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

## Continuous integration

`.github/workflows/all.yml` runs on tags and on `workflow_dispatch` (inputs: `create_release` for a draft
release, `faust-ref` to build another Faust commit). To try a branch:
`gh workflow run all.yml --ref <branch> -f create_release=false`.

| Job | Runner | Result |
|---|---|---|
| `build-macos` (`macos-arm64`) | `macos-15` | `FaustLive-<version>-macos-arm64.dmg` |
| `build-macos` (`macos-x86_64`) | `macos-15-intel` | `FaustLive-<version>-macos-x86_64.dmg` |
| `build-linux` (`linux-x86_64`) | `ubuntu-24.04`, manylinux_2_28 container | `FaustLive-<version>-linux-x86_64.tar.gz` |
| `build-linux` (`linux-aarch64`) | `ubuntu-24.04-arm`, manylinux_2_28 container | `FaustLive-<version>-linux-aarch64.tar.gz` |
| `build-windows` (`win64`) | `windows-2022` | `FaustLive-<version>-win64.exe` (NSIS, `Build/Make.bat package`) |

- **LLVM**: Grame's static LLVM (release `llvm-<version>-<revision>` of grame-cncm/faust), downloaded and
  checked against `.github/llvm/SHA256SUMS` by `.github/actions/fetch-llvm`. These three files are copied from
  the Faust repository (`/.github`): to upgrade LLVM, copy them again and update `FAUST_REF` accordingly.
- **libfaust**: built from the Faust sources at `FAUST_REF` (env of the workflow) with `backends/most.cmake`, static
  `libfaust` + `libOSCFaust` + `libHTTPDFaust`, installed in `$RUNNER_TEMP/deps/faust` and cached
  (key: platform, `DEPS_CACHE_VERSION`, Faust commit, LLVM checksums, deps script). `faust -libdir` is relative to
  the executable, so FaustLive's CMake finds it through `PATH`.
- **macOS and Linux** dependencies come from `.github/scripts/build-deps.sh <prefix> <faust source>`, which also runs
  locally (needs `LLVM_CONFIG`): static libmicrohttpd and libsndfile **without external libraries** (soundfiles in
  WAV/AIFF/CAF..., no FLAC/Ogg/MP3), then Faust.
- **macOS**: deployment target 12.0 (Qt 6.8). Then `make -C Build cmake faustlive`,
  `ctest -LE app`, `make -C Build package` (macdeployqt + ad-hoc `codesign`, `CODESIGN_ID` to sign for real;
  the dmg is not notarized).
- **Linux**: like Faust's libfaust, built in the `quay.io/pypa/manylinux_2_28_<arch>` image of `.github/llvm/release.env`
  (AlmaLinux 8, glibc 2.28: runs on Ubuntu 20.04, Debian 10, RHEL 8 and later). System packages from `dnf`
  (JACK, ALSA, curl, X11/xcb). Qt: 6.8.3 from `aqtinstall` on x86_64; the Qt 5.15 of AlmaLinux 8 (`-DQT6=off`)
  on aarch64, because Qt's aarch64 Linux binaries need glibc 2.38. JACK audio driver, `ctest -LE app`, then
  `.github/scripts/package-linux.sh <builddir> <outdir>`: install in an AppDir, bundle Qt and the other libraries with
  linuxdeploy + linuxdeploy-plugin-qt (pinned, sha256 checked), add a `FaustLive` launcher and a README, make the
  `.tar.gz`. libjack, libasound and libcurl are left to the system.
- **Windows**: Qt via `jurplel/install-qt-action`, libsndfile 1.2.0 zip, libmicrohttpd and PortAudio from `lib/`;
  tests are not run (`TESTS` is off on Windows).
- **LLVM 23 needs Faust >= 2.92.3** (commit `e2b2737f7`): before, libfaust built negative integer constants with
  `ConstantInt::get(i32, int)`, which no longer truncates in LLVM 23, so the x86 code generator crashed on `ef.echo1s`
  (`echo.dsp`, `quadEcho.dsp`, `stereoEcho.dsp`) and arm64 produced wrong code. `dsp.llvm_x86_feedback_delay` catches it.
- Bump `QT_VERSION` / `FAUST_REF` in the workflow env (the deps cache key contains the Faust commit, so a new
  `FAUST_REF` rebuilds libfaust); raise `DEPS_CACHE_VERSION` to force a rebuild of the deps otherwise.

## Runtime layout and caches (important when debugging)

- Session folder: `$HOME/.FaustLive-CurrentSession-<FAUSTLIVE_APP_VERSION>` (Windows:
  `%USERPROFILE%\FaustLive-CurrentSession-…` or `FAUSTLIVE_SESSIONDIR`). `FAUSTLIVE_APP_VERSION` is set in
  `Build/CMakeLists.txt` (currently `2.14`, independent from the app version).
- At startup `FLApp::create_Session_Hierarchy` copies `:/Libs`, `:/Examples`, `:/Html`… into it
  **only for files that are missing**. So a user who upgrades FaustLive keeps the old libraries unless
  `FAUSTLIVE_APP_VERSION` is raised. Raise it for a release that ships new libraries.
- `SHAFolder/<SHA>/`: source (`<SHA>.dsp`), bitcode cache (`<SHA>-<KEY>_bitcode_process.bc`, `_bitcode_effect.bc`),
  dependencies (`<SHA>.ini`). The SHA comes from `expandDSPFromString`; `KEY` (`bitcodeCachePath` in
  FLSessionManager.cpp) is a SHA1 of the SHA, the Faust options, the optimisation level, `getCLibFaustVersion()`,
  `LLVM_VERSION` and `getDSPMachineTarget()`, so a bitcode file from another libfaust/LLVM/CPU is never read.
  `createFactory` reads the cache when the file exists (`app.smoke` checks it on a second launch), else compiles.
- `cleanSHAFolder` keeps at most 100 SHA folders and removes the least recently used one (`touchFolder` dates).
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
  `FAUSTLIVE_VERSION` in `.github/workflows/all.yml` (the workflow checks it against `version.txt`), raise `FAUSTLIVE_APP_VERSION` if the session caches
  must be reset.
- Do not commit build outputs (`Build/bin`, `Build/builddir`, `*.dmg`, `Build/xcode`).

## Known issues (2026-11 review, not fixed)

Fixed since (see `git log`): bitcode cache, `executeInstruction` (touch/rm/tar/sound2faust/script),
option parsing, `isStringInt("")`, `readFile`, Qt 6 network signals, NULL settings in `createDSP`, fader flags and
busy waits, JACK RT allocations, `FJUI::update`, FLServerHttp, several leaks. Still open:

- **JACK DSP swap** (`JA_audioFader::upDate_DSP`, GUI thread): when the fade ends the audio thread goes back to the
  normal path with the *old* `fDSP` at full volume until the GUI swaps `fDSP`/`fDSPIn`, and the extra ports of the old
  DSP are unregistered while the audio thread may still use them. The swap should happen in the audio thread at the end
  of the fade. Not changed: no JACK test setup.
- `start_Fade` ignores a failed start of the new CoreAudio/PortAudio stream: the old one fades out, the window
  plays nothing and the update is reported as successful (needs a `bool` in the `AudioManager` interface).
- `FLWinSettings::setValue` hides (does not override) `QSettings::setValue`: calls through a `QSettings*` do not
  update the general `Windows/<i>/Path|Name|SHA` keys.
- Other leaks: `QNetworkAccessManager` per export request, `QFileDialog`/`QMessageBox` created with `new` and never
  deleted, `FLWindow` objects whose `init_Window` failed (`delete win` does not delete its audio manager and tool bar).
- `Build/CMakeLists.txt`: `message(FATAL …)` should be `FATAL_ERROR`. (FaustLive is compiled as C++17, also with Qt 5.)
- NetJack and REMOTE builds are not maintained against current Faust headers (the NetJack and PortAudio files are not
  compiled on macOS).
