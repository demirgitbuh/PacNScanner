#pragma once

#include <QFrame>

class QLabel;

namespace pacn {

// A dashboard summary tile: a big value over a small caption. Optional accent
// border highlights the primary metric.
class StatCard : public QFrame {
    Q_OBJECT
public:
    explicit StatCard(const QString& caption, QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setValue(int value);
    void setAccent(bool accent);

private:
    QLabel* value_ = nullptr;
    QLabel* caption_ = nullptr;
};

}  // namespace pacn
