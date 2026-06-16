#pragma once

#include <QNetworkAccessManager>
#include <QObject>

namespace pacn {

// Checks GitHub Releases for a newer CalVer build on startup. Failures are
// silent (no nagging); a match emits updateAvailable for an unobtrusive banner.
class UpdateChecker : public QObject {
    Q_OBJECT
public:
    explicit UpdateChecker(QObject* parent = nullptr);

    void checkLatest();

    // Returns true if `candidate` is a strictly newer CalVer than `current`.
    static bool isNewer(const QString& candidate, const QString& current);

signals:
    void updateAvailable(const QString& version, const QString& url);
    void upToDate();
    void checkFailed(const QString& error);

private:
    QNetworkAccessManager nam_;
};

}  // namespace pacn
