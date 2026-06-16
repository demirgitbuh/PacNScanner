#include "storage/Database.h"

#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QVariant>

namespace pacn {

Database::~Database() { close(); }

bool Database::open(const QString& path) {
    close();
    connectionName_ = QStringLiteral("pacn-%1").arg(QUuid::createUuid().toString());
    db_ = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName_);
    db_.setDatabaseName(path);
    if (!db_.open()) {
        lastError_ = db_.lastError().text();
        return false;
    }
    exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    exec(QStringLiteral("PRAGMA foreign_keys=ON"));
    return ensureSchema();
}

void Database::close() {
    if (db_.isOpen()) db_.close();
    if (!connectionName_.isEmpty()) {
        db_ = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName_);
        connectionName_.clear();
    }
}

bool Database::isOpen() const { return db_.isOpen(); }

bool Database::exec(const QString& sql) {
    QSqlQuery q(db_);
    if (!q.exec(sql)) {
        lastError_ = q.lastError().text();
        return false;
    }
    return true;
}

bool Database::ensureSchema() {
    return exec(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS scans ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "ts TEXT, targets TEXT, scanned INTEGER, up INTEGER,"
               "risky INTEGER, elapsed_ms INTEGER)")) &&
           exec(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS devices ("
               "key TEXT PRIMARY KEY, ip TEXT, mac TEXT, hostname TEXT,"
               "vendor TEXT, type TEXT, status TEXT, risk INTEGER,"
               "favorite INTEGER DEFAULT 0, first_seen TEXT, last_seen TEXT,"
               "json TEXT)")) &&
           exec(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS device_history ("
               "key TEXT, scan_id INTEGER, ts TEXT, json TEXT)")) &&
           exec(QStringLiteral(
               "CREATE TABLE IF NOT EXISTS labels ("
               "key TEXT, label TEXT, UNIQUE(key,label))"));
}

qint64 Database::insertScan(const QDateTime& when, const QString& targets,
                            const ScanSummary& summary) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "INSERT INTO scans (ts,targets,scanned,up,risky,elapsed_ms) "
        "VALUES (?,?,?,?,?,?)"));
    q.addBindValue(when.toString(Qt::ISODate));
    q.addBindValue(targets);
    q.addBindValue(static_cast<qulonglong>(summary.scanned));
    q.addBindValue(static_cast<qulonglong>(summary.hostsUp));
    q.addBindValue(static_cast<qulonglong>(summary.risky));
    q.addBindValue(static_cast<qlonglong>(summary.elapsedMs));
    if (!q.exec()) {
        lastError_ = q.lastError().text();
        return -1;
    }
    return q.lastInsertId().toLongLong();
}

bool Database::upsertDevice(const Device& device) {
    const QByteArray json = QJsonDocument(device.toJson()).toJson(QJsonDocument::Compact);
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "INSERT INTO devices (key,ip,mac,hostname,vendor,type,status,risk,"
        "favorite,first_seen,last_seen,json) VALUES (?,?,?,?,?,?,?,?,"
        "COALESCE((SELECT favorite FROM devices WHERE key=?),?),"
        "COALESCE((SELECT first_seen FROM devices WHERE key=?),?),?,?) "
        "ON CONFLICT(key) DO UPDATE SET ip=excluded.ip, mac=excluded.mac,"
        "hostname=excluded.hostname, vendor=excluded.vendor, type=excluded.type,"
        "status=excluded.status, risk=excluded.risk, last_seen=excluded.last_seen,"
        "json=excluded.json"));
    const QString key = device.key();
    const QString firstSeen = device.firstSeen.isValid()
                                  ? device.firstSeen.toString(Qt::ISODate)
                                  : QDateTime::currentDateTime().toString(Qt::ISODate);
    const QString lastSeen = device.lastSeen.isValid()
                                 ? device.lastSeen.toString(Qt::ISODate)
                                 : firstSeen;
    q.addBindValue(key);
    q.addBindValue(device.ipString());
    q.addBindValue(device.mac);
    q.addBindValue(device.hostname);
    q.addBindValue(device.vendor);
    q.addBindValue(toString(device.type));
    q.addBindValue(toString(device.status));
    q.addBindValue(device.riskScore);
    q.addBindValue(key);                         // favorite subquery key
    q.addBindValue(device.favorite ? 1 : 0);     // favorite default
    q.addBindValue(key);                         // first_seen subquery key
    q.addBindValue(firstSeen);                   // first_seen default
    q.addBindValue(lastSeen);
    q.addBindValue(QString::fromUtf8(json));
    if (!q.exec()) {
        lastError_ = q.lastError().text();
        return false;
    }
    return true;
}

bool Database::recordHistory(const Device& device, qint64 scanId) {
    const QByteArray json = QJsonDocument(device.toJson()).toJson(QJsonDocument::Compact);
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "INSERT INTO device_history (key,scan_id,ts,json) VALUES (?,?,?,?)"));
    q.addBindValue(device.key());
    q.addBindValue(scanId);
    q.addBindValue(QDateTime::currentDateTime().toString(Qt::ISODate));
    q.addBindValue(QString::fromUtf8(json));
    if (!q.exec()) {
        lastError_ = q.lastError().text();
        return false;
    }
    return true;
}

QList<Device> Database::loadDevices() {
    QList<Device> out;
    QSqlQuery q(db_);
    if (!q.exec(QStringLiteral("SELECT json,favorite FROM devices"))) {
        lastError_ = q.lastError().text();
        return out;
    }
    while (q.next()) {
        const QJsonDocument doc =
            QJsonDocument::fromJson(q.value(0).toString().toUtf8());
        Device d = Device::fromJson(doc.object());
        d.favorite = q.value(1).toInt() != 0;
        d.labels = labels(d.key());
        out.push_back(d);
    }
    return out;
}

QList<QDateTime> Database::historyForKey(const QString& key) {
    QList<QDateTime> out;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral(
        "SELECT ts FROM device_history WHERE key=? ORDER BY ts DESC"));
    q.addBindValue(key);
    if (q.exec())
        while (q.next())
            out.push_back(QDateTime::fromString(q.value(0).toString(), Qt::ISODate));
    return out;
}

int Database::deviceCount() {
    QSqlQuery q(db_);
    if (q.exec(QStringLiteral("SELECT COUNT(*) FROM devices")) && q.next())
        return q.value(0).toInt();
    return 0;
}

void Database::setFavorite(const QString& key, bool favorite) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("UPDATE devices SET favorite=? WHERE key=?"));
    q.addBindValue(favorite ? 1 : 0);
    q.addBindValue(key);
    q.exec();
}

bool Database::favorite(const QString& key) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT favorite FROM devices WHERE key=?"));
    q.addBindValue(key);
    if (q.exec() && q.next()) return q.value(0).toInt() != 0;
    return false;
}

void Database::addLabel(const QString& key, const QString& label) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("INSERT OR IGNORE INTO labels (key,label) VALUES (?,?)"));
    q.addBindValue(key);
    q.addBindValue(label);
    q.exec();
}

void Database::removeLabel(const QString& key, const QString& label) {
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("DELETE FROM labels WHERE key=? AND label=?"));
    q.addBindValue(key);
    q.addBindValue(label);
    q.exec();
}

QStringList Database::labels(const QString& key) {
    QStringList out;
    QSqlQuery q(db_);
    q.prepare(QStringLiteral("SELECT label FROM labels WHERE key=? ORDER BY label"));
    q.addBindValue(key);
    if (q.exec())
        while (q.next()) out << q.value(0).toString();
    return out;
}

}  // namespace pacn
