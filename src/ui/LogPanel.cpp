#include "ui/LogPanel.h"

#include "core/Logger.h"

namespace pacn {

LogPanel::LogPanel(QWidget* parent) : QPlainTextEdit(parent) {
    setObjectName(QStringLiteral("logPanel"));
    setReadOnly(true);
    setMaximumBlockCount(2000);
    setWordWrapMode(QTextOption::NoWrap);

    connect(&Logger::instance(), &Logger::logged, this, &LogPanel::onLogged,
            Qt::QueuedConnection);
}

void LogPanel::onLogged(int level, const QString& category, const QString& message,
                        const QString& timestamp) {
    QString color;
    switch (static_cast<LogLevel>(level)) {
        case LogLevel::Debug:   color = QStringLiteral("#7f8c8d"); break;
        case LogLevel::Info:    color = QStringLiteral("#c6e9f0"); break;
        case LogLevel::Warning: color = QStringLiteral("#f1c40f"); break;
        case LogLevel::Error:   color = QStringLiteral("#e74c3c"); break;
    }
    const QString time = timestamp.section(QLatin1Char('T'), 1).left(12);
    appendHtml(QStringLiteral("<span style='color:#5b6b7a'>%1</span> "
                              "<span style='color:%2'>[%3] %4</span>")
                   .arg(time, color, category, message.toHtmlEscaped()));
}

}  // namespace pacn
