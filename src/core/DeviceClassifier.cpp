#include "core/DeviceClassifier.h"

namespace pacn {

DeviceType DeviceClassifier::classify(const Device& device, bool isGateway) const {
    if (isGateway) return DeviceType::Gateway;

    const QString vendor = device.vendor.toLower();
    const QStringList svc = device.serviceNames();
    const auto hasPort = [&](quint16 n) {
        for (const Port& p : device.openPorts())
            if (p.number == n) return true;
        return false;
    };

    // Vendor-driven guesses (strong signal).
    if (vendor.contains(QLatin1String("vmware")) ||
        vendor.contains(QLatin1String("virtualbox")) ||
        vendor.contains(QLatin1String("qemu")) || vendor.contains(QLatin1String("xen")) ||
        vendor.contains(QLatin1String("parallels")) ||
        vendor.contains(QLatin1String("hyper-v")))
        return DeviceType::VirtualMachine;
    if (vendor.contains(QLatin1String("raspberry")))
        return DeviceType::IoT;
    if (vendor.contains(QLatin1String("espressif")))
        return DeviceType::IoT;
    if (vendor.contains(QLatin1String("hewlett")) || vendor.contains(QLatin1String("epson")) ||
        vendor.contains(QLatin1String("canon")) || vendor.contains(QLatin1String("brother")))
        if (hasPort(9100) || hasPort(631) || hasPort(515)) return DeviceType::Printer;
    if (vendor.contains(QLatin1String("ubiquiti")) ||
        vendor.contains(QLatin1String("mikrotik")) ||
        vendor.contains(QLatin1String("netgear")) ||
        vendor.contains(QLatin1String("tp-link")) || vendor.contains(QLatin1String("d-link")) ||
        vendor.contains(QLatin1String("asus")))
        return DeviceType::Router;
    if (vendor.contains(QLatin1String("apple")))
        return hasPort(62078) ? DeviceType::Phone : DeviceType::Computer;
    if (vendor.contains(QLatin1String("samsung")) || vendor.contains(QLatin1String("huawei")) ||
        vendor.contains(QLatin1String("xiaomi")))
        return DeviceType::Phone;
    if (vendor.contains(QLatin1String("roku")) || vendor.contains(QLatin1String("amazon")) ||
        vendor.contains(QLatin1String("sony")))
        return DeviceType::MediaPlayer;

    // Port/service-driven guesses.
    if (hasPort(9100) || hasPort(631) || svc.contains(QStringLiteral("ipp")) ||
        svc.contains(QStringLiteral("jetdirect")))
        return DeviceType::Printer;
    if (hasPort(554) || hasPort(8554) || svc.contains(QStringLiteral("rtsp")))
        return DeviceType::Camera;
    if (hasPort(445) || hasPort(2049) || hasPort(548) || svc.contains(QStringLiteral("afp")))
        if (device.openPorts().size() <= 6) return DeviceType::Nas;
    if (hasPort(32400) || svc.contains(QStringLiteral("plex")) ||
        svc.contains(QStringLiteral("upnp")))
        return DeviceType::MediaPlayer;
    if (svc.contains(QStringLiteral("ms-wbt-rdp")) ||
        svc.contains(QStringLiteral("microsoft-ds")))
        return DeviceType::Computer;

    const int openCount = device.openPorts().size();
    if (openCount >= 6) return DeviceType::Server;
    if (openCount >= 1) return DeviceType::Computer;
    return DeviceType::Unknown;
}

}  // namespace pacn
