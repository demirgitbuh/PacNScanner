#pragma once

#include <QHash>
#include <QString>

namespace pacn {

// Theme palette tokens substituted into app.qss (e.g. @ACCENT@). The accent is
// fixed to the logo turquoise across light/dark; everything else adapts.
struct Palette {
    QHash<QString, QString> tokens;

    static Palette light() {
        Palette p;
        p.tokens = {
            {QStringLiteral("ACCENT"), QStringLiteral("#2BBFD6")},
            {QStringLiteral("ACCENT_HOVER"), QStringLiteral("#14A6BE")},
            {QStringLiteral("ACCENT_DARK"), QStringLiteral("#0C7C8E")},
            {QStringLiteral("BG"), QStringLiteral("#f4f7fa")},
            {QStringLiteral("FG"), QStringLiteral("#1f2933")},
            {QStringLiteral("CARD"), QStringLiteral("#ffffff")},
            {QStringLiteral("BORDER"), QStringLiteral("#e1e8ef")},
            {QStringLiteral("ALT_ROW"), QStringLiteral("#f7fafc")},
            {QStringLiteral("MUTED"), QStringLiteral("#aab4c0")},
            {QStringLiteral("MUTED_FG"), QStringLiteral("#7b8794")},
            {QStringLiteral("CONSOLE_BG"), QStringLiteral("#0f1720")},
            {QStringLiteral("CONSOLE_FG"), QStringLiteral("#c6e9f0")},
        };
        return p;
    }

    static Palette dark() {
        Palette p;
        p.tokens = {
            {QStringLiteral("ACCENT"), QStringLiteral("#2BBFD6")},
            {QStringLiteral("ACCENT_HOVER"), QStringLiteral("#46D2E6")},
            {QStringLiteral("ACCENT_DARK"), QStringLiteral("#0C7C8E")},
            {QStringLiteral("BG"), QStringLiteral("#11161c")},
            {QStringLiteral("FG"), QStringLiteral("#e6edf3")},
            {QStringLiteral("CARD"), QStringLiteral("#1a212b")},
            {QStringLiteral("BORDER"), QStringLiteral("#2a333f")},
            {QStringLiteral("ALT_ROW"), QStringLiteral("#161d25")},
            {QStringLiteral("MUTED"), QStringLiteral("#3a4654")},
            {QStringLiteral("MUTED_FG"), QStringLiteral("#8b97a6")},
            {QStringLiteral("CONSOLE_BG"), QStringLiteral("#0b1118")},
            {QStringLiteral("CONSOLE_FG"), QStringLiteral("#c6e9f0")},
        };
        return p;
    }
};

}  // namespace pacn
