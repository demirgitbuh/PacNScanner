#include <QCoreApplication>
#include <gtest/gtest.h>

// A QCoreApplication is required because the scan engine, hostname resolver and
// QObject machinery rely on Qt's event loop / thread infrastructure.
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
