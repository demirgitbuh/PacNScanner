#include <gtest/gtest.h>

#include <QApplication>

#include "ui/DeviceTableModel.h"
#include "ui/MainWindow.h"

using namespace pacn;

TEST(UiSmoke, DeviceTableModelUpsertIsIdempotentByIp) {
    DeviceTableModel model;
    Device d;
    d.ip = QHostAddress(QStringLiteral("192.168.1.1"));
    d.status = DeviceStatus::Online;
    model.upsert(d);
    EXPECT_EQ(model.rowCount(), 1);

    d.hostname = QStringLiteral("router");
    model.upsert(d);  // same IP -> update, not insert
    EXPECT_EQ(model.rowCount(), 1);
    EXPECT_EQ(model.deviceAt(0).hostname.toStdString(), "router");
}

TEST(UiSmoke, DeviceTableModelColumns) {
    DeviceTableModel model;
    EXPECT_EQ(model.columnCount(), DeviceTableModel::ColumnCount);
}

TEST(UiSmoke, MainWindowConstructsAndShows) {
    MainWindow window;
    window.show();
    QCoreApplication::processEvents();
    SUCCEED();
}

int main(int argc, char** argv) {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
