#include <gtest/gtest.h>

#include <QTemporaryDir>

#include "reporting/Exporter.h"

using namespace pacn;

namespace {
QList<Device> sampleDevices() {
    Device a;
    a.ip = QHostAddress(QStringLiteral("192.168.1.10"));
    a.mac = QStringLiteral("AA:BB:CC:DD:EE:FF");
    a.hostname = QStringLiteral("nas.local");
    a.vendor = QStringLiteral("Synology");
    a.type = DeviceType::Nas;
    a.status = DeviceStatus::Online;
    a.labels = {QStringLiteral("home"), QStringLiteral("storage")};
    Port p;
    p.number = 445;
    p.state = PortState::Open;
    p.service = QStringLiteral("microsoft-ds");
    a.ports.push_back(p);

    Device b;
    b.ip = QHostAddress(QStringLiteral("192.168.1.20"));
    b.hostname = QStringLiteral("printer");
    b.type = DeviceType::Printer;
    b.status = DeviceStatus::Online;
    return {a, b};
}
}  // namespace

TEST(Reporting, CsvContainsHeaderAndRows) {
    const QString csv = reporting::toCsv(sampleDevices());
    EXPECT_TRUE(csv.contains(QStringLiteral("IP,MAC,Hostname")));
    EXPECT_TRUE(csv.contains(QStringLiteral("192.168.1.10")));
    EXPECT_TRUE(csv.contains(QStringLiteral("AA:BB:CC:DD:EE:FF")));
}

TEST(Reporting, HtmlIsSelfContained) {
    reporting::ReportContext ctx;
    ctx.network = QStringLiteral("192.168.1.0/24");
    const QString html = reporting::toHtml(sampleDevices(), ctx);
    EXPECT_TRUE(html.contains(QStringLiteral("<!DOCTYPE html>")));
    EXPECT_TRUE(html.contains(QStringLiteral("192.168.1.10")));
    EXPECT_TRUE(html.contains(QStringLiteral("Ethical use")));
}

TEST(Reporting, JsonRoundTrip) {
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("out.json"));
    reporting::ReportContext ctx;
    QString err;
    ASSERT_TRUE(reporting::exportJson(sampleDevices(), ctx, path, &err)) << err.toStdString();

    const QList<Device> back = reporting::importJson(path, &err);
    ASSERT_EQ(back.size(), 2) << err.toStdString();
    EXPECT_EQ(back.first().ipString().toStdString(), "192.168.1.10");
    EXPECT_EQ(back.first().mac.toStdString(), "AA:BB:CC:DD:EE:FF");
    EXPECT_EQ(back.first().openPorts().size(), 1);
}

TEST(Reporting, CsvRoundTrip) {
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("out.csv"));
    QString err;
    ASSERT_TRUE(reporting::exportCsv(sampleDevices(), path, &err)) << err.toStdString();

    const QList<Device> back = reporting::importCsv(path, &err);
    ASSERT_EQ(back.size(), 2);
    EXPECT_EQ(back.first().ipString().toStdString(), "192.168.1.10");
    EXPECT_TRUE(back.first().labels.contains(QStringLiteral("home")));
}
