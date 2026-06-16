#include "ui/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDockWidget>
#include <QFileDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QStandardPaths>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>

#include <pacn/version.h>

#include "core/Logger.h"
#include "core/PortProfiles.h"
#include "reporting/Exporter.h"
#include "ui/AboutDialog.h"
#include "ui/DeviceDetailDialog.h"
#include "ui/DeviceFilterProxyModel.h"
#include "ui/DeviceTableModel.h"
#include "ui/IconProvider.h"
#include "ui/LogPanel.h"
#include "ui/MiniCharts.h"
#include "ui/NetworkInfoPanel.h"
#include "ui/SettingsDialog.h"
#include "ui/StatCard.h"
#include "ui/ThemeManager.h"
#include "ui/TopologyView.h"
#include "ui/UpdateChecker.h"

namespace pacn {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("PacNScanner"));
    setWindowIcon(iconprovider::appIcon());
    resize(1180, 760);

    platform_ = createPlatformServices();
    engine_.setPlatform(platform_);

    const QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    db_.open(QStringLiteral("%1/inventory.sqlite").arg(dataDir));

    theme_ = new ThemeManager(this);
    updater_ = new UpdateChecker(this);

    buildUi();
    buildMenusAndTray();
    applyTheme();
    applyShortcuts();

    // Wire engine signals.
    connect(&engine_, &ScanEngine::started, this, &MainWindow::onScanStarted);
    connect(&engine_, &ScanEngine::progress, this, &MainWindow::onProgress);
    connect(&engine_, &ScanEngine::phaseChanged, this, &MainWindow::onPhaseChanged);
    connect(&engine_, &ScanEngine::deviceDiscovered, this, &MainWindow::onDeviceDiscovered);
    connect(&engine_, &ScanEngine::deviceUpdated, this, &MainWindow::onDeviceUpdated);
    connect(&engine_, &ScanEngine::finished, this, &MainWindow::onScanFinished);
    connect(&engine_, &ScanEngine::failed, this, [this](const QString& e) {
        statusBar()->showMessage(tr("Scan failed: %1").arg(e), 8000);
        Logger::instance().error(QStringLiteral("ui"), e);
    });

    // Load persisted inventory.
    const QList<Device> saved = db_.loadDevices();
    for (const Device& d : saved) {
        favByKey_[d.key()] = d.favorite;
        if (!d.labels.isEmpty()) labelsByKey_[d.key()] = d.labels;
        knownKeys_.insert(d.key());
    }
    model_->setDevices(saved);
    updateCards();
    charts_->setDevices(saved);

    refreshAdapters();
    netInfo_->setPlatform(platform_);
    connect(netInfo_, &NetworkInfoPanel::gatewayDetected, this,
            [this](const QHostAddress& gw) { gatewayIp_ = gw.toString(); });
    netInfo_->refresh();

    targetEdit_->setText(settings_.lastTargets().isEmpty() ? netInfo_->suggestedTarget()
                                                           : settings_.lastTargets());

    periodicTimer_ = new QTimer(this);
    connect(periodicTimer_, &QTimer::timeout, this, &MainWindow::onStartScan);
    reschedulePeriodic();

    connect(updater_, &UpdateChecker::updateAvailable, this,
            [this](const QString& v, const QString& url) {
                statusBar()->showMessage(
                    tr("A new version (%1) is available — see %2").arg(v, url), 15000);
            });
    QTimer::singleShot(1500, updater_, &UpdateChecker::checkLatest);
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(16, 12, 16, 8);
    layout->setSpacing(12);
    layout->addWidget(buildControlBar());
    layout->addWidget(buildDashboard(), 1);
    setCentralWidget(central);

    // Log dock
    auto* dock = new QDockWidget(tr("Log"), this);
    dock->setObjectName(QStringLiteral("logDock"));
    logPanel_ = new LogPanel(dock);
    dock->setWidget(logPanel_);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->hide();
    actions_[QStringLiteral("view.log")] = dock->toggleViewAction();

    statusBar()->showMessage(tr("Ready"));
}

