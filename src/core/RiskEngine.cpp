#include "core/RiskEngine.h"

#include "core/ServiceDetector.h"

namespace pacn {

RiskLevel RiskEngine::levelForScore(int score) {
    if (score >= 80) return RiskLevel::Critical;
    if (score >= 55) return RiskLevel::High;
    if (score >= 30) return RiskLevel::Medium;
    if (score >= 10) return RiskLevel::Low;
    return RiskLevel::Info;
}

void RiskEngine::evaluate(Device& device) const {
    ServiceDetector& svc = ServiceDetector::instance();
    svc.ensureLoaded();

    int score = 0;
    QList<RiskFinding> findings;
    const QList<Port> open = device.openPorts();

    for (const Port& p : open) {
        const QStringList flags = svc.flags(p.number);
        const QString name = p.service.isEmpty() ? svc.serviceName(p.number) : p.service;

        // Specific high-signal services.
        switch (p.number) {
            case 23:
                score += 30;
                findings.push_back({RiskLevel::Critical, QStringLiteral("Telnet exposed"),
                                    QStringLiteral("Port 23 transmits credentials in clear "
                                                   "text. Disable Telnet; use SSH.")});
                break;
            case 21:
                score += 18;
                findings.push_back({RiskLevel::High, QStringLiteral("FTP exposed"),
                                    QStringLiteral("Port 21 (FTP) is typically unencrypted.")});
                break;
            case 5900:
                score += 22;
                findings.push_back({RiskLevel::High, QStringLiteral("VNC exposed"),
                                    QStringLiteral("Remote desktop (VNC) reachable on the "
                                                   "network.")});
                break;
            case 3389:
                score += 16;
                findings.push_back({RiskLevel::High, QStringLiteral("RDP exposed"),
                                    QStringLiteral("Windows Remote Desktop is reachable.")});
                break;
            case 445:
            case 139:
                score += 12;
                findings.push_back({RiskLevel::Medium, QStringLiteral("SMB file sharing"),
                                    QStringLiteral("SMB/NetBIOS sharing is exposed.")});
                break;
            case 161:
                score += 15;
                findings.push_back({RiskLevel::High, QStringLiteral("SNMP exposed"),
                                    QStringLiteral("SNMP can leak device details; v1/v2c is "
                                                   "unauthenticated.")});
                break;
            case 6379:
            case 11211:
            case 27017:
            case 9200:
                score += 18;
                findings.push_back({RiskLevel::High,
                                    QStringLiteral("Database reachable: %1").arg(name),
                                    QStringLiteral("Datastores often ship without auth — "
                                                   "verify it is firewalled.")});
                break;
            default:
                break;
        }

        if (flags.contains(QStringLiteral("plaintext"))) {
            score += 8;
            findings.push_back({RiskLevel::Medium,
                                QStringLiteral("Plaintext service: %1").arg(name),
                                QStringLiteral("Traffic on port %1 is not encrypted.")
                                    .arg(p.number)});
        }
        if (flags.contains(QStringLiteral("db")))
            score += 6;
        if (flags.contains(QStringLiteral("admin")) && flags.contains(QStringLiteral("remote")))
            score += 6;
        if (flags.contains(QStringLiteral("web")) && flags.contains(QStringLiteral("admin"))) {
            score += 8;
            findings.push_back({RiskLevel::Medium,
                                QStringLiteral("Admin web panel: %1").arg(name),
                                QStringLiteral("Management interface exposed on port %1.")
                                    .arg(p.number)});
        }
    }

    if (open.size() > 10) {
        score += 10;
        findings.push_back({RiskLevel::Low, QStringLiteral("Large attack surface"),
                            QStringLiteral("%1 open ports detected.").arg(open.size())});
    }

    if (open.isEmpty() && device.status == DeviceStatus::Online) {
        findings.push_back({RiskLevel::Info, QStringLiteral("No open ports found"),
                            QStringLiteral("Host responded but exposed no scanned ports.")});
    }

    score = qBound(0, score, 100);
    device.riskScore = score;
    device.riskLevel = levelForScore(score);
    device.risks = findings;
}

}  // namespace pacn
