#include "ui/TopologyView.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QMouseEvent>
#include <QtMath>

#include "ui/IconProvider.h"

namespace pacn {

TopologyView::TopologyView(QWidget* parent) : QGraphicsView(parent) {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setMinimumHeight(320);
}

void TopologyView::setDevices(const QList<Device>& devices, const QString& gatewayIp) {
    devices_ = devices;
    gatewayIp_ = gatewayIp;
    rebuild();
}

void TopologyView::rebuild() {
    scene_->clear();
    if (devices_.isEmpty()) {
        auto* t = scene_->addSimpleText(tr("Run a scan to see the network map"));
        t->setBrush(QColor(0x95, 0xa5, 0xa6));
        return;
    }

    const qreal radius = qMax<qreal>(160, devices_.size() * 9);
    const QPointF center(0, 0);

    // Centre node = gateway (or a synthetic "Network" hub).
    Device gateway;
    QList<Device> leaves;
    bool haveGateway = false;
    for (const Device& d : devices_) {
        if (!haveGateway && (d.ipString() == gatewayIp_ || d.type == DeviceType::Gateway)) {
            gateway = d;
            haveGateway = true;
        } else {
            leaves.push_back(d);
        }
    }

    const auto addNode = [&](const Device& d, QPointF pos, qreal r, bool hub) {
        QColor color = iconprovider::statusColor(d.status);
        if (d.riskLevel >= RiskLevel::High) color = iconprovider::riskColor(d.riskLevel);
        auto* node = scene_->addEllipse(pos.x() - r, pos.y() - r, 2 * r, 2 * r,
                                        QPen(color.darker(120), 2), QBrush(color));
        node->setData(0, d.ipString());
        node->setToolTip(QStringLiteral("%1\n%2\n%3")
                             .arg(d.ipString(),
                                  d.hostname.isEmpty() ? toString(d.type) : d.hostname,
                                  d.vendor));
        const QString label = d.hostname.isEmpty() ? d.ipString() : d.hostname;
        auto* text = scene_->addSimpleText(hub ? QStringLiteral("◆ %1").arg(label) : label);
        text->setBrush(palette().color(QPalette::WindowText));
        text->setPos(pos.x() - text->boundingRect().width() / 2, pos.y() + r + 2);
    };

    if (haveGateway) addNode(gateway, center, 26, true);

    const int n = leaves.size();
    for (int i = 0; i < n; ++i) {
        const double angle = (2 * M_PI * i) / qMax(1, n);
        const QPointF pos(center.x() + radius * std::cos(angle),
                          center.y() + radius * std::sin(angle));
        if (haveGateway || i > 0)
            scene_->addLine(center.x(), center.y(), pos.x(), pos.y(),
                            QPen(QColor(0xc6, 0xcf, 0xd8), 1));
        addNode(leaves[i], pos, 16, false);
    }

    scene_->setSceneRect(scene_->itemsBoundingRect().adjusted(-40, -40, 40, 40));
}

void TopologyView::mouseDoubleClickEvent(QMouseEvent* event) {
    if (auto* item = itemAt(event->pos())) {
        const QString ip = item->data(0).toString();
        if (!ip.isEmpty()) {
            emit deviceActivated(ip);
            return;
        }
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void TopologyView::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (scene_ && !scene_->items().isEmpty())
        fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

}  // namespace pacn