QWidget* MainWindow::buildControlBar() {
    auto* frame = new QFrame;
    frame->setObjectName(QStringLiteral("statCard"));
    auto* outer = new QVBoxLayout(frame);
    outer->setContentsMargins(14, 12, 14, 12);

    auto* row = new QHBoxLayout;
    row->addWidget(new QLabel(tr("Targets")));
    targetEdit_ = new QLineEdit;
    targetEdit_->setPlaceholderText(
        QStringLiteral("192.168.1.0/24, 10.0.0.1-10.0.0.50, fe80::/64"));
    targetEdit_->setClearButtonEnabled(true);
    row->addWidget(targetEdit_, 1);

    adapterCombo_ = new QComboBox;
    adapterCombo_->setMinimumWidth(160);
    speedCombo_ = new QComboBox;
    speedCombo_->addItems({tr("Slow"), tr("Normal"), tr("Fast")});
    speedCombo_->setCurrentText(toString(settings_.speedProfile()));
    portCombo_ = new QComboBox;
    portCombo_->addItems(portprofiles::names());
    portCombo_->setCurrentText(settings_.portProfile());

    row->addWidget(new QLabel(tr("Adapter")));
    row->addWidget(adapterCombo_);
    row->addWidget(new QLabel(tr("Speed")));
    row->addWidget(speedCombo_);
    row->addWidget(new QLabel(tr("Ports")));
    row->addWidget(portCombo_);

    startBtn_ = new QPushButton(tr("Start Scan"));
    startBtn_->setObjectName(QStringLiteral("primaryAction"));
    startBtn_->setCursor(Qt::PointingHandCursor);
    pauseBtn_ = new QPushButton(tr("Pause"));
    stopBtn_ = new QPushButton(tr("Stop"));
    pauseBtn_->setEnabled(false);
    stopBtn_->setEnabled(false);
    row->addWidget(startBtn_);
    row->addWidget(pauseBtn_);
    row->addWidget(stopBtn_);
    outer->addLayout(row);

    auto* row2 = new QHBoxLayout;
    progress_ = new QProgressBar;
    progress_->setRange(0, 100);
    progress_->setValue(0);
    progress_->setTextVisible(false);
    phaseLabel_ = new QLabel(tr("Idle"));
    estimateLabel_ = new QLabel;
    estimateLabel_->setStyleSheet(QStringLiteral("color:#7b8794"));
    row2->addWidget(phaseLabel_);
    row2->addWidget(progress_, 1);
    row2->addWidget(estimateLabel_);
    outer->addLayout(row2);

    connect(startBtn_, &QPushButton::clicked, this, &MainWindow::onStartScan);
    connect(pauseBtn_, &QPushButton::clicked, this, &MainWindow::onPauseResume);
    connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::onStopScan);
    connect(targetEdit_, &QLineEdit::textChanged, this, [this] {
        const ScanConfig cfg = buildConfig();
        const auto est = cfg.estimate();
        if (est.hosts == 0) {
            estimateLabel_->clear();
            return;
        }
        const QString hosts = est.hosts == IpRange::kHugeCount
                                  ? tr("very large")
                                  : QString::number(est.hosts);
        estimateLabel_->setText(
            est.seconds < 0 ? tr("~%1 hosts · load %2").arg(hosts, est.load)
                            : tr("~%1 hosts · ~%2 s · load %3")
                                  .arg(hosts)
                                  .arg(QString::number(est.seconds, 'f', 0), est.load));
    });
    return frame;
}

