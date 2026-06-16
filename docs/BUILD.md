# Building PacNScanner

## Prerequisites

| Tool | Version |
|------|---------|
| C++ compiler | C++23-capable: MSVC 2022, GCC 13+, or Clang 16+ / Apple Clang 15+ |
| CMake | 3.21+ |
| Qt | 6.5+ (Core, Network, Sql, Gui, Widgets, Svg; optional LinguistTools, Charts) |
| Ninja | recommended generator |

Optional:
- **Npcap SDK** (Windows) for raw/ARP-active scanning. Without it PacNScanner falls back to
  QtNetwork-only discovery.

## Getting Qt

- **Windows/macOS/Linux:** the [Qt Online Installer](https://www.qt.io/download-qt-installer)
  (select Qt 6.7, Desktop). Then point CMake at it:
  ```bash
  cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/Qt/6.7.3/<compiler>"
  ```
- **CI / reproducible:** the workflows use
  [`jurplel/install-qt-action`](https://github.com/jurplel/install-qt-action) (aqtinstall).

## Configure & build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Outputs land in `build/bin/`:
- `pacnscanner` — the GUI
- `pacnscanner-cli` — the command-line scanner

### CMake options

| Option | Default | Description |
|--------|---------|-------------|
| `PACN_BUILD_GUI` | ON | Build the Qt Widgets GUI |
| `PACN_BUILD_CLI` | ON | Build the headless CLI |
| `PACN_BUILD_TESTS` | ON | Build GoogleTest suites |
| `PACN_WITH_NPCAP` | OFF | Enable optional Npcap raw/ARP scanning (Windows) |

To build with Npcap:
```bash
cmake -S . -B build -DPACN_WITH_NPCAP=ON -DNPCAP_ROOT="C:/NpcapSDK"
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

The UI smoke test runs headless via the `offscreen` Qt platform plugin.

## Generate icons (optional)

```bash
./packaging/generate-icons.sh   # needs rsvg-convert/inkscape, ImageMagick, iconutil/png2icns
```

## Troubleshooting

- **`Qt6 not found`** — set `CMAKE_PREFIX_PATH` to your Qt `lib/cmake` location.
- **MSVC + Ninja** — run from a *Developer Command Prompt* (or use the Visual Studio
  generator) so the compiler is on `PATH`.
- **SQLite driver missing** — ensure the Qt `sqldrivers` plugins are installed alongside Qt.
