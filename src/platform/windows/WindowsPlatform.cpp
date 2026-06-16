// Winsock headers must precede windows.h.
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>

#include "platform/windows/WindowsPlatform.h"

#include <QProcess>
#include <QRegularExpression>

#include "core/NetUtils.h"
#include "platform/AdapterUtil.h"

namespace pacn {

namespace {

QString macFromBytes(const BYTE* bytes, DWORD len) {
    QStringList parts;
    for (DWORD i = 0; i < len; ++i)
        parts << QStringLiteral("%1").arg(bytes[i], 2, 16, QLatin1Char('0')).toUpper();
    return parts.join(QLatin1Char(':'));
}

QString runCommand(const QString& program, const QStringList& args, int timeoutMs = 3000) {
    QProcess proc;
    proc.start(program, args);
    if (!proc.waitForStarted(1000)) return {};
    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        proc.waitForFinished(200);
        return {};
    }
    return QString::fromLocal8Bit(proc.readAllStandardOutput());
}

QString currentSsid() {
    const QString out =
        runCommand(QStringLiteral("netsh"),
                   {QStringLiteral("wlan"), QStringLiteral("show"),
                    QStringLiteral("interfaces")});
    for (const QString& line : out.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const QString l = line.trimmed();
        if (l.startsWith(QLatin1String("SSID")) && !l.startsWith(QLatin1String("BSSID"))) {
            const int colon = l.indexOf(QLatin1Char(':'));
            if (colon > 0) return l.mid(colon + 1).trimmed();
        }
    }
    return {};
}

}  // namespace

QList<AdapterInfo> WindowsPlatform::enumerateAdapters() {
    QList<AdapterInfo> adapters = adapterutil::enumerateViaQt();
    const QString ssid = currentSsid();
    if (!ssid.isEmpty()) {
        for (AdapterInfo& a : adapters)
            if (a.isWireless && a.isUp) a.wifiSsid = ssid;
    }
    return adapters;
}

QList<ArpEntry> WindowsPlatform::arpTable() {
    QList<ArpEntry> out;
    ULONG size = 0;
    if (GetIpNetTable(nullptr, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER) return out;

    QByteArray buffer(static_cast<int>(size), Qt::Uninitialized);
    auto* table = reinterpret_cast<PMIB_IPNETTABLE>(buffer.data());
    if (GetIpNetTable(table, &size, TRUE) != NO_ERROR) return out;

    for (DWORD i = 0; i < table->dwNumEntries; ++i) {
        const MIB_IPNETROW& row = table->table[i];
        if (row.dwType == MIB_IPNET_TYPE_INVALID || row.dwPhysAddrLen == 0) continue;
        const QString mac = netutils::normalizeMac(
            macFromBytes(row.bPhysAddr, row.dwPhysAddrLen));
        if (mac.isEmpty() || mac == QLatin1String("00:00:00:00:00:00")) continue;
        ArpEntry e;
        e.ip = QHostAddress(ntohl(row.dwAddr));
        e.mac = mac;
        out.push_back(e);
    }
    return out;
}

QList<GatewayInfo> WindowsPlatform::defaultGateways() {
    QList<GatewayInfo> out;
    ULONG flags = GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_SKIP_DNS_SERVER |
                  GAA_FLAG_SKIP_MULTICAST;
    ULONG size = 15000;
    QByteArray buffer(static_cast<int>(size), Qt::Uninitialized);
    auto* addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

    ULONG ret = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses, &size);
    if (ret == ERROR_BUFFER_OVERFLOW) {
        buffer.resize(static_cast<int>(size));
        addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
        ret = GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, addresses, &size);
    }
    if (ret != NO_ERROR) return out;

    for (auto* ad = addresses; ad; ad = ad->Next) {
        for (auto* gw = ad->FirstGatewayAddress; gw; gw = gw->Next) {
            if (!gw->Address.lpSockaddr) continue;
            const QHostAddress addr(gw->Address.lpSockaddr);
            if (addr.isNull()) continue;
            bool dup = false;
            for (const GatewayInfo& g : out)
                if (g.address == addr) dup = true;
            if (dup) continue;
            GatewayInfo g;
            g.address = addr;
            g.adapter = QString::fromWCharArray(ad->FriendlyName);
            out.push_back(g);
        }
    }
    return out;
}

bool WindowsPlatform::isElevated() {
    BOOL elevated = FALSE;
    HANDLE token = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION elevation;
        DWORD cb = sizeof(elevation);
        if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &cb))
            elevated = elevation.TokenIsElevated;
        CloseHandle(token);
    }
    return elevated == TRUE;
}

bool WindowsPlatform::rawSocketsAvailable() {
    // Raw/ARP-active scanning needs Npcap; absent that, report elevation so the
    // UI can hint at what extra capabilities elevation would unlock.
    return isElevated();
}

}  // namespace pacn
