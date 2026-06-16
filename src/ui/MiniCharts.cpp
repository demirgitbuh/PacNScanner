#include "ui/MiniCharts.h"

#include <QPainter>
#include <QtMath>
#include <algorithm>
#include <map>
#include <utility>
#include <vector>

namespace pacn {

namespace {
const QList<QColor>& seriesColors() {
    static const QList<QColor> c = {
        QColor(0x2b, 0xbf, 0xd6), QColor(0x27, 0xae, 0x60), QColor(0xe6, 0x7e, 0x22),
        QColor(0x9b, 0x59, 0xb6), QColor(0xf1, 0xc4, 0x0f), QColor(0xe7, 0x4c, 0x3c),
        QColor(0x34, 0x98, 0xdb), QColor(0x1a, 0xbc, 0x9c), QColor(0x95, 0xa5, 0xa6)};
    return c;
}
}  // namespace

MiniCharts::MiniCharts(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(180);
}

void MiniCharts::setDevices(const QList<Device>& devices) {
    devices_ = devices;
    update();
}

void MiniCharts::setAccent(const QColor& accent) {
    accent_ = accent;
    update();
}

QSize MiniCharts::minimumSizeHint() const { return {360, 180}; }

void MiniCharts::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    const QRectF r = rect().adjusted(8, 8, -8, -8);
    const QRectF left(r.left(), r.top(), r.width() * 0.42, r.height());
    const QRectF right(left.right() + 16, r.top(), r.right() - left.right() - 16, r.height());
    paintDonut(p, left);
    paintBars(p, right);
}

void MiniCharts::paintDonut(QPainter& p, const QRectF& area) {
    std::map<QString, int> counts;
    for (const Device& d : devices_) counts[toString(d.type)]++;

    int total = 0;
    for (const auto& kv : counts) total += kv.second;

    const qreal side = qMin(area.width(), area.height()) - 36;
    const QRectF ring(area.center().x() - side / 2, area.top() + 6, side, side);

    p.setPen(palette().color(QPalette::WindowText));
    p.drawText(QRectF(area.left(), area.bottom() - 18, area.width(), 18),
               Qt::AlignCenter, tr("Device types"));

    if (total == 0) {
        p.setPen(QColor(0x95, 0xa5, 0xa6));
        p.drawText(ring, Qt::AlignCenter, tr("No data"));
        return;
    }

    int startAngle = 90 * 16;
    int idx = 0;
    for (const auto& kv : counts) {
        const int span = static_cast<int>(std::round(360.0 * 16 * kv.second / total));
        p.setBrush(seriesColors()[idx % seriesColors().size()]);
        p.setPen(Qt::NoPen);
        p.drawPie(ring, startAngle, -span);
        startAngle -= span;
        ++idx;
    }
    // Punch the hole for a donut.
    p.setBrush(palette().color(QPalette::Window));
    p.drawEllipse(ring.center(), side * 0.28, side * 0.28);
    p.setPen(palette().color(QPalette::WindowText));
    QFont f = p.font();
    f.setPointSizeF(f.pointSizeF() * 1.4);
    f.setBold(true);
    p.setFont(f);
    p.drawText(ring, Qt::AlignCenter, QString::number(total));
}

void MiniCharts::paintBars(QPainter& p, const QRectF& area) {
    std::map<QString, int> svc;
    for (const Device& d : devices_)
        for (const QString& s : d.serviceNames()) svc[s]++;

    std::vector<std::pair<QString, int>> items(svc.begin(), svc.end());
    std::sort(items.begin(), items.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    if (items.size() > 6) items.resize(6);

    p.setPen(palette().color(QPalette::WindowText));
    p.drawText(QRectF(area.left(), area.top(), area.width(), 18),
               Qt::AlignLeft | Qt::AlignVCenter, tr("Top services"));

    if (items.empty()) {
        p.setPen(QColor(0x95, 0xa5, 0xa6));
        p.drawText(area, Qt::AlignCenter, tr("No open services"));
        return;
    }

    int maxVal = 1;
    for (const auto& it : items) maxVal = qMax(maxVal, it.second);

    const qreal top = area.top() + 26;
    const qreal rowH = qMin<qreal>(26, (area.height() - 30) / items.size());
    const qreal labelW = 96;
    const qreal barArea = area.width() - labelW - 36;

    for (size_t i = 0; i < items.size(); ++i) {
        const qreal y = top + i * rowH;
        p.setPen(palette().color(QPalette::WindowText));
        p.drawText(QRectF(area.left(), y, labelW, rowH - 4),
                   Qt::AlignLeft | Qt::AlignVCenter, items[i].first);
        const qreal w = barArea * items[i].second / maxVal;
        p.setPen(Qt::NoPen);
        p.setBrush(accent_);
        p.drawRoundedRect(QRectF(area.left() + labelW, y + 3, qMax<qreal>(2, w), rowH - 10),
                          4, 4);
        p.setPen(palette().color(QPalette::WindowText));
        p.drawText(QRectF(area.left() + labelW + w + 4, y, 30, rowH - 4),
                   Qt::AlignLeft | Qt::AlignVCenter, QString::number(items[i].second));
    }
}

}  // namespace pacn
