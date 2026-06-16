#pragma once

#include <QColor>
#include <QList>
#include <QWidget>

#include "core/Device.h"

namespace pacn {

// Lightweight, dependency-free dashboard charts: a device-type donut and a
// top-services bar chart, custom-painted so no QtCharts dependency is required.
class MiniCharts : public QWidget {
    Q_OBJECT
public:
    explicit MiniCharts(QWidget* parent = nullptr);

    void setDevices(const QList<Device>& devices);
    void setAccent(const QColor& accent);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize minimumSizeHint() const override;

private:
    void paintDonut(QPainter& p, const QRectF& area);
    void paintBars(QPainter& p, const QRectF& area);

    QList<Device> devices_;
    QColor accent_{0x2b, 0xbf, 0xd6};
};

}  // namespace pacn
