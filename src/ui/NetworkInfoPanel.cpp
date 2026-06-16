#include "ui/NetworkInfoPanel.h"

#include <QFormLayout>
#include <QLabel>

namespace pacn {

NetworkInfoPanel::NetworkInfoPanel(QWidget* parent) : QFrame(parent) {
    setObjectName(QStringLiteral("statCard"));
    auto* form = new QFormLayout(this);
    form->setContentsMargins(18, 16, 18, 16);
    form->setSpacing(8);
    form->setLabelAlignment(Qt::AlignLeft);

    const auto caption = [](const QString& t) {
        auto* l = new QLabel(t);
        l->setObjectName(QStringLiteral("statLabel"));
        return l;
    };
    adapterValue_ = new QLabel(QStringLiteral("—"));
    ipValue_ = new QLabel(QStringLiteral("—"));
    gatewayValue_ = new QLabel(QStringLiteral("—"));
    wifiValue_ = new QLabel(QStringLiteral("—"));
    privValue_ = new QLabel(QStringLiteral("—"));
    for (QLabel* v : {adapterValue_, ipValue_, gatewayValue_, wifiValue_, privValue_})
        v->setTextInteractionFlags(Qt::TextSelectableByMouse);

    form->addRow(caption(tr("Adapter")), adapterValue_);
    form->addRow(caption(tr("IP / CIDR")), ipValue_);
    form->addRow(caption(tr("Gateway")), gatewayValue_);
    form->addRow(caption(tr("Wi-Fi")), wifiValue_);
    form->addRow(caption(tr("Privilege")), privValue_);
}

void NetworkInfoPanel::setPlatform(std::shared_ptr<IPlatformServices> platform) {
    platform_ = std::move(platform);
    refresh();
}

QString NetworkInfoPanel::suggestedTarget() const { return suggestedTarget_; }

void NetworkInfoPanel::refresh() {
    if (!platform_) return;

    const QList<AdapterInfo> adapters = platform_->enumerateAdapters();
    const AdapterInfo* best = nullptr;
    for (const AdapterInfo& a : adapters) {
        if (!a.isUp || a.isLoopback || a.primaryIpv4.isNull()) continue;
        if (a.isVirtual || a.isVpn) {
            if (!best) best = &a;  // accept as fallback only
            continue;
        }
        best = &a;
        if (a.isWireless) break;  // prefer Wi-Fi when present
    }

    if (best) {
        adapterValue_->setText(best->description.isEmpty() ? best->name : best->description);
        ipValue_->setText(QStringLiteral("%1/%2")
                              .arg(best->primaryIpv4.toString())
                              .arg(best->prefixLength));
        // Derive a /CIDR target from the adapter network.
        const quint32 ip = best->primaryIpv4.toIPv4Address();
        const int prefix = best->prefixLength;
        const quint32 mask = prefix == 0 ? 0u : (0xFFFFFFFFu << (32 - prefix));
        const QHostAddress network(ip & mask);
        suggestedTarget_ = QStringLiteral("%1/%2").arg(network.toString()).arg(prefix);

        if (best->isWireless) {
            wifiValue_->setText(best->wifiSsid.isEmpty()
                                    ? tr("Connected")
                                    : best->wifiSsid +
                                          (best->wifiSignal >= 0
                                               ? QStringLiteral(" (%1%)").arg(best->wifiSignal)
                                               : QString()));
        } else {
            wifiValue_->setText(tr("Wired"));
        }
    }

    const QList<GatewayInfo> gws = platform_->defaultGateways();
    if (!gws.isEmpty()) {
        gateway_ = gws.first().address;
        gatewayValue_->setText(gateway_.toString());
        emit gatewayDetected(gateway_);
    }

    privValue_->setText(platform_->isElevated() ? tr("Elevated (raw scan available)")
                                                : tr("Standard (TCP/ICMP)"));
}

}  // namespace pacn
