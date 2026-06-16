#pragma once

#include <QFlags>
#include <QString>

namespace pacn {

// --- Discovery / scan configuration enums -----------------------------------

enum class ScanMethod {
    None        = 0x00,
    Arp         = 0x01,  // system ARP table mapping (+ active ARP if Npcap)
    Icmp        = 0x02,  // ICMP echo (raw if privileged, else system ping)
    TcpConnect  = 0x04,  // privilege-free TCP connect probe (default)
    Raw         = 0x08,  // raw-socket / Npcap advanced scan (optional)
};
Q_DECLARE_FLAGS(ScanMethods, ScanMethod)

enum class SpeedProfile { Slow, Normal, Fast };

enum class HostnameMethod {
    None    = 0x00,
    Dns     = 0x01,  // reverse DNS (PTR)
    Mdns    = 0x02,  // multicast DNS
    NetBios = 0x04,  // NetBIOS name service
};
Q_DECLARE_FLAGS(HostnameMethods, HostnameMethod)

// --- Device-facing enums -----------------------------------------------------

enum class PortState { Closed, Open, Filtered };

enum class DeviceStatus { Unknown, Online, Offline, New, Lost };

enum class DeviceType {
    Unknown, Gateway, Router, Switch, AccessPoint, Server, Computer, Laptop,
    Phone, Tablet, Printer, Nas, Camera, MediaPlayer, IoT, VirtualMachine
};

enum class RiskLevel { Info, Low, Medium, High, Critical };

// --- Small value types -------------------------------------------------------

struct Port {
    quint16 number = 0;
    QString protocol = QStringLiteral("tcp");
    PortState state = PortState::Closed;
    QString service;
    QString banner;
};

struct RiskFinding {
    RiskLevel level = RiskLevel::Info;
    QString title;
    QString detail;
};

// --- String conversion helpers ----------------------------------------------

inline QString toString(SpeedProfile p) {
    switch (p) {
        case SpeedProfile::Slow:   return QStringLiteral("Slow");
        case SpeedProfile::Normal: return QStringLiteral("Normal");
        case SpeedProfile::Fast:   return QStringLiteral("Fast");
    }
    return QStringLiteral("Normal");
}

inline SpeedProfile speedProfileFromString(const QString& s) {
    const QString v = s.trimmed().toLower();
    if (v == QLatin1String("slow")) return SpeedProfile::Slow;
    if (v == QLatin1String("fast")) return SpeedProfile::Fast;
    return SpeedProfile::Normal;
}

inline QString toString(DeviceStatus s) {
    switch (s) {
        case DeviceStatus::Online:  return QStringLiteral("Online");
        case DeviceStatus::Offline: return QStringLiteral("Offline");
        case DeviceStatus::New:     return QStringLiteral("New");
        case DeviceStatus::Lost:    return QStringLiteral("Lost");
        case DeviceStatus::Unknown: return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

inline QString toString(DeviceType t) {
    switch (t) {
        case DeviceType::Gateway:        return QStringLiteral("Gateway");
        case DeviceType::Router:         return QStringLiteral("Router");
        case DeviceType::Switch:         return QStringLiteral("Switch");
        case DeviceType::AccessPoint:    return QStringLiteral("Access Point");
        case DeviceType::Server:         return QStringLiteral("Server");
        case DeviceType::Computer:       return QStringLiteral("Computer");
        case DeviceType::Laptop:         return QStringLiteral("Laptop");
        case DeviceType::Phone:          return QStringLiteral("Phone");
        case DeviceType::Tablet:         return QStringLiteral("Tablet");
        case DeviceType::Printer:        return QStringLiteral("Printer");
        case DeviceType::Nas:            return QStringLiteral("NAS");
        case DeviceType::Camera:         return QStringLiteral("Camera");
        case DeviceType::MediaPlayer:    return QStringLiteral("Media Player");
        case DeviceType::IoT:            return QStringLiteral("IoT Device");
        case DeviceType::VirtualMachine: return QStringLiteral("Virtual Machine");
        case DeviceType::Unknown:        return QStringLiteral("Unknown");
    }
    return QStringLiteral("Unknown");
}

inline QString toString(RiskLevel r) {
    switch (r) {
        case RiskLevel::Info:     return QStringLiteral("Info");
        case RiskLevel::Low:      return QStringLiteral("Low");
        case RiskLevel::Medium:   return QStringLiteral("Medium");
        case RiskLevel::High:     return QStringLiteral("High");
        case RiskLevel::Critical: return QStringLiteral("Critical");
    }
    return QStringLiteral("Info");
}

}  // namespace pacn

Q_DECLARE_OPERATORS_FOR_FLAGS(pacn::ScanMethods)
Q_DECLARE_OPERATORS_FOR_FLAGS(pacn::HostnameMethods)
