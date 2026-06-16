#pragma once

#include <QDateTime>
#include <QSqlDatabase>
#include <QString>

#include "core/Device.h"
#include "core/ScanEngine.h"  // ScanSummary

namespace pacn {

// SQLite-backed persistence: device inventory, per-scan history (optional),
// labels and favourites. All operations are best-effort and never throw;
// failures are reported via lastError().
class Database {
public:
    Database() = default;
    ~Database();

    bool open(const QString& path);
    void close();
    bool isOpen() const;
    QString lastError() const { return lastError_; }

    // --- Scans & devices ----------------------------------------------------
    qint64 insertScan(const QDateTime& when, const QString& targets,
                      const ScanSummary& summary);
    bool upsertDevice(const Device& device);
    bool recordHistory(const Device& device, qint64 scanId);

    QList<Device> loadDevices();
    QList<QDateTime> historyForKey(const QString& key);
    int deviceCount();

    // --- Labels & favourites (persist across scans) -------------------------
    void setFavorite(const QString& key, bool favorite);
    bool favorite(const QString& key);
    void addLabel(const QString& key, const QString& label);
    void removeLabel(const QString& key, const QString& label);
    QStringList labels(const QString& key);

private:
    bool ensureSchema();
    bool exec(const QString& sql);

    QSqlDatabase db_;
    QString connectionName_;
    QString lastError_;
};

}  // namespace pacn
