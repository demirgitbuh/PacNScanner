#include "ui/DeviceDetailDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

#include "ui/IconProvider.h"

namespace pacn {

DeviceDetailDialog::DeviceDetailDialog(const Device& device, QWidget* parent)
    : QDialog(parent), device_(device) {
    setWindowTitle(tr("Device — %1").arg(device.ipString()));
    setModal(true);
    resize(560, 640);
    buildUi();
}

void DeviceDetailDialog::buildUi() {
    auto* root = new QVBoxLayout(this);

    // Header
    auto* header = new QHBoxLayout;
    auto* dot = new QLabel;
    dot->setPixmap(iconprovider::statusIcon(device_.status).pixmap(20, 20));
    auto* title = new QLabel(QStringLiteral("<b style='font-size:16px'>%1</b><br><span "
                                            "style='color:#7b8794'>%2 · %3</span>")
                                 .arg(device_.ipString(),
                                      device_.hostname.isEmpty() ? tr("(no hostname)")
                                                                 : device_.hostname,
                                      toString(device_.type)));
    header->addWidget(dot);
    header->addWidget(title, 1);
    auto* fav = new QCheckBox(tr("Favorite"));
    fav->setChecked(device_.favorite);
    connect(fav, &QCheckBox::toggled, this, [this](bool on) {
        device_.favorite = on;
        emit favoriteChanged(device_.ipString(), on);
    });
    header->addWidget(fav);
    root->addLayout(header);

    // Properties
    auto* propsBox = new QGroupBox(tr("Properties"));
    auto* form = new QFormLayout(propsBox);
    const auto val = [](const QString& s) {
        auto* l = new QLabel(s.isEmpty() ? QStringLiteral("—") : s);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse);
        return l;
    };
    form->addRow(tr("MAC"), val(device_.mac));
    form->addRow(tr("Vendor"), val(device_.vendor));
    form->addRow(tr("OS guess"), val(device_.osGuess));
    form->addRow(tr("Status"), val(toString(device_.status)));
    form->addRow(tr("RTT"), val(device_.rttMs >= 0 ? QStringLiteral("%1 ms").arg(device_.rttMs)
                                                    : QString()));
    form->addRow(tr("TTL"), val(device_.ttl >= 0 ? QString::number(device_.ttl) : QString()));
    form->addRow(tr("Adapter"), val(device_.adapter));
    form->addRow(tr("Discovery"), val(device_.discoveryMethod));
    form->addRow(tr("First seen"),
                 val(device_.firstSeen.toString(Qt::ISODate)));
    form->addRow(tr("Last seen"), val(device_.lastSeen.toString(Qt::ISODate)));
    form->addRow(tr("Risk"),
                 val(QStringLiteral("%1 / 100 · %2")
                         .arg(device_.riskScore)
                         .arg(toString(device_.riskLevel))));
    root->addWidget(propsBox);

    // Ports
    auto* portBox = new QGroupBox(tr("Open ports & services"));
    auto* portLayout = new QVBoxLayout(portBox);
    auto* table = new QTableWidget(device_.openPorts().size(), 4, portBox);
    table->setHorizontalHeaderLabels({tr("Port"), tr("Proto"), tr("Service"), tr("Banner")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    int r = 0;
    for (const Port& p : device_.openPorts()) {
        table->setItem(r, 0, new QTableWidgetItem(QString::number(p.number)));
        table->setItem(r, 1, new QTableWidgetItem(p.protocol));
        table->setItem(r, 2, new QTableWidgetItem(p.service));
        table->setItem(r, 3, new QTableWidgetItem(p.banner));
        ++r;
    }
    portLayout->addWidget(table);
    root->addWidget(portBox, 1);

    // Risks
    if (!device_.risks.isEmpty()) {
        auto* riskBox = new QGroupBox(tr("Risk findings"));
        auto* riskLayout = new QVBoxLayout(riskBox);
        for (const RiskFinding& f : device_.risks) {
            auto* l = new QLabel(QStringLiteral("<b>[%1]</b> %2 — <span style='color:#7b8794'>%3</span>")
                                     .arg(toString(f.level), f.title, f.detail));
            l->setWordWrap(true);
            riskLayout->addWidget(l);
        }
        root->addWidget(riskBox);
    }

    // Labels
    auto* labelBox = new QGroupBox(tr("Labels"));
    auto* labelLayout = new QVBoxLayout(labelBox);
    labelList_ = new QListWidget(labelBox);
    labelList_->setMaximumHeight(90);
    for (const QString& l : device_.labels) labelList_->addItem(l);
    auto* labelRow = new QHBoxLayout;
    labelInput_ = new QLineEdit(labelBox);
    labelInput_->setPlaceholderText(tr("Add a label and press Enter"));
    auto* addBtn = new QPushButton(tr("Add"), labelBox);
    labelRow->addWidget(labelInput_, 1);
    labelRow->addWidget(addBtn);
    labelLayout->addWidget(labelList_);
    labelLayout->addLayout(labelRow);
    connect(addBtn, &QPushButton::clicked, this, &DeviceDetailDialog::addLabelFromInput);
    connect(labelInput_, &QLineEdit::returnPressed, this,
            &DeviceDetailDialog::addLabelFromInput);
    connect(labelList_, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem* item) {
                emit labelRemoved(device_.ipString(), item->text());
                delete labelList_->takeItem(labelList_->row(item));
            });
    root->addWidget(labelBox);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    root->addWidget(buttons);
}

void DeviceDetailDialog::addLabelFromInput() {
    const QString text = labelInput_->text().trimmed();
    if (text.isEmpty()) return;
    labelList_->addItem(text);
    emit labelAdded(device_.ipString(), text);
    labelInput_->clear();
}

}  // namespace pacn
