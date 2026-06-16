#include "ui/StatCard.h"

#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

namespace pacn {

StatCard::StatCard(const QString& caption, QWidget* parent) : QFrame(parent) {
    setObjectName(QStringLiteral("statCard"));
    setFrameShape(QFrame::NoFrame);

    value_ = new QLabel(QStringLiteral("0"), this);
    value_->setObjectName(QStringLiteral("statValue"));
    caption_ = new QLabel(caption, this);
    caption_->setObjectName(QStringLiteral("statLabel"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(2);
    layout->addWidget(value_);
    layout->addWidget(caption_);

    setAccessibleName(caption);
}

void StatCard::setValue(const QString& value) {
    value_->setText(value);
    setAccessibleDescription(value);
}

void StatCard::setValue(int value) { setValue(QString::number(value)); }

void StatCard::setAccent(bool accent) {
    setProperty("accent", accent);
    style()->unpolish(this);
    style()->polish(this);
}

}  // namespace pacn
