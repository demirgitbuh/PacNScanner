#pragma once

#include <QFile>
#include <QMutex>
#include <QObject>
#include <QString>
#include <memory>

namespace pacn {

enum class LogLevel { Debug, Info, Warning, Error };

// Application-wide logger. Writes to a rotating file and also emits `logged`
// so the UI log panel can mirror entries live. UI-free (QtCore only) so it
// lives in core and is reused by the CLI.
class Logger : public QObject {
    Q_OBJECT
public:
    static Logger& instance();

    // Opens (or creates) the log file under `dir`. Safe to call once at startup.
    void init(const QString& dir, qint64 maxBytes = 5 * 1024 * 1024);

    void log(LogLevel level, const QString& category, const QString& message);
    QString logFilePath() const { return filePath_; }

    void debug(const QString& c, const QString& m)   { log(LogLevel::Debug, c, m); }
    void info(const QString& c, const QString& m)    { log(LogLevel::Info, c, m); }
    void warning(const QString& c, const QString& m) { log(LogLevel::Warning, c, m); }
    void error(const QString& c, const QString& m)   { log(LogLevel::Error, c, m); }

    static QString levelName(LogLevel level);

signals:
    void logged(int level, const QString& category, const QString& message,
                const QString& timestamp);

private:
    Logger() = default;
    void rotateIfNeeded();

    mutable QMutex mutex_;
    std::unique_ptr<QFile> file_;
    QString filePath_;
    qint64 maxBytes_ = 5 * 1024 * 1024;
};

}  // namespace pacn
