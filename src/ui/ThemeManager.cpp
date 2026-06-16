#include "ui/ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QPalette>

namespace pacn {

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {}

bool ThemeManager::systemPrefersDark() {
    const QColor windowColor = QApplication::palette().color(QPalette::Window);
    return windowColor.lightness() < 128;
}

QString ThemeManager::loadStyleSheet() const {
    QFile f(QStringLiteral(":/app.qss"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
    return QString::fromUtf8(f.readAll());
}

void ThemeManager::apply(const QString& mode) {
    if (mode == QLatin1String("dark"))
        dark_ = true;
    else if (mode == QLatin1String("light"))
        dark_ = false;
    else
        dark_ = systemPrefersDark();

    palette_ = dark_ ? Palette::dark() : Palette::light();

    QString sheet = loadStyleSheet();
    for (auto it = palette_.tokens.cbegin(); it != palette_.tokens.cend(); ++it)
        sheet.replace(QStringLiteral("@%1@").arg(it.key()), it.value());

    if (auto* app = qobject_cast<QApplication*>(QCoreApplication::instance()))
        app->setStyleSheet(sheet);

    emit changed();
}

}  // namespace pacn
