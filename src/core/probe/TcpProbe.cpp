#include "core/probe/TcpProbe.h"

#include <QElapsedTimer>
#include <QTcpSocket>

namespace pacn {

HostProbeResult TcpHostProbe::probe(const QHostAddress& host, const ScanConfig& cfg) {
    HostProbeResult result;
    result.method = QStringLiteral("tcp");

    // A handful of ports likely to provoke either SYN/ACK or RST.
    static const quint16 kPorts[] = {80, 443, 22, 445, 139, 53};
    const int perPort = qMax(120, cfg.hostTimeoutMs() / 2);

    QElapsedTimer clock;
    for (quint16 port : kPorts) {
        QTcpSocket sock;
        clock.start();
        sock.connectToHost(host, port);
        if (sock.waitForConnected(perPort)) {
            result.alive = true;
            result.rttMs = static_cast<int>(clock.elapsed());
            sock.abort();
            return result;
        }
        // An explicit refusal still proves the host exists.
        if (sock.error() == QAbstractSocket::ConnectionRefusedError) {
            result.alive = true;
            result.rttMs = static_cast<int>(clock.elapsed());
            return result;
        }
        sock.abort();
    }
    return result;
}

PortProbeResult TcpPortProbe::probe(const QHostAddress& host, quint16 port, int timeoutMs,
                                    bool grabBanner) {
    PortProbeResult result;
    QTcpSocket sock;
    sock.connectToHost(host, port);
    if (!sock.waitForConnected(qMax(120, timeoutMs))) return result;
    result.open = true;

    if (grabBanner) {
        // Web ports rarely speak first — nudge them; others (FTP/SSH/SMTP) greet.
        if (port == 80 || port == 8080 || port == 8000 || port == 8888 || port == 591) {
            sock.write("GET / HTTP/1.0\r\nHost: scan\r\nUser-Agent: PacNScanner\r\n\r\n");
            sock.flush();
        }
        if (sock.waitForReadyRead(qMin(qMax(200, timeoutMs), 1200))) {
            const QByteArray data = sock.read(512);
            QString banner = QString::fromLatin1(data).trimmed();
            banner.replace(QLatin1Char('\n'), QLatin1Char(' '));
            banner.replace(QLatin1Char('\r'), QLatin1Char(' '));
            result.banner = banner.left(200);
        }
    }
    sock.abort();
    return result;
}

}  // namespace pacn
