#pragma once

#include <QDialog>

#include "core/Device.h"

class QLineEdit;
class QListWidget;

namespace pacn {

// Modal device inspector: full properties, open ports/services, risk findings,
// labels and favourite toggle.
class DeviceDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit DeviceDetailDialog(const Device& device, QWidget* parent = nullptr);

signals:
    void favoriteChanged(const QString& ip, bool favorite);
    void labelAdded(const QString& ip, const QString& label);
    void labelRemoved(const QString& ip, const QString& label);

private:
    void buildUi();
    void addLabelFromInput();

    Device device_;
    QListWidget* labelList_ = nullptr;
    QLineEdit* labelInput_ = nullptr;
};

}  // namespace pacn
