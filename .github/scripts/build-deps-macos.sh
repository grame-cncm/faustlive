#!/bin/bash
# Builds the FaustLive dependencies for macOS into a prefix, for the CI
# (.github/workflows/all.yml) or locally:
#   - libmicrohttpd (static, no HTTPS)
#   - libsndfile (static, no external libraries: WAV, AIFF, CAF... but no FLAC, Ogg or MP3)
#   - Faust: the compiler, libfaust.a with the LLVM backend, libOSCFaust.a, libHTTPDFaust.a
# for the architecture of the machine.
#
# usage: build-deps-macos.sh <prefix> <faust source dir>
# environment:
#   LLVM_CONFIG                llvm-config of the static LLVM (see .github/actions/fetch-llvm), required
#   LLVM_REQUIRED_VERSION      optional, checked by the Faust build
#   MACOSX_DEPLOYMENT_TARGET   default 12.0 (Qt 6.8 needs macOS 12)
#   JOBS                       default: number of CPUs
#
# Afterwards: PATH=<prefix>/faust/bin:$PATH PKG_CONFIG_PATH=<prefix>/lib/pkgconfig

set -euo pipefail

if [ $# -ne 2 ]; then
    echo "usage: $0 <prefix> <faust source dir>" >&2
    exit 2
fi
: "${LLVM_CONFIG:?LLVM_CONFIG must name the llvm-config of the static LLVM}"

mkdir -p "$1"
PREFIX=$(cd "$1" && pwd)
FAUST_SRC=$(cd "$2" && pwd)
export MACOSX_DEPLOYMENT_TARGET=${MACOSX_DEPLOYMENT_TARGET:-12.0}
JOBS=${JOBS:-$(sysctl -n hw.ncpu)}
ARCH=$(uname -m)
WORK="$PREFIX/.work"

MHD_VERSION=1.0.2
MHD_SHA256=df324fcd0834175dab07483133902d9774a605bfa298025f69883288fd20a8c7
SNDFILE_VERSION=1.2.2
SNDFILE_SHA256=3799ca9924d3125038880367bf1468e53a1b7e3686a934f098b7e1d286cdb80e

command -v pkg-config > /dev/null || { echo "pkg-config is required (brew install pkgconf)" >&2; exit 1; }
echo "Building FaustLive dependencies for $ARCH, macOS >= $MACOSX_DEPLOYMENT_TARGET, in $PREFIX"
echo "LLVM: $LLVM_CONFIG ($("$LLVM_CONFIG" --version))"

# download <url> <sha256> : prints the path of the checked archive
download() {
    local file="$WORK/$(basename "$1")"
    [ -f "$file" ] || curl -fsSL --retry 3 -o "$file" "$1"
    if [ "$(shasum -a 256 < "$file" | cut -d' ' -f1)" != "$2" ]; then
        rm -f "$file"
        echo "$(basename "$1"): unexpected sha256" >&2
        exit 1
    fi
    echo "$file"
}

rm -rf "$WORK"
mkdir -p "$WORK"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"

#######################################
echo "::group::libmicrohttpd $MHD_VERSION"
archive=$(download "https://ftpmirror.gnu.org/libmicrohttpd/libmicrohttpd-$MHD_VERSION.tar.gz" $MHD_SHA256)
tar -xzf "$archive" -C "$WORK"
(
    cd "$WORK/libmicrohttpd-$MHD_VERSION"
    ./configure --prefix="$PREFIX" --disable-shared --enable-static --enable-https=no \
        --disable-doc --disable-examples --disable-tools --disable-curl
    make -j "$JOBS"
    make install
)
echo "::endgroup::"

#######################################
echo "::group::libsndfile $SNDFILE_VERSION"
archive=$(download "https://github.com/libsndfile/libsndfile/releases/download/$SNDFILE_VERSION/libsndfile-$SNDFILE_VERSION.tar.xz" $SNDFILE_SHA256)
tar -xJf "$archive" -C "$WORK"
cmake -S "$WORK/libsndfile-$SNDFILE_VERSION" -B "$WORK/sndfile-build" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=OFF \
    -DENABLE_EXTERNAL_LIBS=OFF -DENABLE_MPEG=OFF -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF \
    -DBUILD_TESTING=OFF -DENABLE_CPACK=OFF -DINSTALL_MANPAGES=OFF
cmake --build "$WORK/sndfile-build" --parallel "$JOBS"
cmake --install "$WORK/sndfile-build"
echo "::endgroup::"

#######################################
echo "::group::Faust $(git -C "$FAUST_SRC" rev-parse --short=9 HEAD 2>/dev/null || echo '(no git)')"
cmake -S "$FAUST_SRC/build" -B "$WORK/faust-build" \
    -C "$FAUST_SRC/build/backends/most.cmake" \
    -DINCLUDE_EXECUTABLE=ON -DINCLUDE_STATIC=ON -DINCLUDE_DYNAMIC=OFF \
    -DINCLUDE_OSC=ON -DINCLUDE_HTTP=ON -DOSCDYNAMIC=OFF -DHTTPDYNAMIC=OFF \
    -DINCLUDE_ITP=OFF -DINCLUDE_EMCC=OFF -DINCLUDE_WASM_GLUE=OFF \
    -DINCLUDE_LLVM_STATIC_IN_ARCHIVE=OFF \
    -DUSE_LLVM_CONFIG=ON -DLLVM_CONFIG="$LLVM_CONFIG" \
    -DLLVM_REQUIRED_VERSION="${LLVM_REQUIRED_VERSION:-}" -DLLVM_FORBID_ASSERTIONS=ON \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX/faust"
cmake --build "$WORK/faust-build" --config Release --parallel "$JOBS"
cmake --install "$WORK/faust-build" --config Release
echo "::endgroup::"

#######################################
# Checks
"$PREFIX/faust/bin/faust" --version
for lib in libfaust.a libOSCFaust.a libHTTPDFaust.a; do
    [ -f "$("$PREFIX/faust/bin/faust" -libdir)/$lib" ] || { echo "missing $lib" >&2; exit 1; }
done
pkg-config --modversion libmicrohttpd sndfile
rm -rf "$WORK"
echo "Dependencies installed in $PREFIX"
