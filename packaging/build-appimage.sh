#!/usr/bin/env bash
# Build a Linux AppImage from an existing CMake build directory.
# Usage: ./packaging/build-appimage.sh [build-dir]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${1:-$ROOT/build}"
OUT="$ROOT/packaging/_out"
APPDIR="$OUT/AppDir"

rm -rf "$APPDIR"
mkdir -p "$APPDIR" "$OUT"

cmake --install "$BUILD" --prefix "$APPDIR/usr"

# Fetch linuxdeploy + the Qt plugin if not present.
cd "$OUT"
get() { [ -f "$1" ] || { curl -fL "$2" -o "$1"; chmod +x "$1"; }; }
get linuxdeploy-x86_64.AppImage \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
get linuxdeploy-plugin-qt-x86_64.AppImage \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

export QML_SOURCES_PATHS="$ROOT/src"
./linuxdeploy-x86_64.AppImage \
    --appdir "$APPDIR" \
    --plugin qt \
    --desktop-file "$APPDIR/usr/share/applications/pacnscanner.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/scalable/apps/pacnscanner.svg" \
    --output appimage

echo "AppImage written to $OUT"
