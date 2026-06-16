#pragma once

#include <QDialog>

namespace pacn {

// About / ethical-use screen featuring the logo, version, license and the
// privacy + authorized-use notice required by the product spec.
class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

}  // namespace pacn
