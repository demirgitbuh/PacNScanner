#include "reporting/Exporter.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTextStream>

namespace pacn::reporting {

namespace {

QString csvField(const QString& value) {
    QString v = value;
    if (v.contains(QLatin1Char(',')) || v.contains(QLatin1Char('"')) ||
        v.contains(QLatin1Char('\n')) || v.contains(QLatin1Char('\r'))) {
        v.replace(QLatin1Char('"'), QStringLiteral("\"\""));
        return QLatin1Char('"') + v + QLatin1Char('"');
    }
    return v;
}

bool writeAtomic(const QString& path, const QByteArray& data, QString* err) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (err) *err = file.errorString();
        return false;
    }
    file.write(data);
    if (!file.commit()) {
        if (err) *err = file.errorString();
        return false;
    }
    return true;
}

}  // namespace

QString toCsv(const QList<Device>& devices) {
    QString out;
    QTextStream ts(&out);
    ts << "IP,MAC,Hostname,Vendor,OS,Type,Status,Risk,RiskLevel,OpenPorts,Services,Labels\n";
    for (const Device& d : devices) {
        ts << csvField(d.ipString()) << ','
           << csvField(d.mac) << ','
           << csvField(d.hostname) << ','
           << csvField(d.vendor) << ','
           << csvField(d.osGuess) << ','
           << csvField(toString(d.type)) << ','
           << csvField(toString(d.status)) << ','
           << d.riskScore << ','
           << csvField(toString(d.riskLevel)) << ','
           << csvField(d.openPortNumbers().join(QLatin1Char(' '))) << ','
           << csvField(d.serviceNames().join(QLatin1Char(' '))) << ','
           << csvField(d.labels.join(QLatin1Char('|'))) << '\n';
    }
    return out;
}

bool exportCsv(const QList<Device>& devices, const QString& path, QString* err) {
    return writeAtomic(path, toCsv(devices).toUtf8(), err);
}

QByteArray toJson(const QList<Device>& devices, const ReportContext& ctx) {
    QJsonObject root;
    root[QStringLiteral("application")] = QStringLiteral("PacNScanner");
    root[QStringLiteral("title")] = ctx.title;
    root[QStringLiteral("network")] = ctx.network;
    root[QStringLiteral("adapter")] = ctx.adapter;
    root[QStringLiteral("generatedAt")] = ctx.when.toString(Qt::ISODate);

    QJsonObject summary;
    summary[QStringLiteral("scanned")] = static_cast<double>(ctx.summary.scanned);
    summary[QStringLiteral("hostsUp")] = static_cast<double>(ctx.summary.hostsUp);
    summary[QStringLiteral("risky")] = static_cast<double>(ctx.summary.risky);
    summary[QStringLiteral("elapsedMs")] = static_cast<double>(ctx.summary.elapsedMs);
    root[QStringLiteral("summary")] = summary;

    QJsonArray arr;
    for (const Device& d : devices) arr.append(d.toJson());
    root[QStringLiteral("devices")] = arr;

    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

bool exportJson(const QList<Device>& devices, const ReportContext& ctx, const QString& path,
                QString* err) {
    return writeAtomic(path, toJson(devices, ctx), err);
}

bool exportHtml(const QList<Device>& devices, const ReportContext& ctx, const QString& path,
                QString* err) {
    return writeAtomic(path, toHtml(devices, ctx).toUtf8(), err);
}

}  // namespace pacn::reporting
