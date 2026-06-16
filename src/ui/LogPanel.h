#pragma once

#include <QPlainTextEdit>

namespace pacn {

// Read-only log view mirroring the file logger. Subscribes to Logger::logged so
// UI and file stay in sync. Colour-codes by level and caps its backlog.
class LogPanel : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit LogPanel(QWidget* parent = nullptr);

public slots:
    void onLogged(int level, const QString& category, const QString& message,
                  const QString& timestamp);
};

}  // namespace pacn
