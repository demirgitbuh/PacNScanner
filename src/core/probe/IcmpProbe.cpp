#include "core/probe/IcmpProbe.h"

#include <QProcess>
#include <QRegularExpression>
#include <QtGlobal>

namespace pacn {

HostProbeResult IcmpHostProbe::probe(const QHostAddress& host, const ScanConfig& cfg) {
    HostProbeResult result;
    result.method = QStringLiteral("icmp");

    const QString ip = host.toString();
    const int timeoutMs = cfg.hostTimeoutMs();
    QString program = QStringLiteral("ping");
    QStringList args;

#if defined(Q_OS_WIN)
    args << QStringLiteral("-n") << QStringLiteral("1")
         << QStringLiteral("-w") << QString::number(timeoutMs) << ip;
#elif defined(Q_OS_MACOS)
    const int secs = qMax(1, (timeoutMs + 999) / 1000);
    args << QStringLiteral("-c") << QStringLiteral("1")
         << QStringLiteral("-t") << QString::number(secs) << ip;
#else  // Linux / other Unix
    const int secs = qMax(1, (timeoutMs + 999) / 1000);
    args << QStringLiteral("-c") << QStringLiteral("1")
         << QStringLiteral("-W") << QString::number(secs) << ip;
#endif

    QProcess proc;
    proc.start(program, args);
    if (!proc.waitForStarted(1000)) return result;
    if (!proc.waitForFinished(timeoutMs + 1500)) {
        proc.kill();
        proc.waitForFinished(300);
        return result;
    }

    const QString out = QString::fromLocal8Bit(proc.readAllStandardOutput());
    if (proc.exitCode() != 0) {
        // Some pings exit 0 even on partial loss; double-check the text.
        if (!out.contains(QLatin1String("ttl"), Qt::CaseInsensitive) &&
            !out.contains(QLatin1String("TTL"))) {
            return result;
        }
    }
    // "Reply from" / "bytes from" without "unreachable" indicates success.
    if (out.contains(QLatin1String("unreachable"), Qt::CaseInsensitive) ||
        out.contains(QLatin1String("100% packet loss")) ||
        out.contains(QLatin1String("100% loss"))) {
        return result;
    }

    result.alive = true;

    static const QRegularExpression ttlRe(QStringLiteral("ttl[=:]\\s*(\\d+)"),
                                          QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch ttlM = ttlRe.match(out);
    if (ttlM.hasMatch()) result.ttl = ttlM.captured(1).toInt();

    static const QRegularExpression timeRe(
        QStringLiteral("time[=<]\\s*([0-9]+(?:\\.[0-9]+)?)\\s*ms"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch timeM = timeRe.match(out);
    if (timeM.hasMatch()) result.rttMs = static_cast<int>(timeM.captured(1).toDouble());

    return result;
}

}  // namespace pacn
