#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QString>

#include "core/Device.h"
#include "core/ScanEngine.h"  // ScanSummary

namespace pacn::reporting {

// Metadata shown in the report header / JSON envelope.
struct ReportContext {
    QString title = QStringLiteral("PacNScanner Report");
    QString network;   // e.g. "192.168.1.0/24"
    QString adapter;   // e.g. "Wi-Fi (192.168.1.34)"
    QDateTime when = QDateTime::currentDateTime();
    ScanSummary summary;
};

// --- Export ---------------------------------------------------------------
QString toCsv(const QList<Device>& devices);
bool exportCsv(const QList<Device>& devices, const QString& path, QString* err = nullptr);

QByteArray toJson(const QList<Device>& devices, const ReportContext& ctx);
bool exportJson(const QList<Device>& devices, const ReportContext& ctx, const QString& path,
                QString* err = nullptr);

QString toHtml(const QList<Device>& devices, const ReportContext& ctx);
bool exportHtml(const QList<Device>& devices, const ReportContext& ctx, const QString& path,
                QString* err = nullptr);

// --- Import ---------------------------------------------------------------
QList<Device> importJson(const QString& path, QString* err = nullptr);
QList<Device> importCsv(const QString& path, QString* err = nullptr);

}  // namespace pacn::reporting
