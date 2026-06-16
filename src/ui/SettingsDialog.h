#pragma once

#include <QDialog>
#include <QHash>

#include "storage/SettingsStore.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
class QKeySequenceEdit;

namespace pacn {

// Tabbed preferences backed by SettingsStore. Emits settingsApplied() so the
// main window can re-apply theme/language and reschedule periodic scans.
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(SettingsStore* settings, QWidget* parent = nullptr);

signals:
    void settingsApplied();

private:
    QWidget* buildGeneralTab();
    QWidget* buildScanTab();
    QWidget* buildPortsTab();
    QWidget* buildAppearanceTab();
    QWidget* buildShortcutsTab();
    QWidget* buildPrivacyTab();

    void load();
    void save();

    SettingsStore* settings_;

    QCheckBox* autoScan_ = nullptr;
    QCheckBox* periodic_ = nullptr;
    QSpinBox* periodicInterval_ = nullptr;
    QCheckBox* keepHistory_ = nullptr;
    QCheckBox* tray_ = nullptr;
    QCheckBox* notifications_ = nullptr;
    QCheckBox* startOnBoot_ = nullptr;

    QCheckBox* methodArp_ = nullptr;
    QCheckBox* methodIcmp_ = nullptr;
    QCheckBox* methodTcp_ = nullptr;
    QCheckBox* methodRaw_ = nullptr;
    QComboBox* speed_ = nullptr;
    QCheckBox* resolveHostnames_ = nullptr;
    QCheckBox* mdns_ = nullptr;
    QCheckBox* netbios_ = nullptr;
    QCheckBox* detectOs_ = nullptr;
    QCheckBox* grabBanners_ = nullptr;
    QCheckBox* onlineVendor_ = nullptr;

    QComboBox* portProfile_ = nullptr;
    QLineEdit* customPorts_ = nullptr;

    QComboBox* theme_ = nullptr;
    QComboBox* language_ = nullptr;

    QHash<QString, QKeySequenceEdit*> shortcutEdits_;
};

}  // namespace pacn
