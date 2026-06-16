#include <QCommandLineParser>
#include <QCoreApplication>
#include <QHash>
#include <QTextStream>
#include <QTimer>

#include <algorithm>

#include <pacn/version.h>

#include "core/IpRange.h"
#include "core/Logger.h"
#include "core/PortProfiles.h"
#include "core/ScanConfig.h"
#include "core/ScanEngine.h"
#include "reporting/Exporter.h"

using namespace pacn;

namespace {

QTextStream& out() {
    static QTextStream s(stdout);
    return s;
}
QTextStream& err() {
    static QTextStream s(stderr);
    return s;
}

ScanMethods parseMethods(const QString& csv) {
    ScanMethods m;
    for (const QString& tok : csv.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const QString t = tok.trimmed().toLower();
        if (t == QLatin1String("arp")) m |= ScanMethod::Arp;
        else if (t == QLatin1String("icmp")) m |= ScanMethod::Icmp;
        else if (t == QLatin1String("tcp")) m |= ScanMethod::TcpConnect;
        else if (t == QLatin1String("raw")) m |= ScanMethod::Raw;
    }
    if (!m) m |= ScanMethod::TcpConnect | ScanMethod::Icmp;
    return m;
}

void printTable(const QList<Device>& devices) {
    out() << '\n'
          << QStringLiteral("%1  %2  %3  %4  %5")
                 .arg(QStringLiteral("IP"), -16)
                 .arg(QStringLiteral("MAC"), -18)
                 .arg(QStringLiteral("HOSTNAME"), -24)
                 .arg(QStringLiteral("RISK"), -10)
                 .arg(QStringLiteral("OPEN PORTS"))
          << '\n';
    out() << QString(90, QLatin1Char('-')) << '\n';
    for (const Device& d : devices) {
        out() << QStringLiteral("%1  %2  %3  %4  %5")
                     .arg(d.ipString(), -16)
                     .arg(d.mac.isEmpty() ? QStringLiteral("-") : d.mac, -18)
                     .arg(d.hostname.isEmpty() ? QStringLiteral("-") : d.hostname.left(23), -24)
                     .arg(toString(d.riskLevel), -10)
                     .arg(d.openPortNumbers().join(QLatin1Char(',')))
              << '\n';
    }
    out().flush();
}

}  // namespace

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("pacnscanner-cli"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(kVersionString));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("PacNScanner CLI — fast local network scanner.\n"
                       "Use only on networks you own or are authorized to assess."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("targets"),
                                 QStringLiteral("CIDR/range/host targets"),
                                 QStringLiteral("[targets...]"));
    parser.addOptions({
        {{QStringLiteral("p"), QStringLiteral("ports")},
         QStringLiteral("Port profile name or spec (e.g. \"Top 100\" or 22,80,443,8000-8100)."),
         QStringLiteral("ports"), QStringLiteral("Top 100")},
        {{QStringLiteral("m"), QStringLiteral("methods")},
         QStringLiteral("Discovery methods csv: arp,icmp,tcp,raw."),
         QStringLiteral("methods"), QStringLiteral("icmp,tcp,arp")},
        {{QStringLiteral("s"), QStringLiteral("speed")},
         QStringLiteral("Speed profile: slow|normal|fast."), QStringLiteral("speed"),
         QStringLiteral("normal")},
        {QStringLiteral("no-hostnames"), QStringLiteral("Disable hostname resolution.")},
        {QStringLiteral("no-banners"), QStringLiteral("Disable banner grabbing.")},
        {{QStringLiteral("o"), QStringLiteral("output")},
         QStringLiteral("Write a report to FILE (format from extension)."),
         QStringLiteral("file")},
        {{QStringLiteral("f"), QStringLiteral("format")},
         QStringLiteral("Report format: csv|json|html (overrides extension)."),
         QStringLiteral("format")},
    });
    parser.process(app);

    const QStringList targets = parser.positionalArguments();
    if (targets.isEmpty()) {
        err() << "error: at least one target is required\n";
        return 2;
    }

    ScanConfig cfg;
    QStringList errors;
    for (const QString& t : targets) cfg.targets += IpRange::parseList(t, &errors);
    for (const QString& e : errors) err() << "warning: " << e << '\n';
    if (cfg.targets.isEmpty()) {
        err() << "error: no valid targets\n";
        return 2;
    }

    const QString portsArg = parser.value(QStringLiteral("ports"));
    if (portprofiles::names().contains(portsArg))
        cfg.ports = portprofiles::forName(portsArg);
    else
        cfg.ports = portprofiles::parseSpec(portsArg);
    cfg.methods = parseMethods(parser.value(QStringLiteral("methods")));
    cfg.speed = speedProfileFromString(parser.value(QStringLiteral("speed")));
    cfg.resolveHostnames = !parser.isSet(QStringLiteral("no-hostnames"));
    cfg.hostnameMethods = cfg.resolveHostnames ? HostnameMethods(HostnameMethod::Dns)
                                               : HostnameMethods(HostnameMethod::None);
    cfg.grabBanners = !parser.isSet(QStringLiteral("no-banners"));

    out() << "PacNScanner CLI " << kVersionString << " — scanning "
          << cfg.totalHostCount() << " host(s)...\n";
    out().flush();

    auto* engine = new ScanEngine(&app);
    auto* devices = new QHash<QString, Device>;

    QObject::connect(engine, &ScanEngine::deviceDiscovered, &app,
                     [devices](const Device& d) { devices->insert(d.ipString(), d); });
    QObject::connect(engine, &ScanEngine::deviceUpdated, &app,
                     [devices](const Device& d) { devices->insert(d.ipString(), d); });
    QObject::connect(engine, &ScanEngine::progress, &app,
                     [](quint64 done, quint64 total) {
                         err() << "\r  " << done << " / " << total << " scanned";
                         err().flush();
                     });

    QObject::connect(engine, &ScanEngine::finished, &app,
                     [&app, engine, devices, &parser](const ScanSummary& summary) {
                         err() << "\n";
                         QList<Device> list = devices->values();
                         std::sort(list.begin(), list.end(),
                                   [](const Device& a, const Device& b) {
                                       return a.ip.toIPv4Address() < b.ip.toIPv4Address();
                                   });
                         printTable(list);
                         out() << QStringLiteral("\n%1 up / %2 scanned in %3 s, %4 at risk\n")
                                      .arg(summary.hostsUp)
                                      .arg(summary.scanned)
                                      .arg(QString::number(summary.elapsedMs / 1000.0, 'f', 1))
                                      .arg(summary.risky);

                         const QString output = parser.value(QStringLiteral("output"));
                         if (!output.isEmpty()) {
                             reporting::ReportContext ctx;
                             ctx.summary = summary;
                             QString fmt = parser.value(QStringLiteral("format")).toLower();
                             if (fmt.isEmpty()) {
                                 if (output.endsWith(QLatin1String(".csv"))) fmt = QStringLiteral("csv");
                                 else if (output.endsWith(QLatin1String(".json"))) fmt = QStringLiteral("json");
                                 else fmt = QStringLiteral("html");
                             }
                             QString e;
                             bool ok = fmt == QLatin1String("csv")
                                           ? reporting::exportCsv(list, output, &e)
                                           : fmt == QLatin1String("json")
                                                 ? reporting::exportJson(list, ctx, output, &e)
                                                 : reporting::exportHtml(list, ctx, output, &e);
                             if (ok) out() << "Report written to " << output << '\n';
                             else err() << "export failed: " << e << '\n';
                         }
                         out().flush();
                         engine->deleteLater();
                         delete devices;
                         app.quit();
                     });

    QTimer::singleShot(0, engine, [engine, cfg] {
        ScanEngine* e = engine;
        e->setConfig(cfg);
        e->start();
    });

    return app.exec();
}
