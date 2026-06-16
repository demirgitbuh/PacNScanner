#pragma once

#include <QObject>
#include <memory>

#include "core/Device.h"
#include "core/PlatformServices.h"
#include "core/ScanConfig.h"
#include "core/probe/Probe.h"

class QThread;

namespace pacn {

class ScanWorker;

// High-level summary emitted when a scan ends.
struct ScanSummary {
    quint64 scanned = 0;
    quint64 hostsUp = 0;
    quint64 risky = 0;  // devices with High/Critical risk
    qint64 elapsedMs = 0;
    bool canceled = false;
};

// UI-facing orchestrator. Owns a worker thread; never blocks the caller. The
// engine is reusable: configure → start → (pause/resume/cancel) → finished.
class ScanEngine : public QObject {
    Q_OBJECT
public:
    explicit ScanEngine(QObject* parent = nullptr);
    ~ScanEngine() override;

    // Defaults to real network probes and the OS platform services when unset.
    void setProbeFactory(std::shared_ptr<IProbeFactory> factory);
    void setPlatform(std::shared_ptr<IPlatformServices> platform);
    void setConfig(const ScanConfig& config);

    bool isRunning() const { return running_; }

public slots:
    void start();
    void pause();
    void resume();
    void cancel();

signals:
    void started();
    void progress(quint64 done, quint64 total);
    void phaseChanged(const QString& phase);
    void deviceDiscovered(const pacn::Device& device);
    void deviceUpdated(const pacn::Device& device);
    void logMessage(const QString& text);
    void finished(const pacn::ScanSummary& summary);
    void failed(const QString& error);

private:
    void teardown();

    QThread* thread_ = nullptr;
    ScanWorker* worker_ = nullptr;
    ScanConfig config_;
    std::shared_ptr<IProbeFactory> factory_;
    std::shared_ptr<IPlatformServices> platform_;
    bool running_ = false;
};

}  // namespace pacn

Q_DECLARE_METATYPE(pacn::ScanSummary)
