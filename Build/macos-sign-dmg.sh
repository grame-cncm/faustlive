#!/bin/bash
# Signs a FaustLive .dmg (made by 'make package' or by the CI) with a Developer ID,
# with the hardened runtime and Build/rsrc/FaustLive.entitlements, then notarizes and
# staples it when a notarytool keychain profile is given. Without notarization, macOS
# (Gatekeeper) refuses to open a downloaded FaustLive.app.
#
# usage: macos-sign-dmg.sh <input.dmg> <output.dmg> <identity> [notarytool profile]
#   identity : e.g. "Developer ID Application: GRAME (XN9RP996NW)"
#   profile  : created once with
#              xcrun notarytool store-credentials <profile> --apple-id <id> --team-id <team>
#              (the password is asked and kept in the keychain)

set -euo pipefail

if [ $# -lt 3 ]; then
    echo "usage: $0 <input.dmg> <output.dmg> <identity> [notarytool profile]" >&2
    exit 2
fi
IN=$1
OUT=$2
IDENTITY=$3
PROFILE=${4:-}
ENTITLEMENTS="$(cd "$(dirname "$0")" && pwd)/rsrc/FaustLive.entitlements"

WORK=$(mktemp -d -t faustlive-sign)
trap 'hdiutil detach -quiet "$WORK/in" 2>/dev/null || true; rm -rf "$WORK"' EXIT

# copy the content of the input dmg
hdiutil attach -nobrowse -readonly -mountpoint "$WORK/in" "$IN" > /dev/null
VOLNAME=$(basename "$IN" .dmg)
mkdir "$WORK/$VOLNAME"
ditto "$WORK/in" "$WORK/$VOLNAME"
hdiutil detach -quiet "$WORK/in"
APP="$WORK/$VOLNAME/FaustLive.app"

# sign from the inside out: plugins and dylibs, frameworks, then the application
SIGN=(codesign --force --timestamp --options runtime --sign "$IDENTITY")
find "$APP/Contents" \( -name "*.dylib" -o -name "*.so" \) -type f -print0 | while IFS= read -r -d '' f; do
    "${SIGN[@]}" "$f"
done
for fw in "$APP"/Contents/Frameworks/*.framework; do
    [ -d "$fw" ] && "${SIGN[@]}" "$fw"
done
"${SIGN[@]}" --entitlements "$ENTITLEMENTS" "$APP"
codesign --verify --deep --strict --verbose=2 "$APP"
codesign -d --entitlements - "$APP" 2>/dev/null | grep -q allow-jit

# new dmg, signed
rm -f "$OUT"
for i in 1 2 3 4 5; do
    hdiutil create "$OUT" -volname "$VOLNAME" -fs HFS+ -srcfolder "$WORK/$VOLNAME" -format UDBZ -ov > /dev/null && break
    echo "hdiutil create failed (attempt $i), retrying" >&2
    sleep 5
done
codesign --force --timestamp --sign "$IDENTITY" "$OUT"

if [ -n "$PROFILE" ]; then
    xcrun notarytool submit "$OUT" --keychain-profile "$PROFILE" --wait
    xcrun stapler staple "$OUT"
    spctl -a -t open --context context:primary-signature -vv "$OUT"
else
    echo "not notarized (no notarytool profile given)"
fi
echo "Signed: $OUT"
