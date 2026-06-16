#include <QFile>

#include <pacn/version.h>

#include "reporting/Exporter.h"

namespace pacn::reporting {

namespace {

QString esc(const QString& s) {
    QString o = s;
    o.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    o.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    o.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    o.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    return o;
}

QString riskBadge(RiskLevel level) {
    QString color, text = toString(level);
    switch (level) {
        case RiskLevel::Critical: color = QStringLiteral("#c0392b"); break;
        case RiskLevel::High:     color = QStringLiteral("#e67e22"); break;
        case RiskLevel::Medium:   color = QStringLiteral("#f1c40f"); break;
        case RiskLevel::Low:      color = QStringLiteral("#27ae60"); break;
        case RiskLevel::Info:     color = QStringLiteral("#7f8c8d"); break;
    }
    return QStringLiteral("<span class=\"badge\" style=\"background:%1\">%2</span>")
        .arg(color, esc(text));
}

QString logoSvg() {
    QFile f(QStringLiteral(":/logo.svg"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    return QString::fromUtf8(f.readAll());
}

}  // namespace

QString toHtml(const QList<Device>& devices, const ReportContext& ctx) {
    QString rows;
    for (const Device& d : devices) {
        rows += QStringLiteral(
                    "<tr><td>%1</td><td class=\"mono\">%2</td><td>%3</td><td>%4</td>"
                    "<td>%5</td><td>%6</td><td class=\"mono\">%7</td><td>%8</td>"
                    "<td>%9</td><td>%10</td></tr>")
                    .arg(esc(d.ipString()), esc(d.mac.isEmpty() ? QStringLiteral("—") : d.mac),
                         esc(d.hostname), esc(d.vendor), esc(d.osGuess), esc(toString(d.type)),
                         esc(d.openPortNumbers().join(QStringLiteral(", "))),
                         esc(d.serviceNames().join(QStringLiteral(", "))))
                    .arg(riskBadge(d.riskLevel), esc(toString(d.status)));
    }

    QString riskSection;
    for (const Device& d : devices) {
        if (d.risks.isEmpty()) continue;
        bool meaningful = false;
        for (const RiskFinding& f : d.risks)
            if (f.level >= RiskLevel::Medium) meaningful = true;
        if (!meaningful) continue;
        riskSection += QStringLiteral("<h3>%1 <small>%2</small></h3><ul>")
                           .arg(esc(d.ipString()), esc(d.hostname));
        for (const RiskFinding& f : d.risks) {
            riskSection += QStringLiteral("<li>%1 <b>%2</b> — %3</li>")
                               .arg(riskBadge(f.level), esc(f.title), esc(f.detail));
        }
        riskSection += QStringLiteral("</ul>");
    }
    if (riskSection.isEmpty())
        riskSection = QStringLiteral("<p class=\"muted\">No medium-or-higher risks detected.</p>");

    const QString secs = ctx.summary.elapsedMs > 0
                             ? QString::number(ctx.summary.elapsedMs / 1000.0, 'f', 1)
                             : QStringLiteral("0");

    return QStringLiteral(R"HTML(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<title>%1</title>
<style>
:root{--accent:#2BBFD6;--ink:#1f2933;--muted:#7b8794;--bg:#f7fafc;--card:#fff;--border:#e4e9f0;}
*{box-sizing:border-box}body{margin:0;font-family:'Segoe UI',Roboto,Helvetica,Arial,sans-serif;
background:var(--bg);color:var(--ink);padding:32px;}
.header{display:flex;align-items:center;gap:18px;margin-bottom:8px}
.header svg{width:64px;height:64px}
.header h1{margin:0;font-size:24px}
.meta{color:var(--muted);font-size:13px;margin-bottom:24px}
.cards{display:flex;gap:16px;flex-wrap:wrap;margin-bottom:28px}
.card{background:var(--card);border:1px solid var(--border);border-radius:14px;
padding:18px 22px;min-width:150px}
.card .v{font-size:30px;font-weight:700}.card .l{color:var(--muted);font-size:12px;text-transform:uppercase;letter-spacing:.5px}
.card.accent{border-color:var(--accent)}
table{width:100%;border-collapse:collapse;background:var(--card);border:1px solid var(--border);
border-radius:12px;overflow:hidden;font-size:13px}
th,td{padding:9px 12px;text-align:left;border-bottom:1px solid var(--border)}
th{background:#f0f5f8;color:var(--muted);text-transform:uppercase;font-size:11px;letter-spacing:.4px}
tr:last-child td{border-bottom:none}
.mono{font-family:'Cascadia Code',Consolas,monospace}
.badge{color:#fff;padding:2px 9px;border-radius:20px;font-size:11px;font-weight:600}
.muted{color:var(--muted)}
h2{margin-top:34px;border-bottom:2px solid var(--accent);padding-bottom:6px;display:inline-block}
ul{line-height:1.7}.footer{margin-top:40px;color:var(--muted);font-size:12px}
.ethics{margin-top:24px;padding:14px 18px;background:#fff7ed;border:1px solid #fed7aa;border-radius:10px;font-size:13px}
</style></head>
<body>
<div class="header">%2<div><h1>%3</h1></div></div>
<div class="meta">Network: <b>%4</b> &nbsp;·&nbsp; Adapter: %5 &nbsp;·&nbsp; Generated: %6
&nbsp;·&nbsp; PacNScanner %7</div>
<div class="cards">
<div class="card accent"><div class="v">%8</div><div class="l">Devices</div></div>
<div class="card"><div class="v">%9</div><div class="l">Hosts up</div></div>
<div class="card"><div class="v">%10</div><div class="l">At risk</div></div>
<div class="card"><div class="v">%11s</div><div class="l">Duration</div></div>
</div>
<h2>Devices</h2>
<table><thead><tr><th>IP</th><th>MAC</th><th>Hostname</th><th>Vendor</th><th>OS</th>
<th>Type</th><th>Open ports</th><th>Services</th><th>Risk</th><th>Status</th></tr></thead>
<tbody>%12</tbody></table>
<h2>Risk findings</h2>
%13
<div class="ethics"><b>Ethical use:</b> PacNScanner is intended only for networks you own or are
explicitly authorized to assess. Scanning third-party networks without permission may be illegal.</div>
<div class="footer">Report generated by PacNScanner — data stays on your device. No telemetry.</div>
</body></html>)HTML")
        .arg(esc(ctx.title), logoSvg(), esc(ctx.title), esc(ctx.network),
             esc(ctx.adapter.isEmpty() ? QStringLiteral("—") : ctx.adapter),
             esc(ctx.when.toString(Qt::ISODate)), QString::fromLatin1(kVersionString))
        .arg(devices.size())
        .arg(ctx.summary.hostsUp)
        .arg(ctx.summary.risky)
        .arg(secs)
        .arg(rows, riskSection);
}

}  // namespace pacn::reporting
