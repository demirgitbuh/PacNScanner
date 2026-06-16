#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "reporting/Exporter.h"

namespace pacn::reporting {

namespace {

// Splits one CSV record honouring quotes and escaped double-quotes.
QStringList parseCsvLine(const QString& line) {
    QStringList fields;
    QString cur;
    bool inQuotes = false;
    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line[i];
        if (inQuotes) {
            if (c == QLatin1Char('"')) {
                if (i + 1 < line.size() && line[i + 1] == QLatin1Char('"')) {
                    cur.append(QLatin1Char('"'));
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                cur.append(c);
            }
        } else {
            if (c == QLatin1Char('"'))
                inQuotes = true;
            else if (c == QLatin1Char(','))
                fields << cur, cur.clear();
            else
                cur.append(c);
        }
    }
    fields << cur;
    return fields;
}

}  // namespace

QList<Device> importJson(const QString& path, QString* err) {
    QList<Device> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (err) *err = f.errorString();
        return out;
    }
    QJsonParseError pe{};
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    if (pe.error != QJsonParseError::NoError) {
        if (err) *err = pe.errorString();
        return out;
    }
    QJsonArray arr;
    if (doc.isArray())
        arr = doc.array();
    else if (doc.isObject())
        arr = doc.object().value(QStringLiteral("devices")).toArray();
    for (const QJsonValue& v : arr)
        if (v.isObject()) out.push_back(Device::fromJson(v.toObject()));
    return out;
}

QList<Device> importCsv(const QString& path, QString* err) {
    QList<Device> out;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (err) *err = f.errorString();
        return out;
    }
    const QString text = QString::fromUtf8(f.readAll());
    const QStringList lines = text.split(QRegularExpression(QStringLiteral("\r\n|\n|\r")),
                                         Qt::SkipEmptyParts);
    if (lines.isEmpty()) return out;

    const QStringList header = parseCsvLine(lines.first());
    QHash<QString, int> col;
    for (int i = 0; i < header.size(); ++i) col.insert(header[i].trimmed().toLower(), i);

    const auto at = [&](const QStringList& f, const QString& name) -> QString {
        const int idx = col.value(name, -1);
        return (idx >= 0 && idx < f.size()) ? f[idx] : QString();
    };

    for (int i = 1; i < lines.size(); ++i) {
        const QStringList fields = parseCsvLine(lines[i]);
        if (fields.isEmpty()) continue;
        Device d;
        d.ip = QHostAddress(at(fields, QStringLiteral("ip")));
        d.mac = at(fields, QStringLiteral("mac"));
        d.hostname = at(fields, QStringLiteral("hostname"));
        d.vendor = at(fields, QStringLiteral("vendor"));
        d.osGuess = at(fields, QStringLiteral("os"));
        d.riskScore = at(fields, QStringLiteral("risk")).toInt();
        const QString ports = at(fields, QStringLiteral("openports"));
        for (const QString& p :
             ports.split(QRegularExpression(QStringLiteral("[ ,]")), Qt::SkipEmptyParts)) {
            bool ok = false;
            const quint16 n = static_cast<quint16>(p.toUInt(&ok));
            if (ok) {
                Port port;
                port.number = n;
                port.state = PortState::Open;
                d.ports.push_back(port);
            }
        }
        const QString labels = at(fields, QStringLiteral("labels"));
        if (!labels.isEmpty())
            d.labels = labels.split(QLatin1Char('|'), Qt::SkipEmptyParts);
        if (!d.ip.isNull()) out.push_back(d);
    }
    return out;
}

}  // namespace pacn::reporting
