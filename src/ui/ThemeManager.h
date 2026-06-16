#pragma once

#include <QObject>
#include <QString>

#include "ui/Palette.h"

namespace pacn {

// Applies the bundled stylesheet with palette tokens resolved for the active
// theme ("system" follows the OS light/dark setting).
class ThemeManager : public QObject {
    Q_OBJECT
public:
    explicit ThemeManager(QObject* parent = nullptr);

    // mode: "system" | "light" | "dark"
    void apply(const QString& mode);
    bool isDark() const { return dark_; }
    const Palette& palette() const { return palette_; }

    static bool systemPrefersDark();

signals:
    void changed();

private:
    QString loadStyleSheet() const;

    Palette palette_;
    bool dark_ = false;
};

}  // namespace pacn
