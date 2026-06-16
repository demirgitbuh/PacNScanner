#include "storage/SettingsStore.h"

#include <pacn/version.h>

namespace pacn {

SettingsStore::SettingsStore()
    : settings_(QString::fromLatin1(kOrgName), QString::fromLatin1(kAppName)) {}

SpeedProfile SettingsStore::speedProfile() const {
    return speedProfileFromString(
        settings_.value(QStringLiteral("scan/speed"), QStringLiteral("Normal")).toString());
}
void SettingsStore::setSpeedProfile(SpeedProfile p) {
    settings_.setValue(QStringLiteral("scan/speed"), toString(p));
}

QString SettingsStore::adapterName() const {
    return settings_.value(QStringLiteral("scan/adapter")).toString();
}
void SettingsStore::setAdapterName(const QString& a) {
    settings_.setValue(QStringLiteral("scan/adapter"), a);
}

QString SettingsStore::portProfile() const {
    return settings_.value(QStringLiteral("scan/portProfile"), QStringLiteral("Top 100"))
        .toString();
}
void SettingsStore::setPortProfile(const QString& p) {
    settings_.setValue(QStringLiteral("scan/portProfile"), p);
}

QString SettingsStore::customPorts() const {
    return settings_.value(QStringLiteral("scan/customPorts")).toString();
}
void SettingsStore::setCustomPorts(const QString& p) {
    settings_.setValue(QStringLiteral("scan/customPorts"), p);
}

bool SettingsStore::method(ScanMethod m) const {
    QString key;
    bool def = true;
    switch (m) {
        case ScanMethod::Arp:        key = QStringLiteral("scan/method/arp"); break;
        case ScanMethod::Icmp:       key = QStringLiteral("scan/method/icmp"); break;
        case ScanMethod::TcpConnect: key = QStringLiteral("scan/method/tcp"); break;
        case ScanMethod::Raw:        key = QStringLiteral("scan/method/raw"); def = false; break;
        default: return false;
    }
    return settings_.value(key, def).toBool();
}
void SettingsStore::setMethod(ScanMethod m, bool on) {
    switch (m) {
        case ScanMethod::Arp:        settings_.setValue(QStringLiteral("scan/method/arp"), on); break;
        case ScanMethod::Icmp:       settings_.setValue(QStringLiteral("scan/method/icmp"), on); break;
        case ScanMethod::TcpConnect: settings_.setValue(QStringLiteral("scan/method/tcp"), on); break;
        case ScanMethod::Raw:        settings_.setValue(QStringLiteral("scan/method/raw"), on); break;
        default: break;
    }
}

bool SettingsStore::resolveHostnames() const {
    return settings_.value(QStringLiteral("scan/resolveHostnames"), true).toBool();
}
void SettingsStore::setResolveHostnames(bool on) {
    settings_.setValue(QStringLiteral("scan/resolveHostnames"), on);
}

bool SettingsStore::detectOs() const {
    return settings_.value(QStringLiteral("scan/detectOs"), true).toBool();
}
void SettingsStore::setDetectOs(bool on) {
    settings_.setValue(QStringLiteral("scan/detectOs"), on);
}

bool SettingsStore::grabBanners() const {
    return settings_.value(QStringLiteral("scan/grabBanners"), true).toBool();
}
void SettingsStore::setGrabBanners(bool on) {
    settings_.setValue(QStringLiteral("scan/grabBanners"), on);
}

bool SettingsStore::useOnlineVendor() const {
    return settings_.value(QStringLiteral("scan/useOnlineVendor"), false).toBool();
}
void SettingsStore::setUseOnlineVendor(bool on) {
    settings_.setValue(QStringLiteral("scan/useOnlineVendor"), on);
}

bool SettingsStore::autoScanOnStartup() const {
    return settings_.value(QStringLiteral("behavior/autoScanOnStartup"), true).toBool();
}
void SettingsStore::setAutoScanOnStartup(bool on) {
    settings_.setValue(QStringLiteral("behavior/autoScanOnStartup"), on);
}

bool SettingsStore::periodicScanEnabled() const {
    return settings_.value(QStringLiteral("behavior/periodicScan"), false).toBool();
}
void SettingsStore::setPeriodicScanEnabled(bool on) {
    settings_.setValue(QStringLiteral("behavior/periodicScan"), on);
}

int SettingsStore::periodicIntervalMinutes() const {
    return settings_.value(QStringLiteral("behavior/periodicInterval"), 15).toInt();
}
void SettingsStore::setPeriodicIntervalMinutes(int m) {
    settings_.setValue(QStringLiteral("behavior/periodicInterval"), m);
}

bool SettingsStore::keepHistory() const {
    return settings_.value(QStringLiteral("behavior/keepHistory"), true).toBool();
}
void SettingsStore::setKeepHistory(bool on) {
    settings_.setValue(QStringLiteral("behavior/keepHistory"), on);
}

bool SettingsStore::startOnBoot() const {
    return settings_.value(QStringLiteral("behavior/startOnBoot"), false).toBool();
}
void SettingsStore::setStartOnBoot(bool on) {
    settings_.setValue(QStringLiteral("behavior/startOnBoot"), on);
}

bool SettingsStore::trayEnabled() const {
    return settings_.value(QStringLiteral("behavior/tray"), true).toBool();
}
void SettingsStore::setTrayEnabled(bool on) {
    settings_.setValue(QStringLiteral("behavior/tray"), on);
}

bool SettingsStore::notificationsEnabled() const {
    return settings_.value(QStringLiteral("behavior/notifications"), true).toBool();
}
void SettingsStore::setNotificationsEnabled(bool on) {
    settings_.setValue(QStringLiteral("behavior/notifications"), on);
}

QString SettingsStore::language() const {
    return settings_.value(QStringLiteral("ui/language")).toString();
}
void SettingsStore::setLanguage(const QString& code) {
    settings_.setValue(QStringLiteral("ui/language"), code);
}

QString SettingsStore::theme() const {
    return settings_.value(QStringLiteral("ui/theme"), QStringLiteral("system")).toString();
}
void SettingsStore::setTheme(const QString& t) {
    settings_.setValue(QStringLiteral("ui/theme"), t);
}

QString SettingsStore::shortcut(const QString& actionId, const QString& fallback) const {
    return settings_.value(QStringLiteral("shortcuts/") + actionId, fallback).toString();
}
void SettingsStore::setShortcut(const QString& actionId, const QString& seq) {
    settings_.setValue(QStringLiteral("shortcuts/") + actionId, seq);
}

QString SettingsStore::lastTargets() const {
    return settings_.value(QStringLiteral("scan/lastTargets")).toString();
}
void SettingsStore::setLastTargets(const QString& t) {
    settings_.setValue(QStringLiteral("scan/lastTargets"), t);
}

}  // namespace pacn
