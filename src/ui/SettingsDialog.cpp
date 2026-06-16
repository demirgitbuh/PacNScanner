#include "ui/SettingsDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

#include "core/PortProfiles.h"

namespace pacn {

namespace {
struct ShortcutDef {
    const char* id;
    const char* label;
    const char* fallback;
};
const ShortcutDef kShortcuts[] = {
    {"scan.start", "Start scan", "Ctrl+R"},
    {"scan.stop", "Stop scan", "Ctrl+."},
    {"scan.pause", "Pause / resume", "Ctrl+P"},
    {"export.report", "Export report", "Ctrl+E"},
    {"app.settings", "Open settings", "Ctrl+,"},
};
}  // namespace

SettingsDialog::SettingsDialog(SettingsStore* settings, QWidget* parent)
    : QDialog(parent), settings_(settings) {
    setWindowTitle(tr("Settings"));
    setModal(true);
    resize(560, 560);

    auto* root = new QVBoxLayout(this);
    auto* tabs = new QTabWidget(this);
    tabs->addTab(buildGeneralTab(), tr("General"));
    tabs->addTab(buildScanTab(), tr("Scan"));
    tabs->addTab(buildPortsTab(), tr("Ports"));
    tabs->addTab(buildAppearanceTab(), tr("Appearance"));
    tabs->addTab(buildShortcutsTab(), tr("Shortcuts"));
    tabs->addTab(buildPrivacyTab(), tr("Privacy"));
    root->addWidget(tabs);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        save();
        emit settingsApplied();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(buttons);

    load();
}

QWidget* SettingsDialog::buildGeneralTab() {
    auto* w = new QWidget;
    auto* form = new QFormLayout(w);
    autoScan_ = new QCheckBox(tr("Run an automatic scan on startup"));
    periodic_ = new QCheckBox(tr("Enable periodic re-scan"));
    periodicInterval_ = new QSpinBox;
    periodicInterval_->setRange(1, 1440);
    periodicInterval_->setSuffix(tr(" min"));
    keepHistory_ = new QCheckBox(tr("Keep per-device history"));
    tray_ = new QCheckBox(tr("Show tray icon"));
    notifications_ = new QCheckBox(tr("Notify on new / lost devices"));
    startOnBoot_ = new QCheckBox(tr("Start on system login"));

    form->addRow(autoScan_);
    form->addRow(periodic_);
    form->addRow(tr("Interval"), periodicInterval_);
    form->addRow(keepHistory_);
    form->addRow(tray_);
    form->addRow(notifications_);
    form->addRow(startOnBoot_);
    return w;
}

QWidget* SettingsDialog::buildScanTab() {
    auto* w = new QWidget;
    auto* form = new QFormLayout(w);
    methodArp_ = new QCheckBox(tr("ARP (MAC discovery)"));
    methodIcmp_ = new QCheckBox(tr("ICMP ping"));
    methodTcp_ = new QCheckBox(tr("TCP connect (no privileges)"));
    methodRaw_ = new QCheckBox(tr("Raw / Npcap (requires elevation)"));
    speed_ = new QComboBox;
    speed_->addItems({tr("Slow"), tr("Normal"), tr("Fast")});
    resolveHostnames_ = new QCheckBox(tr("Resolve hostnames (reverse DNS)"));
    mdns_ = new QCheckBox(tr("Also try mDNS"));
    netbios_ = new QCheckBox(tr("Also try NetBIOS"));
    detectOs_ = new QCheckBox(tr("Estimate OS (TTL / banners)"));
    grabBanners_ = new QCheckBox(tr("Grab service banners"));
    onlineVendor_ = new QCheckBox(tr("Use online vendor lookup (off by default)"));

    form->addRow(tr("Methods"), methodArp_);
    form->addRow(QString(), methodIcmp_);
    form->addRow(QString(), methodTcp_);
    form->addRow(QString(), methodRaw_);
    form->addRow(tr("Speed profile"), speed_);
    form->addRow(resolveHostnames_);
    form->addRow(QString(), mdns_);
    form->addRow(QString(), netbios_);
    form->addRow(detectOs_);
    form->addRow(grabBanners_);
    form->addRow(onlineVendor_);
    return w;
}

QWidget* SettingsDialog::buildPortsTab() {
    auto* w = new QWidget;
    auto* form = new QFormLayout(w);
    portProfile_ = new QComboBox;
    portProfile_->addItems(portprofiles::names());
    customPorts_ = new QLineEdit;
    customPorts_->setPlaceholderText(QStringLiteral("22, 80, 443, 8000-8100"));
    form->addRow(tr("Default profile"), portProfile_);
    form->addRow(tr("Custom ports"), customPorts_);
    auto* hint = new QLabel(
        tr("The custom list applies when the profile is set to \"Custom\". Full (1-65535) "
           "scans every port and is slower."));
    hint->setWordWrap(true);
    hint->setStyleSheet(QStringLiteral("color:#7b8794"));
    form->addRow(hint);
    return w;
}

QWidget* SettingsDialog::buildAppearanceTab() {
    auto* w = new QWidget;
    auto* form = new QFormLayout(w);
    theme_ = new QComboBox;
    theme_->addItem(tr("Follow system"), QStringLiteral("system"));
    theme_->addItem(tr("Light"), QStringLiteral("light"));
    theme_->addItem(tr("Dark"), QStringLiteral("dark"));
    language_ = new QComboBox;
    language_->addItem(tr("System language"), QString());
    language_->addItem(QStringLiteral("English"), QStringLiteral("en"));
    language_->addItem(QStringLiteral("Türkçe"), QStringLiteral("tr"));
    form->addRow(tr("Theme"), theme_);
    form->addRow(tr("Language"), language_);
    return w;
}

QWidget* SettingsDialog::buildShortcutsTab() {
    auto* w = new QWidget;
    auto* form = new QFormLayout(w);
    for (const ShortcutDef& s : kShortcuts) {
        auto* edit = new QKeySequenceEdit;
        shortcutEdits_.insert(QString::fromLatin1(s.id), edit);
        form->addRow(tr(s.label), edit);
    }
    return w;
}

QWidget* SettingsDialog::buildPrivacyTab() {
    auto* w = new QWidget;
    auto* layout = new QVBoxLayout(w);
    auto* text = new QLabel(
        tr("<h3>Privacy & ethical use</h3>"
           "<p>PacNScanner collects <b>no telemetry</b> and sends nothing to the cloud. "
           "All scan results stay on your device.</p>"
           "<p>Online vendor lookup (optional, off by default) is the only feature that "
           "may contact a third party, and only for MAC prefixes you choose to resolve.</p>"
           "<p><b>Only scan networks you own or are explicitly authorized to assess.</b></p>"));
    text->setWordWrap(true);
    layout->addWidget(text);
    layout->addStretch(1);
    return w;
}

void SettingsDialog::load() {
    autoScan_->setChecked(settings_->autoScanOnStartup());
    periodic_->setChecked(settings_->periodicScanEnabled());
    periodicInterval_->setValue(settings_->periodicIntervalMinutes());
    keepHistory_->setChecked(settings_->keepHistory());
    tray_->setChecked(settings_->trayEnabled());
    notifications_->setChecked(settings_->notificationsEnabled());
    startOnBoot_->setChecked(settings_->startOnBoot());

    methodArp_->setChecked(settings_->method(ScanMethod::Arp));
    methodIcmp_->setChecked(settings_->method(ScanMethod::Icmp));
    methodTcp_->setChecked(settings_->method(ScanMethod::TcpConnect));
    methodRaw_->setChecked(settings_->method(ScanMethod::Raw));
    speed_->setCurrentText(toString(settings_->speedProfile()));
    resolveHostnames_->setChecked(settings_->resolveHostnames());
    mdns_->setChecked(settings_->raw().value(QStringLiteral("scan/mdns"), false).toBool());
    netbios_->setChecked(
        settings_->raw().value(QStringLiteral("scan/netbios"), false).toBool());
    detectOs_->setChecked(settings_->detectOs());
    grabBanners_->setChecked(settings_->grabBanners());
    onlineVendor_->setChecked(settings_->useOnlineVendor());

    portProfile_->setCurrentText(settings_->portProfile());
    customPorts_->setText(settings_->customPorts());

    const int themeIdx = theme_->findData(settings_->theme());
    theme_->setCurrentIndex(themeIdx >= 0 ? themeIdx : 0);
    const int langIdx = language_->findData(settings_->language());
    language_->setCurrentIndex(langIdx >= 0 ? langIdx : 0);

    for (const ShortcutDef& s : kShortcuts) {
        const QString id = QString::fromLatin1(s.id);
        shortcutEdits_[id]->setKeySequence(
            QKeySequence(settings_->shortcut(id, QString::fromLatin1(s.fallback))));
    }
}

void SettingsDialog::save() {
    settings_->setAutoScanOnStartup(autoScan_->isChecked());
    settings_->setPeriodicScanEnabled(periodic_->isChecked());
    settings_->setPeriodicIntervalMinutes(periodicInterval_->value());
    settings_->setKeepHistory(keepHistory_->isChecked());
    settings_->setTrayEnabled(tray_->isChecked());
    settings_->setNotificationsEnabled(notifications_->isChecked());
    settings_->setStartOnBoot(startOnBoot_->isChecked());

    settings_->setMethod(ScanMethod::Arp, methodArp_->isChecked());
    settings_->setMethod(ScanMethod::Icmp, methodIcmp_->isChecked());
    settings_->setMethod(ScanMethod::TcpConnect, methodTcp_->isChecked());
    settings_->setMethod(ScanMethod::Raw, methodRaw_->isChecked());
    settings_->setSpeedProfile(speedProfileFromString(speed_->currentText()));
    settings_->setResolveHostnames(resolveHostnames_->isChecked());
    settings_->raw().setValue(QStringLiteral("scan/mdns"), mdns_->isChecked());
    settings_->raw().setValue(QStringLiteral("scan/netbios"), netbios_->isChecked());
    settings_->setDetectOs(detectOs_->isChecked());
    settings_->setGrabBanners(grabBanners_->isChecked());
    settings_->setUseOnlineVendor(onlineVendor_->isChecked());

    settings_->setPortProfile(portProfile_->currentText());
    settings_->setCustomPorts(customPorts_->text());

    settings_->setTheme(theme_->currentData().toString());
    settings_->setLanguage(language_->currentData().toString());

    for (const ShortcutDef& s : kShortcuts) {
        const QString id = QString::fromLatin1(s.id);
        settings_->setShortcut(id, shortcutEdits_[id]->keySequence().toString());
    }
}

}  // namespace pacn