QWidget* MainWindow::buildDashboard() {
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(12);

    // Cards
    auto* cards = new QHBoxLayout;
    cardTotal_ = new StatCard(tr("Devices"));
    cardTotal_->setAccent(true);
    cardOnline_ = new StatCard(tr("Online"));
    cardRisky_ = new StatCard(tr("At risk"));
    cardNew_ = new StatCard(tr("New"));
    for (StatCard* c : {cardTotal_, cardOnline_, cardRisky_, cardNew_}) cards->addWidget(c);
    v->addLayout(cards);

    // Charts + network info
    auto* mid = new QHBoxLayout;
    auto* chartFrame = new QFrame;
    chartFrame->setObjectName(QStringLiteral("statCard"));
    auto* chartLayout = new QVBoxLayout(chartFrame);
    charts_ = new MiniCharts;
    chartLayout->addWidget(charts_);
    netInfo_ = new NetworkInfoPanel;
    mid->addWidget(chartFrame, 2);
    mid->addWidget(netInfo_, 1);
    v->addLayout(mid);

    // Filter bar
    auto* filterBar = new QHBoxLayout;
    filterEdit_ = new QLineEdit;
    filterEdit_->setPlaceholderText(tr("Filter by IP, MAC, hostname, vendor, service…"));
    filterEdit_->setClearButtonEnabled(true);
    onlyRisky_ = new QCheckBox(tr("Risky only"));
    onlyFavorites_ = new QCheckBox(tr("Favorites"));
    prioritize_ = new QCheckBox(tr("Priority sort"));
    filterBar->addWidget(filterEdit_, 1);
    filterBar->addWidget(onlyRisky_);
    filterBar->addWidget(onlyFavorites_);
    filterBar->addWidget(prioritize_);
    v->addLayout(filterBar);

    // Table + topology in tabs
    model_ = new DeviceTableModel(this);
    proxy_ = new DeviceFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    table_ = new QTableView;
    table_->setModel(proxy_);
    table_->setSortingEnabled(true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setAlternatingRowColors(true);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table_->setColumnWidth(DeviceTableModel::ColFavorite, 34);
    table_->sortByColumn(DeviceTableModel::ColIp, Qt::AscendingOrder);

    topology_ = new TopologyView;

    auto* tabs = new QTabWidget;
    tabs->addTab(table_, tr("Devices"));
    tabs->addTab(topology_, tr("Topology"));
    v->addWidget(tabs, 1);

    connect(filterEdit_, &QLineEdit::textChanged, proxy_,
            &DeviceFilterProxyModel::setFilterText);
    connect(onlyRisky_, &QCheckBox::toggled, proxy_, &DeviceFilterProxyModel::setOnlyRisky);
    connect(onlyFavorites_, &QCheckBox::toggled, proxy_,
            &DeviceFilterProxyModel::setOnlyFavorites);
    connect(prioritize_, &QCheckBox::toggled, proxy_, &DeviceFilterProxyModel::setPrioritize);
    connect(table_, &QTableView::doubleClicked, this, &MainWindow::onTableActivated);
    connect(topology_, &TopologyView::deviceActivated, this, &MainWindow::openDeviceDetail);

    return w;
}

void MainWindow::buildMenusAndTray() {
    auto* fileMenu = menuBar()->addMenu(tr("&File"));
    auto* exportAct = fileMenu->addAction(tr("&Export report…"), this, &MainWindow::exportReport);
    actions_[QStringLiteral("export.report")] = exportAct;
    fileMenu->addAction(tr("&Import…"), this, &MainWindow::importData);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), this, [this] {
        forceQuit_ = true;
        close();
    });

    auto* scanMenu = menuBar()->addMenu(tr("&Scan"));
    actions_[QStringLiteral("scan.start")] =
        scanMenu->addAction(tr("&Start"), this, &MainWindow::onStartScan);
    actions_[QStringLiteral("scan.pause")] =
        scanMenu->addAction(tr("&Pause / Resume"), this, &MainWindow::onPauseResume);
    actions_[QStringLiteral("scan.stop")] =
        scanMenu->addAction(tr("S&top"), this, &MainWindow::onStopScan);

    auto* viewMenu = menuBar()->addMenu(tr("&View"));
    if (actions_.contains(QStringLiteral("view.log")))
        viewMenu->addAction(actions_[QStringLiteral("view.log")]);

    auto* toolsMenu = menuBar()->addMenu(tr("&Tools"));
    actions_[QStringLiteral("app.settings")] =
        toolsMenu->addAction(tr("&Settings…"), this, &MainWindow::openSettings);

    auto* helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("Check for &updates"), updater_, &UpdateChecker::checkLatest);
    helpMenu->addAction(tr("&About PacNScanner"), this, &MainWindow::openAbout);

    // Tray
    if (settings_.trayEnabled() && QSystemTrayIcon::isSystemTrayAvailable()) {
        tray_ = new QSystemTrayIcon(iconprovider::appIcon(), this);
        auto* menu = new QMenu(this);
        menu->addAction(tr("Show PacNScanner"), this, [this] {
            showNormal();
            raise();
            activateWindow();
        });
        menu->addAction(tr("Scan now"), this, &MainWindow::onStartScan);
        menu->addSeparator();
        menu->addAction(tr("Quit"), this, [this] {
            forceQuit_ = true;
            close();
        });
        tray_->setContextMenu(menu);
        tray_->setToolTip(QStringLiteral("PacNScanner"));
        tray_->show();
    }
}

void MainWindow::applyTheme() { theme_->apply(settings_.theme()); }

