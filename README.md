<div align="center">

<img src="assets/logo.svg" width="120" alt="PacNScanner logo"/>

# PacNScanner

**Fast, modern, cross-platform local network scanner.**

Discover every device on your network, inventory open ports and services, estimate device
types and operating systems, and surface basic security risks — in a clean, system-themed
dashboard built for home users, sysadmins and security professionals.

[![CI](https://github.com/demirgitbuh/PacNScanner/actions/workflows/ci.yml/badge.svg)](https://github.com/demirgitbuh/PacNScanner/actions/workflows/ci.yml)
![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C)
![Qt6](https://img.shields.io/badge/Qt-6-41CD52)
![License: MIT](https://img.shields.io/badge/License-MIT-2BBFD6)
![Platforms](https://img.shields.io/badge/platforms-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey)

</div>

---

## Why PacNScanner?

- ⚡ **Fast.** Highly concurrent scan engine with Slow/Normal/Fast profiles and adaptive
  timeouts. A typical `/24` finishes in seconds; large ranges stay measurable.
- 🖥️ **Modern UI.** A real dashboard — summary cards, live animated results, charts, an
  active-network panel, a device table with smart sorting, and a topology map. Follows your
  system light/dark theme with a turquoise accent.
- 🔍 **Thorough discovery.** ARP, ICMP and privilege-free TCP connect scanning, with optional
  raw/Npcap when available. IPv4 and advanced IPv6, multiple CIDR/ranges at once.
- 🧩 **Rich inventory.** Hostnames (reverse DNS / mDNS / NetBIOS), MAC vendor (local OUI db),
  OS guess, open ports, service + banner detection, device-type classification.
- 🛡️ **Security visibility.** Rule-based risk scoring with clear findings for plaintext,
  admin and database services.
- 🔒 **Private by design.** No telemetry. All data stays on your device.

> **Screenshots** — _add dashboard / device detail / topology screenshots here once built._
>
> | Dashboard | Device detail | Topology |
> |-----------|---------------|----------|
> | _todo_ | _todo_ | _todo_ |

---

## Quick start

```bash
# Build (see docs/BUILD.md for prerequisites: Qt 6.5+, CMake 3.21+, a C++23 compiler)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run the GUI
./build/bin/pacnscanner

# Or scan from the terminal
./build/bin/pacnscanner-cli 192.168.1.0/24 -p "Top 100" -o report.html
```

PacNScanner auto-detects your active adapter and suggests a target on first launch. Press
**Start Scan** (Ctrl+R) and watch devices stream in.

---

## Features at a glance

| Area | Highlights |
|------|-----------|
| **Discovery** | ARP · ICMP · TCP connect · optional raw/Npcap · IPv4 + advanced IPv6 · multiple targets |
| **Ports/services** | Top 100 / Top 1000 / Well-known / Full / custom · banner grabbing · service map |
| **Identity** | reverse DNS · mDNS · NetBIOS · OUI vendor lookup · OS guess · device-type classifier |
| **Security** | rule-based risk score · findings for plaintext/admin/db services · ethical-use guidance |
| **UX** | dashboard · live results · charts · topology · filters · smart IP sort · favorites · labels · custom shortcuts · i18n · accessibility |
| **Data** | SQLite inventory + history · QSettings · CSV/JSON/HTML export · JSON/CSV import · tray + notifications |
| **Ops** | CMake build · GoogleTest · GitHub Actions CI/CD · CPack (MSI/EXE/ZIP, DMG, AppImage/DEB/RPM/Flatpak) · CalVer · auto update check |

---

## Documentation

- 📘 [User Guide](docs/USER_GUIDE.md)
- 🛠️ [Developer Guide](docs/DEVELOPER_GUIDE.md)
- 🔧 [Build instructions](docs/BUILD.md)
- 📦 [Packaging](docs/PACKAGING.md)
- 🔒 [Privacy Policy](docs/PRIVACY.md)
- 🤝 [Contributing](CONTRIBUTING.md)

## Ethical use

PacNScanner is intended **only** for networks you own or are explicitly authorized to assess.
Scanning third-party networks without permission may be illegal in your jurisdiction. The
authors accept no liability for misuse.

## License

[MIT](LICENSE) © 2026 PacNScanner.
