#pragma once

#include <QHostAddress>
#include <QString>
#include <memory>

#include "core/PlatformServices.h"
#include "core/Types.h"

namespace pacn {

// Resolves a hostname for an address via reverse DNS, mDNS and NetBIOS. mDNS
// and NetBIOS are delegated to the platform layer (and may return empty on
// platforms/builds without support — that is handled gracefully).
class HostnameResolver {
public:
    explicit HostnameResolver(std::shared_ptr<IPlatformServices> platform = nullptr)
        : platform_(std::move(platform)) {}

    QString resolve(const QHostAddress& ip, HostnameMethods methods, int timeoutMs) const;

    static QString reverseDns(const QHostAddress& ip);

private:
    std::shared_ptr<IPlatformServices> platform_;
};

}  // namespace pacn
