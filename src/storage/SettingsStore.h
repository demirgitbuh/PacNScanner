#pragma once

#include <QSettings>
#include <QString>

#include "core/Types.h"

namespace pacn {

// Thin, typed wrapper over QSettings. Centralises keys/defaults so the UI and
// CLI agree. Persisted under the platform-native settings backend.
class SettingsStore {
public:
    SettingsStore();

    // Scan defaults
    SpeedProfile speedProfile() const;
    void setSpeedProfile(SpeedProfile p);

    QString adapterName() const;          // empty = auto
    void setAdapterName(const QString& a);

    QString portProfile() const;          // e.g. "Top 100"
    void setPortProfile(const QString& p);

    QString customPorts() const;
    void setCustomPorts(const QString& p);

    bool method(ScanMethod m) const;
    void setMethod(ScanMethod m, bool on);

    bool resolveHostnames() const;
    void setResolveHostnames(bool on);

    bool detectOs() const;
    void setDetectOs(bool on);

    bool grabBanners() const;
    void setGrabBanners(bool on);

    bool useOnlineVendor() const;
    void setUseOnlineVendor(bool on);

    // Behaviour
    bool autoScanOnStartup() const;
    void setAutoScanOnStartup(bool on);

    bool periodicScanEnabled() const;
    void setPeriodicScanEnabled(bool on);

    int periodicIntervalMinutes() const;
    void setPeriodicIntervalMinutes(int m);

    bool keepHistory() const;
    void setKeepHistory(bool on);

    bool startOnBoot() const;             // default OFF
    void setStartOnBoot(bool on);

    bool trayEnabled() const;
    void setTrayEnabled(bool on);

    bool notificationsEnabled() const;
    void setNotificationsEnabled(bool on);

    // Appearance / i18n
    QString language() const;             // empty = system
    void setLanguage(const QString& code);

    QString theme() const;                // "system" | "light" | "dark"
    void setTheme(const QString& t);

    // Custom keyboard shortcuts (action id -> key sequence).
    QString shortcut(const QString& actionId, const QString& fallback) const;
    void setShortcut(const QString& actionId, const QString& seq);

    // Last targets typed by the user.
    QString lastTargets() const;
    void setLastTargets(const QString& t);

    QSettings& raw() { return settings_; }

private:
    QSettings settings_;
};

}  // namespace pacn
