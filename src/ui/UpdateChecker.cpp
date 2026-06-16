#include "ui/UpdateChecker.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <pacn/version.h>

namespace pacn {

UpdateChecker::UpdateChecker(QObject* parent) : QObject(parent) {}

bool UpdateChecker::isNewer(const QString& candidateRaw, const QString& currentRaw) {
    const auto parts = [](QString v) {
        v.remove(QLatin1Char('v'));
        const QStringList s = v.split(QLatin1Char('.'), Qt::SkipEmptyParts);
        QList<int> out;
        for (const QString& p : s) out << p.toInt();
        while (out.size() < 3) out << 0;
        return out;
    };
    const QList<int> c = parts(candidateRaw);
    const QList<int> cur = parts(currentRaw);
    for (int i = 0; i < 3; ++i) {
        if (c[i] > cur[i]) return true;
        if (c[i] < cur[i]) return false;
    }
    return false;
}

void UpdateChecker::checkLatest() {
    const QString url =
        QStringLiteral("https://api.github.com/repos/%1/releases/latest")
            .arg(QString::fromLatin1(kGitHubRepo));
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("PacNScanner"));
    req.setRawHeader("Accept", "application/vnd.github+json");

    QNetworkReply* reply = nam_.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit checkFailed(reply->errorString());
            return;
        }
        const QJsonObject obj =
            QJsonDocument::fromJson(reply->readAll()).object();
        const QString tag = obj.value(QStringLiteral("tag_name")).toString();
        const QString htmlUrl = obj.value(QStringLiteral("html_url")).toString();
        if (tag.isEmpty()) {
            emit checkFailed(QStringLiteral("no release tag"));
            return;
        }
        if (isNewer(tag, QString::fromLatin1(kVersionString)))
            emit updateAvailable(tag, htmlUrl);
        else
            emit upToDate();
    });
}

}  // namespace pacn
