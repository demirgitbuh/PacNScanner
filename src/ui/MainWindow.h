#pragma once

#include <QHash>
#include <QMainWindow>
#include <QSet>
#include <memory>

#include "core/PlatformServices.h"
#include "core/ScanEngine.h"
#include "storage/Database.h"
#include "storage/SettingsStore.h"

class QAction;
class QComboBox;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSystemTrayIcon;
class QTableView;
class QTimer;
class QCheckBox;

namespace pacn {

class StatCard;
class MiniCharts;
class NetworkInfoPanel;
class DeviceTableModel;
class DeviceFilterProxyModel;
class TopologyView;
class LogPanel;
class ThemeManager;
class UpdateChecker;

// The application's dashboard window: target/adapter controls + Start Scan,
// summary cards, charts, active-network panel, device table, topology, and a
// log dock. Owns the ScanEngine and persists results via SQLite/QSettings.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void startInitialScan();  // called after construction if auto-scan enabled

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onStartScan();
    void onPauseResume();
    void onStopScan();

    void onScanStarted();
    void onProgress(quint64 done, quint64 total);
    void onPhaseChanged(const QString& phase);
    void onDeviceDiscovered(const pacn::Device& device);
    void onDeviceUpdated(const pacn::Device& device);
    void onScanFinished(const pacn::ScanSummary& summary);

    void onTableActivated(const QModelIndex& index);
    void openDeviceDetail(const QString& ip);
    void openSettings();
    void openAbout();
    void exportReport();
    void importData();

private:
    void buildUi();
    QWidget* buildControlBar();
    QWidget* buildDashboard();
    void buildMenusAndTray();
    void applyTheme();
    void applyShortcuts();
    void refreshAdapters();
    void updateCards();
    void reschedulePeriodic();
    void persistDevices();
    ScanConfig buildConfig();

    SettingsStore settings_;
    Database db_;
    std::shared_ptr<IPlatformServices> platform_;
    ScanEngine engine_;
    ThemeManager* theme_ = nullptr;
    UpdateChecker* updater_ = nullptr;

    // Controls
    QLineEdit* targetEdit_ = nullptr;
    QComboBox* adapterCombo_ = nullptr;
    QComboBox* speedCombo_ = nullptr;
    QComboBox* portCombo_ = nullptr;
    QPushButton* startBtn_ = nullptr;
    QPushButton* pauseBtn_ = nullptr;
    QPushButton* stopBtn_ = nullptr;
    QProgressBar* progress_ = nullptr;
    QLabel* phaseLabel_ = nullptr;
    QLabel* estimateLabel_ = nullptr;

    // Dashboard
    StatCard* cardTotal_ = nullptr;
    StatCard* cardOnline_ = nullptr;
    StatCard* cardRisky_ = nullptr;
    StatCard* cardNew_ = nullptr;
    MiniCharts* charts_ = nullptr;
    NetworkInfoPanel* netInfo_ = nullptr;

    DeviceTableModel* model_ = nullptr;
    DeviceFilterProxyModel* proxy_ = nullptr;
    QTableView* table_ = nullptr;
    QLineEdit* filterEdit_ = nullptr;
    QCheckBox* onlyRisky_ = nullptr;
    QCheckBox* onlyFavorites_ = nullptr;
    QCheckBox* prioritize_ = nullptr;

    TopologyView* topology_ = nullptr;
    LogPanel* logPanel_ = nullptr;
    QSystemTrayIcon* tray_ = nullptr;
    QTimer* periodicTimer_ = nullptr;

    QHash<QString, QAction*> actions_;
    QSet<QString> knownKeys_;
    bool firstScanDone_ = false;
    bool forceQuit_ = false;
    int newThisScan_ = 0;
    int chartThrottle_ = 0;
    QString gatewayIp_;
    ScanSummary lastSummary_;
    QHash<QString, bool> favByKey_;
    QHash<QString, QStringList> labelsByKey_;
};

}  // namespace pacn
