#pragma once

#include <QDateTime>
#include <QHostAddress>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include "core/Types.h"

namespace pacn {

// A discovered network device and everything PacNScanner knows about it.
struct Device {
    QHostAddress ip;
    QString mac;
    QString hostname;
    QString vendor;
    QString osGuess;

    QList<Port> ports;
    DeviceType type = DeviceType::Unknown;

    int riskScore = 0;  // 0..100
    RiskLevel riskLevel = RiskLevel::Info;
    QList<RiskFinding> risks;

    DeviceStatus status = DeviceStatus::Unknown;

    QStringList labels;
    bool favorite = false;

    QDateTime firstSeen;
    QDateTime lastSeen;
    int rttMs = -1;
    int ttl = -1;
    QString adapter;
    QString discoveryMethod;

    // Identity used for de-duplication & history: prefer MAC, fall back to IP.
    QString key() const;
    QString ipString() const { return ip.toString(); }

    QList<Port> openPorts() const;
    QStringList openPortNumbers() const;   // ["22", "80", "443"]
    QStringList serviceNames() const;      // ["ssh", "http", "https"]
    bool isPrioritized() const;            // favorites / risky float to top

    QJsonObject toJson() const;
    static Device fromJson(const QJsonObject& obj);
};

}  // namespace pacn

Q_DECLARE_METATYPE(pacn::Device)