void MainWindow::applyShortcuts() {
    const QHash<QString, QString> defaults = {
        {QStringLiteral("scan.start"), QStringLiteral("Ctrl+R")},
        {QStringLiteral("scan.stop"), QStringLiteral("Ctrl+.")},
        {QStringLiteral("scan.pause"), QStringLiteral("Ctrl+P")},
        {QStringLiteral("export.report"), QStringLiteral("Ctrl+E")},
        {QStringLiteral("app.settings"), QStringLiteral("Ctrl+,")},
    };
    for (auto it = defaults.cbegin(); it != defaults.cend(); ++it) {
        if (auto* act = actions_.value(it.key()))
            act->setShortcut(QKeySequence(settings_.shortcut(it.key(), it.value())));
    }
}

void MainWindow::refreshAdapters() {
    adapterCombo_->clear();
    adapterCombo_->addItem(tr("Automatic"), QString());
    if (!platform_) return;
    for (const AdapterInfo& a : platform_->enumerateAdapters()) {
        if (a.isLoopback || a.primaryIpv4.isNull()) continue;
        QString label = a.description.isEmpty() ? a.name : a.description;
        if (a.isVpn) label += tr(" [VPN]");
        else if (a.isVirtual) label += tr(" [virtual]");
        else if (a.isWireless) label += tr(" [Wi-Fi]");
        adapterCombo_->addItem(label, label);
    }
}

ScanConfig MainWindow::buildConfig() {
    ScanConfig cfg;
    cfg.targets = IpRange::parseList(targetEdit_->text());
    if (cfg.targets.isEmpty() && !netInfo_->suggestedTarget().isEmpty()) {
        auto r = IpRange::parse(netInfo_->suggestedTarget());
        if (r) cfg.targets.push_back(r.value());
    }

    ScanMethods methods;
    if (settings_.method(ScanMethod::Arp)) methods |= ScanMethod::Arp;
    if (settings_.method(ScanMethod::Icmp)) methods |= ScanMethod::Icmp;
    if (settings_.method(ScanMethod::TcpConnect)) methods |= ScanMethod::TcpConnect;
    if (settings_.method(ScanMethod::Raw) && platform_ && platform_->rawSocketsAvailable())
        methods |= ScanMethod::Raw;
    if (!methods) methods |= ScanMethod::TcpConnect;
    cfg.methods = methods;

    cfg.speed = speedProfileFromString(speedCombo_->currentText());

    const QString prof = portCombo_->currentText();
    cfg.portProfileName = prof;
    cfg.ports = prof == QLatin1String("Custom")
                    ? portprofiles::parseSpec(settings_.customPorts())
                    : portprofiles::forName(prof);

    HostnameMethods hm;
    if (settings_.resolveHostnames()) hm |= HostnameMethod::Dns;
    if (settings_.raw().value(QStringLiteral("scan/mdns"), false).toBool())
        hm |= HostnameMethod::Mdns;
    if (settings_.raw().value(QStringLiteral("scan/netbios"), false).toBool())
        hm |= HostnameMethod::NetBios;
    cfg.hostnameMethods = hm;
    cfg.resolveHostnames = settings_.resolveHostnames();
    cfg.detectOs = settings_.detectOs();
    cfg.grabBanners = settings_.grabBanners();
    cfg.useOnlineVendor = settings_.useOnlineVendor();
    cfg.adapterName = adapterCombo_->currentData().toString();
    return cfg;
}

void MainWindow::onStartScan() {
    if (engine_.isRunning()) return;
    const ScanConfig cfg = buildConfig();
    if (cfg.targets.isEmpty()) {
        QMessageBox::warning(this, tr("No target"),
                             tr("Enter at least one target (CIDR, range or host)."));
        return;
    }
    const auto est = cfg.estimate();
    if (cfg.hasHugeTarget() || est.hosts > 65536) {
        const QString hosts = est.hosts == IpRange::kHugeCount ? tr("a very large number of")
                                                               : QString::number(est.hosts);
        const auto answer = QMessageBox::question(
            this, tr("Large scan"),
            tr("This will scan %1 hosts (estimated load: %2). Continue?")
                .arg(hosts, est.load));
        if (answer != QMessageBox::Yes) return;
    }

    settings_.setLastTargets(targetEdit_->text());
    settings_.setSpeedProfile(cfg.speed);
    settings_.setPortProfile(cfg.portProfileName);

    model_->clear();
    newThisScan_ = 0;
    chartThrottle_ = 0;
    engine_.setConfig(cfg);
    engine_.start();
}

void MainWindow::onPauseResume() {
    if (!engine_.isRunning()) return;
    if (pauseBtn_->text() == tr("Pause")) {
        engine_.pause();
        pauseBtn_->setText(tr("Resume"));
        phaseLabel_->setText(tr("Paused"));
    } else {
        engine_.resume();
        pauseBtn_->setText(tr("Pause"));
    }
}

