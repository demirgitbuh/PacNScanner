# PacNScanner — Developer Guide

## Architecture

PacNScanner is layered so the scan engine is fully testable and reusable, independent of the
GUI:

```
        ┌────────────┐      ┌────────────┐
        │  src/app   │      │  src/cli   │   entry points (GUI / headless)
        └─────┬──────┘      └─────┬──────┘
              │  src/ui (Qt Widgets dashboard)
              ▼                    ▼
        ┌──────────────────────────────────┐
        │  src/core  (pacn_core, no GUI)    │  ScanEngine, Device, IpRange, probes,
        │                                   │  service/risk/vendor/OS, logging
        └───┬───────────┬───────────┬───────┘
            │           │           │
   src/platform   src/storage   src/reporting
   (OS facts)     (SQLite/      (CSV/JSON/HTML)
                   QSettings)
```

- **`core`** depends only on `Qt::Core`/`Qt::Network`. It defines the `IPlatformServices`
  *port*; `platform` provides the OS *adapter* (and a `FakePlatform` for tests).
- The **scan engine** runs on a dedicated `QThread`; per-host work is parallelised on a
  `QThreadPool`. Probes are injected via `IProbeFactory`, so tests run fully offline.
- All long work is async; the UI never blocks. Errors are carried via `Result`/signals — the
  app does not crash on scan failures.

### Key types

| Type | File | Role |
|------|------|------|
| `ScanEngine` | `src/core/ScanEngine.*` | UI-facing orchestrator (start/pause/resume/cancel, signals) |
| `ScanWorker` | `src/core/ScanWorker.*` | Streams targets, runs probes on a pool, enriches devices |
| `IProbeFactory` / `*Probe` | `src/core/probe/` | Pluggable host/port probes (real + fake) |
| `IpRange` | `src/core/IpRange.*` | CIDR/range/host parsing, IPv4+IPv6, estimates |
| `Device` | `src/core/Device.*` | Inventory record + JSON (de)serialisation |
| `RiskEngine` / `DeviceClassifier` / `OsFingerprinter` | `src/core/` | Heuristics |
| `IPlatformServices` | `src/core/PlatformServices.h` | Adapters, ARP table, gateways, elevation |
| `DeviceTableModel` / `DeviceFilterProxyModel` | `src/ui/` | Table + smart sort/filter |

## Adding a discovery method

Implement `IHostProbe` (and/or `IPortProbe`) in `src/core/probe/`, then wire it into
`DefaultHostProbe`/`DefaultProbeFactory`. Honour the `ScanMethods` flags in `ScanConfig` and
keep the probe **reentrant** (create sockets/processes inside `probe()`).

## Adding a platform

Implement `IPlatformServices` under `src/platform/<os>/`, then return it from
`createPlatformServices()` (`PlatformFactory.cpp`). Reuse `adapterutil::enumerateViaQt()` for
adapter enumeration.

## Tests

GoogleTest suites live in `tests/`:
- **Unit:** IP/CIDR parsing, MAC/IP utils, port profiles, service detection, vendor lookup,
  risk scoring, classification, reporting round-trips.
- **Integration:** the full engine driven by `FakeProbeFactory` + `FakePlatform` — no network.
- **UI smoke:** model behaviour and `MainWindow` construction under the `offscreen` platform.

```bash
ctest --test-dir build --output-on-failure
```

## Code style

`.clang-format` (Google-based, 4-space indent, 100 cols) and `.clang-tidy` are provided. CI
runs clang-format, clang-tidy and cppcheck (informational) plus build+test on all three OSes.

## Performance targets & benchmarks

| Scenario | Target |
|----------|--------|
| `/24` (254 hosts), Normal | results within a few seconds |
| `/16` (~65k hosts), Fast | controlled, measurable; progress + ETA shown |
| Idle memory | tens of MB; targets are **streamed**, never fully materialised |
| Large ranges | no hard cap — estimated hosts/duration/load shown, confirmation requested |

Concurrency and timeouts scale with the speed profile (`ScanConfig::concurrency()` /
`hostTimeoutMs()` / `portTimeoutMs()`). When measuring, record host count, profile, wall-clock
time and peak RSS, and compare against the table above.

## Versioning & releases

CalVer (`YYYY.MINOR.PATCH`) defined in `cmake/version.cmake` and surfaced via
`pacn/version.h`. Tagging `vX`/`YYYY.M.P` triggers `.github/workflows/release.yml`, which
builds, packages with CPack and publishes artifacts to GitHub Releases. The app checks GitHub
Releases on startup and shows an unobtrusive banner when a newer build exists.
