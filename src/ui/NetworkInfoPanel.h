#pragma once

#include <QFrame>
#include <memory>

#include "core/PlatformServices.h"

class QLabel;

namespace pacn {

// Compact panel showing the active adapter, IP/CIDR, detected gateway, Wi-Fi
// SSID and privilege level. Refreshed on demand from IPlatformServices.
class NetworkInfoPanel : public QFrame {
    Q_OBJECT
public:
    explicit NetworkInfoPanel(QWidget* parent = nullptr);

    void setPlatform(std::shared_ptr<IPlatformServices> platform);
    void refresh();

    QHostAddress detectedGateway() const { return gateway_; }
    QString suggestedTarget() const;  // e.g. "192.168.1.0/24"

signals:
    void gatewayDetected(const QHostAddress& gateway);

private:
    QLabel* makeRow(const QString& caption);

    std::shared_ptr<IPlatformServices> platform_;
    QLabel* adapterValue_ = nullptr;
    QLabel* ipValue_ = nullptr;
    QLabel* gatewayValue_ = nullptr;
    QLabel* wifiValue_ = nullptr;
    QLabel* privValue_ = nullptr;

    QHostAddress gateway_;
    QString suggestedTarget_;
};

}  // namespace pacn
