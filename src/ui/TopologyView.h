#pragma once

#include <QGraphicsView>
#include <QList>

#include "core/Device.h"

class QGraphicsScene;

namespace pacn {

// Radial network map: the gateway sits at the centre with discovered devices
// arranged around it. Nodes are colour-coded by status and clickable.
class TopologyView : public QGraphicsView {
    Q_OBJECT
public:
    explicit TopologyView(QWidget* parent = nullptr);

    void setDevices(const QList<Device>& devices, const QString& gatewayIp);

signals:
    void deviceActivated(const QString& ip);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void rebuild();

    QGraphicsScene* scene_ = nullptr;
    QList<Device> devices_;
    QString gatewayIp_;
};

}  // namespace pacn
