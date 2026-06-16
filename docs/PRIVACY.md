# Privacy Policy

_Last updated: 2026-06-15_

PacNScanner is built to respect your privacy.

## No telemetry

PacNScanner collects **no** usage analytics, crash telemetry or tracking of any kind. It does
not phone home.

## Your data stays local

- Scan results, device inventory, history, labels and favourites are stored **only on your
  device** (SQLite database + QSettings under your platform's app-data folder).
- Logs are written locally (`.../PacNScanner/logs/pacnscanner.log`).
- Exported reports (CSV/JSON/HTML) are written wherever you choose.

## Network connections PacNScanner makes

| Feature | Connects to | Default | Notes |
|---------|-------------|---------|-------|
| Scanning | hosts on the networks **you** target | n/a | the core function |
| Reverse DNS / mDNS / NetBIOS | your DNS / local network | on | resolves hostnames |
| Update check | `api.github.com` | on | checks for a newer release on startup; sends only a standard HTTP request |
| Online vendor lookup | a MAC-vendor API | **off** | optional; only resolves MAC prefixes you choose |

You can disable the update check and online vendor lookup in **Settings**.

## Ethical & authorized use

PacNScanner is intended **only** for networks you own or are explicitly authorized to assess.
Scanning networks without permission may be illegal in your jurisdiction. You are responsible
for how you use this tool.

## Contact

Questions? Open an issue at https://github.com/demirgitbuh/PacNScanner/issues.
