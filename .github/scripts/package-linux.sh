#!/bin/bash
# Makes the Linux archive FaustLive-<version>-linux-<arch>.tar.gz from a configured and
# built FaustLive (cmake build folder), for the CI (.github/workflows/all.yml) or locally.
#
# The archive holds a folder FaustLive-<version>-linux-<arch>/ with:
#   FaustLive                  launcher script
#   usr/bin/FaustLive          the application
#   usr/lib, usr/plugins       Qt and the other libraries, bundled by linuxdeploy (+ its Qt plugin)
#   usr/share                  desktop file, icons, documentation
#   README.txt
# libjack, libasound and libcurl are not bundled: the ones of the system are used
# (JACK or PipeWire's JACK, ALSA, curl).
#
# usage: package-linux.sh <cmake build folder> <output folder>
# environment:
#   QMAKE   qmake of the Qt used to build FaustLive (required by linuxdeploy-plugin-qt)

set -euo pipefail

if [ $# -ne 2 ]; then
    echo "usage: $0 <cmake build folder> <output folder>" >&2
    exit 2
fi
: "${QMAKE:?QMAKE must name the qmake of the Qt used to build FaustLive}"

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
BUILDDIR=$(cd "$1" && pwd)
mkdir -p "$2"
OUT=$(cd "$2" && pwd)
VERSION=$(cat "$ROOT/version.txt")
ARCH=$(uname -m)
NAME=FaustLive-$VERSION-linux-$ARCH
WORK="$OUT/.package"
DIST="$WORK/$NAME"

# linuxdeploy and its Qt plugin, pinned and checked
LINUXDEPLOY_TAG=1-alpha-20251107-1
LINUXDEPLOY_QT_TAG=1-alpha-20250213-1
case "$ARCH" in
    x86_64)
        LINUXDEPLOY_SHA256=c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d
        LINUXDEPLOY_QT_SHA256=15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724
        ;;
    aarch64)
        LINUXDEPLOY_SHA256=620095110d693282b8ebeb244a95b5e911cf8f65f76c88b4b47d16ae6346fcff
        LINUXDEPLOY_QT_SHA256=bf1c24aff6d749b5cf423afad6f15abd4440f81dec1aab95706b25f6667cdcf1
        ;;
    *)
        echo "unsupported architecture $ARCH" >&2
        exit 1
        ;;
esac

# download <url> <sha256> <file>
download() {
    [ -f "$3" ] || curl -fsSL --retry 3 -o "$3" "$1"
    if [ "$(sha256sum < "$3" | cut -d' ' -f1)" != "$2" ]; then
        rm -f "$3"
        echo "$(basename "$1"): unexpected sha256" >&2
        exit 1
    fi
    chmod +x "$3"
}

rm -rf "$WORK"
mkdir -p "$WORK/tools" "$DIST"
download "https://github.com/linuxdeploy/linuxdeploy/releases/download/$LINUXDEPLOY_TAG/linuxdeploy-$ARCH.AppImage" \
    $LINUXDEPLOY_SHA256 "$WORK/tools/linuxdeploy"
download "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/$LINUXDEPLOY_QT_TAG/linuxdeploy-plugin-qt-$ARCH.AppImage" \
    $LINUXDEPLOY_QT_SHA256 "$WORK/tools/linuxdeploy-plugin-qt"
export PATH="$WORK/tools:$PATH"
# no FUSE in containers
export APPIMAGE_EXTRACT_AND_RUN=1

#######################################
echo "::group::Install FaustLive in $DIST/usr"
cmake "$BUILDDIR" -DPREFIX="$DIST/usr"
cmake --build "$BUILDDIR" --target install
echo "::endgroup::"

#######################################
echo "::group::Bundle the libraries (linuxdeploy)"
"$WORK/tools/linuxdeploy" --appdir "$DIST" \
    --executable "$DIST/usr/bin/FaustLive" \
    --desktop-file "$DIST/usr/share/applications/FaustLive.desktop" \
    --icon-file "$DIST/usr/share/icons/hicolor/scalable/apps/Faustlive.svg" \
    --exclude-library 'libjack.so*' \
    --exclude-library 'libasound.so*' \
    --exclude-library 'libcurl.so*' \
    --plugin qt
# AppImage leftovers, not used by the archive
rm -f "$DIST/AppRun" "$DIST/.DirIcon" "$DIST"/*.desktop "$DIST"/*.svg
echo "::endgroup::"

#######################################
cat > "$DIST/FaustLive" <<'EOF'
#!/bin/sh
# FaustLive launcher: runs the application with the libraries of this folder
HERE=$(dirname "$(readlink -f "$0")")
exec "$HERE/usr/bin/FaustLive" "$@"
EOF
chmod +x "$DIST/FaustLive"

cat > "$DIST/README.txt" <<EOF
FaustLive $VERSION for Linux ($ARCH)
https://github.com/grame-cncm/faustlive

Run ./FaustLive from this folder (it can be moved anywhere).

Requirements, from the distribution:
  - JACK (jackd, or PipeWire with its JACK support: pipewire-jack), for the audio
  - libcurl and libasound (installed on most desktops)
  - an X11 session, or Wayland with XWayland
Built on glibc 2.28 (manylinux_2_28): runs on Ubuntu 20.04, Debian 10, Fedora 29,
RHEL 8 or later.

Soundfiles: WAV, AIFF, CAF and the other formats of libsndfile without external
libraries (no FLAC, Ogg or MP3).

Desktop integration: copy usr/share/applications/FaustLive.desktop to
~/.local/share/applications/ and set its Exec line to the full path of ./FaustLive.

See usr/share/doc/faustlive for the documentation.
EOF
cp "$ROOT/changelog.txt" "$DIST/usr/share/doc/faustlive/" 2>/dev/null || true

#######################################
# Every library must be bundled or come from the system list above
if LD_LIBRARY_PATH= ldd "$DIST/usr/bin/FaustLive" | grep "not found"; then
    echo "unresolved libraries" >&2
    exit 1
fi
ldd "$DIST/usr/bin/FaustLive" | grep -v "$DIST" | sed 's/^/  system: /'

tar -C "$WORK" -czf "$OUT/$NAME.tar.gz" "$NAME"
rm -rf "$WORK"
echo "Created $OUT/$NAME.tar.gz"
