#pragma once

#include <QHostAddress>
#include <QString>

namespace pacn::netutils {

// Normalizes any MAC representation ("aa-bb-cc...", "aabb.ccdd.eeff",
// "AA:BB:CC:DD:EE:FF", stray spaces) to canonical "AA:BB:CC:DD:EE:FF".
// Returns an empty string when the input has fewer than 12 hex digits.
QString normalizeMac(const QString& raw);

// First 6 hex digits (the OUI) of a MAC, uppercased, separators stripped.
QString ouiKey(const QString& mac);

// Numeric, version-aware comparison suitable for "smart" IP sorting:
// IPv4 sorts before IPv6; within a family, compared by raw address bytes.
// Returns <0, 0 or >0.
int compareIp(const QHostAddress& a, const QHostAddress& b);

// Returns the next address (IPv4 or IPv6). Wraps are clamped at the max.
QHostAddress nextAddress(const QHostAddress& addr);

// RFC1918 / ULA / link-local detection — used for the "authorized local
// network" guidance and risk context.
bool isPrivate(const QHostAddress& addr);

// Network/broadcast addresses for an IPv4 network (used to skip in /24 etc.).
bool isNetworkOrBroadcast(const QHostAddress& addr, int prefixLength,
                          const QHostAddress& network);

}  // namespace pacn::netutils
