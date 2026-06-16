#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLocale>
#include <QSettings>
#include <QStandardPaths>
#include <QTranslator>

#include <pacn/version.h>

#include "core/Logger.h"
#include "ui/IconProvider.h"
#include "ui/MainWindow.h"

using namespace pacn;

namespace {

void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg) {
    LogLevel level = LogLevel::Info;
    switch (type) {
        case QtDebugMsg:    level = LogLevel::Debug; break;
        case QtInfoMsg:     level = LogLevel::Info; break;
        case QtWarningMsg:  level = LogLevel::Warning; break;
        case QtCriticalMsg:
        case QtFatalMsg:    level = LogLevel::Error; break;
    }
    Logger::instance().log(level, QStringLiteral("qt"), msg);
}

// Returns false if another instance already owns the lock (claims it otherwise).
bool claimSingleInstance(QLocalServer& server) {
    const QString name = QStringLiteral("PacNScanner.singleton.%1")
                             .arg(QStandardPaths::writableLocation(
                                      QStandardPaths::HomeLocation));
    QLocalSocket probe;
    probe.connectToServer(name);
    if (probe.waitForConnected(150)) return false;  // already running

    QLocalServer::removeServer(name);
    server.listen(name);
    return true;
}

void installTranslations(QApplication& app, const QString& configured) {
    QString lang = configured;
    if (lang.isEmpty()) lang = QLocale::system().name();  // e.g. "tr_TR"
    const QString shortCode = lang.section(QLatin1Char('_'), 0, 0);

    // Qt's own strings.
    auto* qtTr = new QTranslator(&app);
    if (qtTr->load(QStringLiteral("qtbase_%1").arg(shortCode),
                   QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(qtTr);

    auto* appTr = new QTranslator(&app);
    const QStringList bases = {QStringLiteral(":/i18n/"),
                               app.applicationDirPath() + QStringLiteral("/i18n/")};
    for (const QString& base : bases) {
        if (appTr->load(QStringLiteral("pacnscanner_%1").arg(lang), base) ||
            appTr->load(QStringLiteral("pacnscanner_%1").arg(shortCode), base)) {
            app.installTranslator(appTr);
            break;
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QString::fromLatin1(kOrgName));
    QApplication::setApplicationName(QString::fromLatin1(kAppName));
    QApplication::setApplicationVersion(QString::fromLatin1(kVersionString));
    QApplication::setWindowIcon(iconprovider::appIcon());

    const QString logDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        QStringLiteral("/logs");
    QDir().mkpath(logDir);
    Logger::instance().init(logDir);
    qInstallMessageHandler(messageHandler);
    Logger::instance().info(QStringLiteral("app"),
                            QStringLiteral("PacNScanner %1 starting")
                                .arg(QString::fromLatin1(kVersionString)));

    QLocalServer singleInstanceServer;
    if (!claimSingleInstance(singleInstanceServer)) {
        Logger::instance().info(QStringLiteral("app"),
                                QStringLiteral("Another instance is running — exiting"));
        return 0;
    }

    // Language preference is read directly from QSettings to avoid pulling the
    // settings store before translators are installed.
    QSettings settings(QString::fromLatin1(kOrgName), QString::fromLatin1(kAppName));
    installTranslations(app, settings.value(QStringLiteral("ui/language")).toString());

    MainWindow window;
    window.show();
    window.startInitialScan();
    return app.exec();
}
