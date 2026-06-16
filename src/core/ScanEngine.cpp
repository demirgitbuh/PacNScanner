#include "core/ScanEngine.h"

#include <QThread>

#include "core/ScanWorker.h"
#include "core/probe/DefaultProbeFactory.h"

namespace pacn {

ScanEngine::ScanEngine(QObject* parent) : QObject(parent) {
    qRegisterMetaType<pacn::Device>("pacn::Device");
    qRegisterMetaType<pacn::ScanSummary>("pacn::ScanSummary");
}

ScanEngine::~ScanEngine() {
    if (worker_) worker_->requestCancel();
    teardown();
}

void ScanEngine::setProbeFactory(std::shared_ptr<IProbeFactory> factory) {
    factory_ = std::move(factory);
}

void ScanEngine::setPlatform(std::shared_ptr<IPlatformServices> platform) {
    platform_ = std::move(platform);
}

void ScanEngine::setConfig(const ScanConfig& config) { config_ = config; }

void ScanEngine::start() {
    if (running_) return;
    if (!factory_) factory_ = std::make_shared<DefaultProbeFactory>();
    if (!platform_) platform_ = createPlatformServices();

    worker_ = new ScanWorker(config_, factory_, platform_);
    thread_ = new QThread(this);
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

    connect(worker_, &ScanWorker::finished, this,
            [this](const ScanSummary& s) {
                running_ = false;
                emit finished(s);
                if (thread_) thread_->quit();
            },
            Qt::QueuedConnection);

    // Canonical worker-thread teardown: delete both once the loop stops.
    connect(thread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(thread_, &QThread::finished, this, [this] {
        if (thread_) {
            thread_->deleteLater();
            thread_ = nullptr;
        }
        worker_ = nullptr;
    });

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
    // Synchronous stop used only from the destructor (event loop not pumping).
    if (thread_) {
        thread_->quit();
        thread_->wait();
        delete thread_;
        thread_ = nullptr;
    }
    delete worker_;
    worker_ = nullptr;
    running_ = false;
}

}  // namespace pacn
