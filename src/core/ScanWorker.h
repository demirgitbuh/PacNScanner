#pragma once

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QThreadPool>
#include <QWaitCondition>
#include <atomic>
#include <memory>

#include "core/Device.h"
#include "core/PlatformServices.h"
#include "core/ScanConfig.h"
#include "core/ScanEngine.h"
#include "core/probe/Probe.h"

namespace pacn {

// Runs the actual scan inside a dedicated thread. Streams targets so even very
// large ranges never materialise a giant list. Supports cooperative pause/
// resume/cancel.
class ScanWorker : public QObject {
    Q_OBJECT
public:
    ScanWorker(ScanConfig config, std::shared_ptr<IProbeFactory> factory,
               std::shared_ptr<IPlatformServices> platform);

    void requestPause();
    void requestResume();
    void requestCancel();

public slots:
    void run();

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
    friend class HostTask;

    // Called from pool threads when a host probe completes. Thread-safe.
    void reportHostFinished(const Device& device, bool alive);

    void waitWhilePaused();
    void enrichMacsFromArp();
    Device enrichHost(const QHostAddress& ip, const HostProbeResult& probe) const;

    ScanConfig config_;
    std::shared_ptr<IProbeFactory> factory_;
    std::shared_ptr<IPlatformServices> platform_;

    QThreadPool pool_;
    std::atomic<bool> cancel_{false};
    std::atomic<bool> paused_{false};
    QMutex pauseMutex_;
    QWaitCondition pauseCond_;

    // Read-only snapshots shared with host tasks.
    QHash<QString, QString> arpByIp_;  // ip string -> MAC
    QSet<QString> gatewayIps_;
    QString gatewayMac_;

    // Accumulated results — guarded by resultsMutex_ (written from pool threads).
    QMutex resultsMutex_;
    QHash<QString, Device> devices_;  // ip string -> device
    ScanSummary summary_;
    quint64 total_ = 0;
    quint64 done_ = 0;
};

}  // namespace pacn
