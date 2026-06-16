#include "ui/AboutDialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

#include <pacn/version.h>

#include "ui/IconProvider.h"

namespace pacn {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("About PacNScanner"));
    setModal(true);
    resize(520, 560);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(14);

    auto* logo = new QLabel;
    logo->setPixmap(iconprovider::logoPixmap(96));
    logo->setAlignment(Qt::AlignCenter);
    root->addWidget(logo);

    auto* title = new QLabel(
        QStringLiteral("<div align='center'><span style='font-size:22px;font-weight:700'>"
                       "PacNScanner</span><br><span style='color:#7b8794'>%1 · v%2</span></div>")
            .arg(tr("Fast, modern local network scanner"),
                 QString::fromLatin1(kVersionString)));
    title->setTextFormat(Qt::RichText);
    root->addWidget(title);

    auto* desc = new QLabel(
        tr("PacNScanner discovers devices on your local network, inventories their open "
           "ports and services, and highlights basic security risks — with a fast, "
           "system-themed interface for home users, sysadmins and security professionals."));
    desc->setWordWrap(true);
    desc->setAlignment(Qt::AlignCenter);
    root->addWidget(desc);

    auto* ethics = new QLabel(
        tr("<b>Ethical use:</b> Use PacNScanner only on networks you own or are explicitly "
           "authorized to assess. Scanning networks without permission may be illegal in "
           "your jurisdiction."));
    ethics->setWordWrap(true);
    ethics->setStyleSheet(QStringLiteral(
        "background:#fff7ed;color:#7c2d12;border:1px solid #fed7aa;border-radius:8px;padding:10px;"));
    root->addWidget(ethics);

    auto* privacy = new QLabel(
        tr("<b>Privacy:</b> No telemetry or analytics are collected. All scan data stays on "
           "your device by default."));
    privacy->setWordWrap(true);
    root->addWidget(privacy);

    auto* links = new QLabel(
        QStringLiteral(
            "<div align='center'>"
            "<a href='https://github.com/%1'>%2</a> &nbsp;·&nbsp; "
            "<a href='https://github.com/%1/blob/main/docs/PRIVACY.md'>%3</a> &nbsp;·&nbsp; "
            "<a href='https://github.com/%1/blob/main/LICENSE'>%4</a></div>")
            .arg(QString::fromLatin1(kGitHubRepo), tr("Project"), tr("Privacy Policy"),
                 tr("MIT License")));
    links->setOpenExternalLinks(true);
    links->setTextFormat(Qt::RichText);
    root->addWidget(links);

    auto* copyright = new QLabel(
        QStringLiteral("<div align='center' style='color:#7b8794'>© 2026 PacNScanner — MIT "
                       "License</div>"));
    copyright->setTextFormat(Qt::RichText);
    root->addWidget(copyright);

    root->addStretch(1);
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    root->addWidget(buttons);
}

}  // namespace pacn
