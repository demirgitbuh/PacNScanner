#include "core/HostnameResolver.h"

#include <QEventLoop>
#include <QHostInfo>
#include <QTimer>

namespace pacn {

namespace {

QString reverseDnsBounded(const QHostAddress& ip, int timeoutMs) {
    const QString ipStr = ip.toString();
    QString result;
    QEventLoop loop;
    bool finished = false;

    // Reverse lookup is delivered to this thread's (local) event loop.
    QHostInfo::lookupHost(ipStr, [&](const QHostInfo& info) {
        if (info.error() == QHostInfo::NoError) {
            const QString name = info.hostName();
            if (!name.isEmpty() && name != ipStr) result = name;
        }
        finished = true;
        loop.quit();
    });

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(qMax(200, timeoutMs));
    if (!finished) loop.exec();
    return result;
}

}  // namespace

QString HostnameResolver::reverseDns(const QHostAddress& ip) {
    return reverseDnsBounded(ip, 1500);
}

QString HostnameResolver::resolve(const QHostAddress& ip, HostnameMethods methods,
                                  int timeoutMs) const {
    if (methods.testFlag(HostnameMethod::Dns)) {
        const QString dns = reverseDnsBounded(ip, timeoutMs);
        if (!dns.isEmpty()) return dns;
    }
    if (platform_) {
        if (methods.testFlag(HostnameMethod::Mdns)) {
            const QString m = platform_->mdnsName(ip);
            if (!m.isEmpty()) return m;
        }
        if (methods.testFlag(HostnameMethod::NetBios)) {
            const QString nb = platform_->netbiosName(ip);
            if (!nb.isEmpty()) return nb;
        }
    }
    return {};
}

}  // namespace pacn
