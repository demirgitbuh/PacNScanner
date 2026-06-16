#!/usr/bin/env bash
# Generate the platform icon sets (ICO, ICNS, PNG) from assets/logo.svg.
# Requires: inkscape or rsvg-convert (SVG -> PNG), icnsutils or iconutil (ICNS),
#           ImageMagick `convert` (ICO).
#
# Usage:  ./packaging/generate-icons.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SVG="$ROOT/assets/logo.svg"
OUT="$ROOT/assets/icons"
mkdir -p "$OUT"

render() { # render <size> <outfile>
  if command -v rsvg-convert >/dev/null; then
    rsvg-convert -w "$1" -h "$1" "$SVG" -o "$2"
  elif command -v inkscape >/dev/null; then
    inkscape "$SVG" --export-type=png --export-filename="$2" -w "$1" -h "$1"
  else
    echo "Need rsvg-convert or inkscape" >&2; exit 1
  fi
}

for s in 16 24 32 48 64 128 256 512 1024; do
  render "$s" "$OUT/app-$s.png"
done
cp "$OUT/app-256.png" "$OUT/app.png"

# Windows .ico
if command -v convert >/dev/null; then
  convert "$OUT/app-16.png" "$OUT/app-24.png" "$OUT/app-32.png" \
          "$OUT/app-48.png" "$OUT/app-64.png" "$OUT/app-256.png" "$OUT/app.ico"
fi

# macOS .icns
if command -v iconutil >/dev/null; then
  ICONSET="$OUT/app.iconset"; mkdir -p "$ICONSET"
  cp "$OUT/app-16.png"   "$ICONSET/icon_16x16.png"
  cp "$OUT/app-32.png"   "$ICONSET/icon_16x16@2x.png"
  cp "$OUT/app-32.png"   "$ICONSET/icon_32x32.png"
  cp "$OUT/app-64.png"   "$ICONSET/icon_32x32@2x.png"
  cp "$OUT/app-128.png"  "$ICONSET/icon_128x128.png"
  cp "$OUT/app-256.png"  "$ICONSET/icon_128x128@2x.png"
  cp "$OUT/app-256.png"  "$ICONSET/icon_256x256.png"
  cp "$OUT/app-512.png"  "$ICONSET/icon_256x256@2x.png"
  cp "$OUT/app-512.png"  "$ICONSET/icon_512x512.png"
  cp "$OUT/app-1024.png" "$ICONSET/icon_512x512@2x.png"
  iconutil -c icns "$ICONSET" -o "$OUT/app.icns"
elif command -v png2icns >/dev/null; then
  png2icns "$OUT/app.icns" "$OUT/app-16.png" "$OUT/app-32.png" \
           "$OUT/app-128.png" "$OUT/app-256.png" "$OUT/app-512.png"
fi

echo "Icons written to $OUT"