void MainWindow::onStopScan() {
    if (engine_.isRunning()) engine_.cancel();
}

void MainWindow::onScanStarted() {
    startBtn_->setEnabled(false);
    pauseBtn_->setEnabled(true);
    pauseBtn_->setText(tr("Pause"));
    stopBtn_->setEnabled(true);
    progress_->setValue(0);
    Logger::instance().info(QStringLiteral("ui"), QStringLiteral("Scan started"));
}

void MainWindow::onProgress(quint64 done, quint64 total) {
    if (total > 0 && total != IpRange::kHugeCount)
        progress_->setValue(static_cast<int>(done * 100 / total));
    statusBar()->showMessage(tr("Scanned %1 / %2").arg(done).arg(total));
}

void MainWindow::onPhaseChanged(const QString& phase) { phaseLabel_->setText(phase); }

void MainWindow::onDeviceDiscovered(const Device& device) {
    Device d = device;
    const QString key = d.key();
    d.favorite = favByKey_.value(key, false);
    if (labelsByKey_.contains(key)) d.labels = labelsByKey_.value(key);

    if (firstScanDone_ && !knownKeys_.contains(key)) {
        ++newThisScan_;
        d.status = DeviceStatus::New;
        if (tray_ && settings_.notificationsEnabled())
            tray_->showMessage(tr("New device"),
                               QStringLiteral("%1  %2").arg(d.ipString(), d.vendor),
                               QSystemTrayIcon::Information, 4000);
    }
    knownKeys_.insert(key);

    model_->upsert(d);
    updateCards();
    if (++chartThrottle_ % 20 == 0) charts_->setDevices(model_->devices());
}

void MainWindow::onDeviceUpdated(const Device& device) {
    Device d = device;
    const QString key = d.key();
    d.favorite = favByKey_.value(key, d.favorite);
    if (labelsByKey_.contains(key)) d.labels = labelsByKey_.value(key);
    model_->upsert(d);
    updateCards();
}

void MainWindow::onScanFinished(const ScanSummary& summary) {
    lastSummary_ = summary;
    startBtn_->setEnabled(true);
    pauseBtn_->setEnabled(false);
    stopBtn_->setEnabled(false);
    progress_->setValue(100);
    phaseLabel_->setText(summary.canceled ? tr("Canceled") : tr("Done"));
    firstScanDone_ = true;

    charts_->setDevices(model_->devices());
    topology_->setDevices(model_->devices(), gatewayIp_);
    updateCards();
    persistDevices();
    reschedulePeriodic();

    const QString msg = tr("Found %1 host(s) of %2 scanned in %3 s — %4 at risk")
                            .arg(summary.hostsUp)
                            .arg(summary.scanned)
                            .arg(QString::number(summary.elapsedMs / 1000.0, 'f', 1))
                            .arg(summary.risky);
    statusBar()->showMessage(msg, 10000);
    Logger::instance().info(QStringLiteral("ui"), msg);
}

void MainWindow::updateCards() {
    const QList<Device> devices = model_->devices();
    int online = 0, risky = 0;
    for (const Device& d : devices) {
        if (d.status == DeviceStatus::Online || d.status == DeviceStatus::New) ++online;
        if (d.riskLevel >= RiskLevel::High) ++risky;
    }
    cardTotal_->setValue(devices.size());
    cardOnline_->setValue(online);
    cardRisky_->setValue(risky);
    cardNew_->setValue(newThisScan_);
    cardRisky_->setAccent(risky > 0);
}

void MainWindow::onTableActivated(const QModelIndex& index) {
    const QModelIndex src = proxy_->mapToSource(index);
    openDeviceDetail(model_->deviceAt(src.row()).ipString());
}

