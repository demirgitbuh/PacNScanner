# Packaging PacNScanner

Packaging uses **CPack** plus platform deploy tools. The GUI target runs
`qt_generate_deploy_app_script` on install, so `cmake --install` bundles the Qt runtime
(via `windeployqt`/`macdeployqt`) and CPack archives are standalone.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DPACN_BUILD_TESTS=OFF
cmake --build build --parallel
cd build && cpack            # uses the per-OS generators from cmake/packaging.cmake
```

## Windows

- **Portable ZIP** — `cpack -G ZIP`
- **EXE installer (NSIS)** — `cpack -G NSIS` (requires `makensis`; `choco install nsis`)
- **MSI (WiX)** — `cpack -G WIX` (requires the WiX Toolset)

Code signing is **not required** initially; signing hooks can be added later.

## macOS

- **DMG** — `cpack -G DragNDrop` (bundles the `.app` via `macdeployqt`)
- **Notarization** is wired into the release workflow and activates when the signing secrets
  (`MACOS_CERT`, `AC_USERNAME`, `AC_PASSWORD`, team id) are present; otherwise an unsigned
  build is produced.

## Linux

- **DEB / RPM** — `cpack -G DEB` / `cpack -G RPM` (RPM needs `rpmbuild`)
- **AppImage** — `bash packaging/build-appimage.sh build` (uses `linuxdeploy` + Qt plugin)
- **Flatpak** — `flatpak-builder --user --install build-flatpak \
  packaging/flatpak/io.github.demirgitbuh.PacNScanner.yml`
- A `.desktop` file, scalable icon (`pacnscanner.svg`) and AppStream metainfo are installed
  for desktop/icon-theme integration.

## Icons

`packaging/generate-icons.sh` renders `assets/logo.svg` into the ICO/ICNS/PNG sets used by the
installers and desktop integration.

## CI/CD

`.github/workflows/release.yml` builds on Windows, macOS and Linux, runs the packagers above
and uploads every produced artifact to the GitHub Release for the pushed tag.
