# PacNScanner — User Guide

## First launch

PacNScanner detects your active network adapter and pre-fills a suggested target (your local
`/24`). If **auto-scan on startup** is enabled (default), a scan begins shortly after launch.
There is no onboarding wizard — the dashboard is ready immediately.

## The dashboard

- **Summary cards** — total devices, online, at-risk, and newly discovered counts.
- **Charts** — device-type donut and top-services bar chart, updated live.
- **Active network panel** — adapter, IP/CIDR, detected gateway, Wi-Fi SSID/signal, and your
  privilege level (elevated unlocks raw scanning).
- **Device table** — every discovered device with smart sorting and filtering.
- **Topology tab** — a radial map centred on the gateway.
- **Log dock** — toggle from **View → Log**; mirrors the on-disk log.

## Running a scan

1. Enter one or more **targets** in the box: CIDR (`192.168.1.0/24`), ranges
   (`10.0.0.1-10.0.0.50` or `10.0.0.1-50`), single hosts (`10.0.0.5`), or IPv6 (`fe80::/64`).
   Separate multiple targets with commas or spaces.
2. Pick an **adapter**, **speed** profile (Slow/Normal/Fast) and **port** profile.
3. Press **Start Scan** (Ctrl+R). Use **Pause/Resume** and **Stop** as needed.

For very large ranges PacNScanner shows an estimated host count, duration and network load,
and asks for confirmation — there is no hard size limit.

## Working with devices

- **Double-click** a row (or a topology node) to open the **device detail** dialog: full
  properties, open ports/services, banners, risk findings, labels and a favourite toggle.
- **Filter** with the search box (matches IP, MAC, hostname, vendor, service). Toggle
  **Risky only**, **Favorites**, and **Priority sort** (floats favourites/risky/new to the top).
- **Sort** by any column; IPs sort numerically, risk by score.
- **Label** devices and mark **favourites** — both persist across scans.

## Status & risk

| Status | Meaning |
|--------|---------|
| 🟢 Online | Responded to this scan |
| 🔵 New | Seen for the first time |
| 🟠 Lost | Previously seen, now missing |
| ⚪ Offline/Unknown | Not currently reachable |

Risk levels (Info → Low → Medium → High → Critical) come from rule-based scoring of open
ports and services. Hover the **Risk** cell for the findings.

## Export & import

- **File → Export report** — choose `.html` (professional report with logo, summary, devices,
  ports, services, risks and timestamp), `.csv`, or `.json`.
- **File → Import** — load a previously exported `.json` or `.csv`.

## Settings

**Tools → Settings** (Ctrl+,):
- **General** — auto-scan, periodic re-scan + interval, history, tray, notifications, start on
  login (off by default).
- **Scan** — methods, speed, hostname resolution (DNS/mDNS/NetBIOS), OS guess, banners,
  optional online vendor lookup.
- **Ports** — default profile and custom port list/ranges.
- **Appearance** — theme (system/light/dark) and language (system by default).
- **Shortcuts** — customise keyboard shortcuts.
- **Privacy** — privacy & ethical-use notes.

## Tray & notifications

When enabled, PacNScanner keeps a tray icon and can notify you about new/lost devices. Closing
the window minimises to the tray; quit from the tray menu or **File → Quit**.

## Logs

Logs are written under your platform's app-data folder (`.../PacNScanner/logs/pacnscanner.log`)
and mirrored in the in-app log panel. Error dialogs reference this file.

## Ethical use

Only scan networks you own or are explicitly authorized to assess.
