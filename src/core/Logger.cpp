#include "core/Logger.h"

#include <QDateTime>
#include <QDir>
#include <QTextStream>

namespace pacn {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

QString Logger::levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return QStringLiteral("DEBUG");
        case LogLevel::Info:    return QStringLiteral("INFO");
        case LogLevel::Warning: return QStringLiteral("WARN");
        case LogLevel::Error:   return QStringLiteral("ERROR");
    }
    return QStringLiteral("INFO");
}

void Logger::init(const QString& dir, qint64 maxBytes) {
    QMutexLocker lock(&mutex_);
    maxBytes_ = maxBytes;
    QDir().mkpath(dir);
    filePath_ = QDir(dir).filePath(QStringLiteral("pacnscanner.log"));
    file_ = std::make_unique<QFile>(filePath_);
    file_->open(QIODevice::Append | QIODevice::Text);
}

void Logger::rotateIfNeeded() {
    if (!file_ || maxBytes_ <= 0) return;
    if (file_->size() < maxBytes_) return;
    file_->close();
    const QString backup = filePath_ + QStringLiteral(".1");
    QFile::remove(backup);
    QFile::rename(filePath_, backup);
    file_->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
}

void Logger::log(LogLevel level, const QString& category, const QString& message) {
    const QString ts = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    {
        QMutexLocker lock(&mutex_);
        if (file_ && file_->isOpen()) {
            rotateIfNeeded();
            QTextStream out(file_.get());
            out << ts << " [" << levelName(level) << "] " << category << ": "
                << message << '\n';
            out.flush();
        }
    }
    emit logged(static_cast<int>(level), category, message, ts);
}

}  // namespace pacn
