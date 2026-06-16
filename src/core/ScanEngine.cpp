#include "core/ScanEngine.h"

#include <QThread>

#include "core/ScanWorker.h"
#include "core/probe/DefaultProbeFactory.h"

namespace pacn {

ScanEngine::ScanEngine(QObject* parent) : QObject(parent) {
    qRegisterMetaType<pacn::Device>("pacn::Device");
    qRegisterMetaType<pacn::ScanSummary>("pacn::ScanSummary");
}

ScanEngine::~ScanEngine() { teardown(); }

void ScanEngine::setProbeFactory(std::shared_ptr<IProbeFactory> factory) {
    factory_ = std::move(factory);
}

void ScanEngine::setPlatform(std::shared_ptr<IPlatformServices> platform) {
    platform_ = std::move(platform);
}

void ScanEngine::setConfig(const ScanConfig& config) { config_ = config; }

void ScanEngine::start() {
    if (running_) return;
    teardown();  // clean up any previous (finished) run before starting a new one
    if (!factory_) factory_ = std::make_shared<DefaultProbeFactory>();
    if (!platform_) platform_ = createPlatformServices();

    worker_ = new ScanWorker(config_, factory_, platform_);
    thread_ = new QThread;  // no parent: lifetime managed explicitly in teardown()
    worker_->moveToThread(thread_);

    connect(thread_, &QThread::started, worker_, &ScanWorker::run);

    connect(worker_, &ScanWorker::started, this, &ScanEngine::started,
            Qt::QueuedConnection);
    connect(worker_, &ScanWorker::progress, this, &ScanEngine::progress,
            Qt::QueuedConnection);
    connect(worker_, &ScanWorker::phaseChanged, this, &ScanEngine::phaseChanged,
            Qt::QueuedConnection);
    connect(worker_, &ScanWorker::deviceDiscovered, this, &ScanEngine::deviceDiscovered,
            Qt::QueuedConnection);
    connect(worker_, &ScanWorker::deviceUpdated, this, &ScanEngine::deviceUpdated,
            Qt::QueuedConnection);
    connect(worker_, &ScanWorker::logMessage, this, &ScanEngine::logMessage,
            Qt::QueuedConnection);
    connect(worker_, &ScanWorker::failed, this, &ScanEngine::failed, Qt::QueuedConnection);

    // When the worker finishes, stop the thread's event loop. The thread object
    // and worker are joined and deleted synchronously in teardown() (next start
    // or destruction), which avoids cross-thread / use-after-free races.
    connect(worker_, &ScanWorker::finished, this,
            [this](const ScanSummary& s) {
                running_ = false;
                emit finished(s);
                if (thread_) thread_->quit();
            },
            Qt::QueuedConnection);

    running_ = true;
    thread_->start();
}

void ScanEngine::pause() {
    if (worker_) worker_->requestPause();
}

void ScanEngine::resume() {
    if (worker_) worker_->requestResume();
}

void ScanEngine::cancel() {
    if (worker_) worker_->requestCancel();
}

void ScanEngine::teardown() {
    // Synchronous, deterministic stop. Safe to call when idle (no-op) or while a
    // scan is running (cancels, joins, then deletes).
    if (!thread_) {
        running_ = false;
        return;
    }
    if (worker_) worker_->requestCancel();
    thread_->quit();
    thread_->wait();
    delete worker_;  // thread joined; worker is inert and has no live timers
    worker_ = nullptr;
    delete thread_;
    thread_ = nullptr;
    running_ = false;
}

}  // namespace pacn
