#include "core/Device.h"

#include <QHash>
#include <QJsonArray>

namespace pacn {

namespace {

QString portStateName(PortState s) {
    switch (s) {
        case PortState::Open:     return QStringLiteral("open");
        case PortState::Filtered: return QStringLiteral("filtered");
        case PortState::Closed:   return QStringLiteral("closed");
    }
    return QStringLiteral("closed");
}

PortState portStateFromName(const QString& s) {
    if (s == QLatin1String("open")) return PortState::Open;
    if (s == QLatin1String("filtered")) return PortState::Filtered;
    return PortState::Closed;
}

DeviceStatus statusFromName(const QString& s) {
    if (s == QLatin1String("Online")) return DeviceStatus::Online;
    if (s == QLatin1String("Offline")) return DeviceStatus::Offline;
    if (s == QLatin1String("New")) return DeviceStatus::New;
    if (s == QLatin1String("Lost")) return DeviceStatus::Lost;
    return DeviceStatus::Unknown;
}

RiskLevel riskFromName(const QString& s) {
    if (s == QLatin1String("Low")) return RiskLevel::Low;
    if (s == QLatin1String("Medium")) return RiskLevel::Medium;
    if (s == QLatin1String("High")) return RiskLevel::High;
    if (s == QLatin1String("Critical")) return RiskLevel::Critical;
    return RiskLevel::Info;
}

DeviceType typeFromName(const QString& s) {
    static const QHash<QString, DeviceType> map = {
        {QStringLiteral("Gateway"), DeviceType::Gateway},
        {QStringLiteral("Router"), DeviceType::Router},
        {QStringLiteral("Switch"), DeviceType::Switch},
        {QStringLiteral("Access Point"), DeviceType::AccessPoint},
        {QStringLiteral("Server"), DeviceType::Server},
        {QStringLiteral("Computer"), DeviceType::Computer},
        {QStringLiteral("Laptop"), DeviceType::Laptop},
        {QStringLiteral("Phone"), DeviceType::Phone},
        {QStringLiteral("Tablet"), DeviceType::Tablet},
        {QStringLiteral("Printer"), DeviceType::Printer},
        {QStringLiteral("NAS"), DeviceType::Nas},
        {QStringLiteral("Camera"), DeviceType::Camera},
        {QStringLiteral("Media Player"), DeviceType::MediaPlayer},
        {QStringLiteral("IoT Device"), DeviceType::IoT},
        {QStringLiteral("Virtual Machine"), DeviceType::VirtualMachine},
    };
    return map.value(s, DeviceType::Unknown);
}

}  // namespace

QString Device::key() const {
    const QString normalizedMac = mac.trimmed().toUpper();
    if (!normalizedMac.isEmpty() && normalizedMac != QLatin1String("00:00:00:00:00:00"))
        return normalizedMac;
    return ip.toString();
}

QList<Port> Device::openPorts() const {
    QList<Port> out;
    for (const Port& p : ports)
        if (p.state == PortState::Open) out.push_back(p);
    return out;
}

QStringList Device::openPortNumbers() const {
    QStringList out;
    for (const Port& p : openPorts()) out << QString::number(p.number);
    return out;
}

QStringList Device::serviceNames() const {
    QStringList out;
    for (const Port& p : openPorts())
        if (!p.service.isEmpty() && !out.contains(p.service)) out << p.service;
    return out;
}

bool Device::isPrioritized() const {
    return favorite || riskLevel >= RiskLevel::High || status == DeviceStatus::New;
}

QJsonObject Device::toJson() const {
    QJsonObject o;
    o[QStringLiteral("ip")] = ip.toString();
    o[QStringLiteral("mac")] = mac;
    o[QStringLiteral("hostname")] = hostname;
    o[QStringLiteral("vendor")] = vendor;
    o[QStringLiteral("os")] = osGuess;
    o[QStringLiteral("type")] = toString(type);
    o[QStringLiteral("status")] = toString(status);
    o[QStringLiteral("riskScore")] = riskScore;
    o[QStringLiteral("riskLevel")] = toString(riskLevel);
    o[QStringLiteral("favorite")] = favorite;
    o[QStringLiteral("rttMs")] = rttMs;
    o[QStringLiteral("ttl")] = ttl;
    o[QStringLiteral("adapter")] = adapter;
    o[QStringLiteral("discoveryMethod")] = discoveryMethod;
    if (firstSeen.isValid())
        o[QStringLiteral("firstSeen")] = firstSeen.toString(Qt::ISODate);
    if (lastSeen.isValid())
        o[QStringLiteral("lastSeen")] = lastSeen.toString(Qt::ISODate);

    QJsonArray labelArr;
    for (const QString& l : labels) labelArr.append(l);
    o[QStringLiteral("labels")] = labelArr;

    QJsonArray portArr;
    for (const Port& p : ports) {
        QJsonObject po;
        po[QStringLiteral("port")] = p.number;
        po[QStringLiteral("protocol")] = p.protocol;
        po[QStringLiteral("state")] = portStateName(p.state);
        po[QStringLiteral("service")] = p.service;
        po[QStringLiteral("banner")] = p.banner;
        portArr.append(po);
    }
    o[QStringLiteral("ports")] = portArr;

    QJsonArray riskArr;
    for (const RiskFinding& f : risks) {
        QJsonObject fo;
        fo[QStringLiteral("level")] = toString(f.level);
        fo[QStringLiteral("title")] = f.title;
        fo[QStringLiteral("detail")] = f.detail;
        riskArr.append(fo);
    }
    o[QStringLiteral("risks")] = riskArr;
    return o;
}

Device Device::fromJson(const QJsonObject& o) {
    Device d;
    d.ip = QHostAddress(o.value(QStringLiteral("ip")).toString());
    d.mac = o.value(QStringLiteral("mac")).toString();
    d.hostname = o.value(QStringLiteral("hostname")).toString();
    d.vendor = o.value(QStringLiteral("vendor")).toString();
    d.osGuess = o.value(QStringLiteral("os")).toString();
    d.type = typeFromName(o.value(QStringLiteral("type")).toString());
    d.status = statusFromName(o.value(QStringLiteral("status")).toString());
    d.riskLevel = riskFromName(o.value(QStringLiteral("riskLevel")).toString());
    d.riskScore = o.value(QStringLiteral("riskScore")).toInt();
    d.favorite = o.value(QStringLiteral("favorite")).toBool();
    d.rttMs = o.value(QStringLiteral("rttMs")).toInt(-1);
    d.ttl = o.value(QStringLiteral("ttl")).toInt(-1);
    d.adapter = o.value(QStringLiteral("adapter")).toString();
    d.discoveryMethod = o.value(QStringLiteral("discoveryMethod")).toString();
    d.firstSeen = QDateTime::fromString(o.value(QStringLiteral("firstSeen")).toString(),
                                        Qt::ISODate);
    d.lastSeen = QDateTime::fromString(o.value(QStringLiteral("lastSeen")).toString(),
                                       Qt::ISODate);

    for (const QJsonValue& v : o.value(QStringLiteral("labels")).toArray())
        d.labels << v.toString();

    for (const QJsonValue& v : o.value(QStringLiteral("ports")).toArray()) {
        const QJsonObject po = v.toObject();
        Port p;
        p.number = static_cast<quint16>(po.value(QStringLiteral("port")).toInt());
        p.protocol = po.value(QStringLiteral("protocol")).toString(QStringLiteral("tcp"));
        p.state = portStateFromName(po.value(QStringLiteral("state")).toString());
        p.service = po.value(QStringLiteral("service")).toString();
        p.banner = po.value(QStringLiteral("banner")).toString();
        d.ports.push_back(p);
    }
    return d;
}

}  // namespace pacn