void MainWindow::openDeviceDetail(const QString& ip) {
    const int row = model_->rowForIp(ip);
    if (row < 0) return;
    const Device d = model_->deviceAt(row);
    auto* dlg = new DeviceDetailDialog(d, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    connect(dlg, &DeviceDetailDialog::favoriteChanged, this,
            [this](const QString& ipStr, bool fav) {
                const int r = model_->rowForIp(ipStr);
                if (r < 0) return;
                Device dev = model_->deviceAt(r);
                favByKey_[dev.key()] = fav;
                model_->toggleFavorite(r);
                db_.setFavorite(dev.key(), fav);
            });
    connect(dlg, &DeviceDetailDialog::labelAdded, this,
            [this](const QString& ipStr, const QString& label) {
                const int r = model_->rowForIp(ipStr);
                if (r < 0) return;
                Device dev = model_->deviceAt(r);
                labelsByKey_[dev.key()].append(label);
                db_.addLabel(dev.key(), label);
            });
    connect(dlg, &DeviceDetailDialog::labelRemoved, this,
            [this](const QString& ipStr, const QString& label) {
                const int r = model_->rowForIp(ipStr);
                if (r < 0) return;
                Device dev = model_->deviceAt(r);
                labelsByKey_[dev.key()].removeAll(label);
                db_.removeLabel(dev.key(), label);
            });
    dlg->show();
}

void MainWindow::openSettings() {
    SettingsDialog dlg(&settings_, this);
    connect(&dlg, &SettingsDialog::settingsApplied, this, [this] {
        applyTheme();
        applyShortcuts();
        reschedulePeriodic();
        speedCombo_->setCurrentText(toString(settings_.speedProfile()));
        portCombo_->setCurrentText(settings_.portProfile());
    });
    dlg.exec();
}

void MainWindow::openAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}

void MainWindow::exportReport() {
    if (model_->devices().isEmpty()) {
        QMessageBox::information(this, tr("Nothing to export"), tr("Run a scan first."));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Export report"), QStringLiteral("pacnscanner-report.html"),
        tr("HTML report (*.html);;CSV (*.csv);;JSON (*.json)"));
    if (path.isEmpty()) return;

    reporting::ReportContext ctx;
    ctx.title = QStringLiteral("PacNScanner Report");
    ctx.network = targetEdit_->text();
    ctx.adapter = adapterCombo_->currentText();
    ctx.summary = lastSummary_;

    QString err;
    bool ok = false;
    if (path.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive))
        ok = reporting::exportCsv(model_->devices(), path, &err);
    else if (path.endsWith(QLatin1String(".json"), Qt::CaseInsensitive))
        ok = reporting::exportJson(model_->devices(), ctx, path, &err);
    else
        ok = reporting::exportHtml(model_->devices(), ctx, path, &err);

    if (ok)
        statusBar()->showMessage(tr("Report exported to %1").arg(path), 6000);
    else
        QMessageBox::warning(this, tr("Export failed"), err);
}

void MainWindow::importData() {
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Import"), QString(), tr("JSON (*.json);;CSV (*.csv)"));
    if (path.isEmpty()) return;
    QString err;
    const QList<Device> devices = path.endsWith(QLatin1String(".csv"), Qt::CaseInsensitive)
                                      ? reporting::importCsv(path, &err)
                                      : reporting::importJson(path, &err);
    if (devices.isEmpty()) {
        QMessageBox::warning(this, tr("Import failed"),
                             err.isEmpty() ? tr("No devices found in file.") : err);
        return;
    }
    for (const Device& d : devices) {
        model_->upsert(d);
        knownKeys_.insert(d.key());
    }
    updateCards();
    charts_->setDevices(model_->devices());
    persistDevices();
    statusBar()->showMessage(tr("Imported %1 device(s)").arg(devices.size()), 6000);
}

void MainWindow::persistDevices() {
    if (!db_.isOpen()) return;
    const qint64 scanId = db_.insertScan(QDateTime::currentDateTime(), targetEdit_->text(),
                                         lastSummary_);
    for (const Device& d : model_->devices()) {
        db_.upsertDevice(d);
        if (settings_.keepHistory() && scanId >= 0) db_.recordHistory(d, scanId);
    }
}

void MainWindow::reschedulePeriodic() {
    if (!periodicTimer_) return;
    if (settings_.periodicScanEnabled())
        periodicTimer_->start(qMax(1, settings_.periodicIntervalMinutes()) * 60 * 1000);
    else
        periodicTimer_->stop();
}

void MainWindow::startInitialScan() {
    if (settings_.autoScanOnStartup() && !targetEdit_->text().trimmed().isEmpty())
        QTimer::singleShot(400, this, &MainWindow::onStartScan);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (!forceQuit_ && tray_ && tray_->isVisible()) {
        hide();
        tray_->showMessage(QStringLiteral("PacNScanner"),
                           tr("Still running in the tray."),
                           QSystemTrayIcon::Information, 3000);
        event->ignore();
        return;
    }
    if (engine_.isRunning()) engine_.cancel();
    persistDevices();
    event->accept();
}

}  // namespace pacn
