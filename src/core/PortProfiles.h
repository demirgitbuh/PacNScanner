#pragma once

#include <QList>
#include <QString>
#include <QStringList>

namespace pacn {

// Built-in port profiles plus parsing of user-supplied port specifications
// such as "22,80,443,8000-8100".
namespace portprofiles {

QStringList names();                              // for the UI combo box
QList<quint16> forName(const QString& name);      // resolves a named profile
QList<quint16> top100();
QList<quint16> top1000();
QList<quint16> wellKnown();                        // 1..1024
QList<quint16> full();                             // 1..65535

// Parses "22, 80, 443, 8000-8100"; invalid pieces are skipped. Result is sorted
// and de-duplicated.
QList<quint16> parseSpec(const QString& spec);

}  // namespace portprofiles
}  // namespace pacn
