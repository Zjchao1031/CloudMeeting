#include "ui/dialogs/CreateMeetingDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFrame>

CreateMeetingDialog::CreateMeetingDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

int CreateMeetingDialog::maxMembers() const
{
    int val = m_maxMembersEdit->text().toInt();
    return qBound(2, val, 100);
}

bool CreateMeetingDialog::hasPassword() const
{
    return m_passwordToggle->isChecked();
}

QString CreateMeetingDialog::password() const
{
    return m_passwordEdit->text();
}

void CreateMeetingDialog::onPasswordToggleChanged(int state)
{
    bool on = (state == Qt::Checked);
    m_passwordLabel->setVisible(on);
    m_passwordEdit->setVisible(on);
    if (!on) m_passwordEdit->clear();
    // 动态调整对话框高度以适应内容
    adjustSize();
}

void CreateMeetingDialog::setupUi()
{
    setWindowTitle("\u521b\u5efa\u4f1a\u8bae");
    setFixedWidth(400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(12);

    auto *title = new QLabel("\u521b\u5efa\u65b0\u4f1a\u8bae", this);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    root->addWidget(sep);

    auto *membersLabel = new QLabel("\u6700\u5927\u53c2\u4f1a\u4eba\u6570", this);
    membersLabel->setObjectName("accentLabel");
    m_maxMembersEdit = new QLineEdit(this);
    m_maxMembersEdit->setValidator(new QIntValidator(2, 100, this));
    m_maxMembersEdit->setText("10");
    m_maxMembersEdit->setPlaceholderText("2~100");
    root->addWidget(membersLabel);
    root->addWidget(m_maxMembersEdit);

    m_passwordToggle = new QCheckBox("\u542f\u7528\u4f1a\u8bae\u5bc6\u7801", this);
    m_passwordToggle->setStyleSheet(
        "QCheckBox { color: #E8E8F0; font-size: 14px; spacing: 8px; }"
        "QCheckBox::indicator { width: 20px; height: 20px; border-radius: 4px;"
        "  border: 2px solid #3A3A52; background: #2A2A3E; }"
        "QCheckBox::indicator:checked { background: #4F8EF7; border-color: #4F8EF7; }"
    );
    root->addWidget(m_passwordToggle);

    m_passwordLabel = new QLabel("\u4f1a\u8bae\u5bc6\u7801\uff084~16 \u4f4d\uff09", this);
    m_passwordLabel->setObjectName("accentLabel");
    m_passwordLabel->setVisible(false);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("\u8bf7\u8f93\u5165\u5bc6\u7801");
    m_passwordEdit->setMaxLength(16);
    m_passwordEdit->setVisible(false);
    root->addWidget(m_passwordLabel);
    root->addWidget(m_passwordEdit);

    auto *hintBox = new QWidget(this);
    hintBox->setStyleSheet("background: #1E1E2E; border-radius: 8px;");
    auto *hintLayout = new QVBoxLayout(hintBox);
    hintLayout->setContentsMargins(12, 10, 12, 10);
    hintLayout->setSpacing(4);
    auto *hintTitle = new QLabel("\u8fdb\u5165\u4f1a\u8bae\u65f6\u7684\u9ed8\u8ba4\u72b6\u6001", this);
    hintTitle->setObjectName("hintLabel");
    auto *micHint = new QLabel("  \u9ea6\u514b\u98ce\uff1a\u5173\u95ed", this);
    auto *camHint = new QLabel("  \u6444\u50cf\u5934\uff1a\u5173\u95ed", this);
    auto *scrHint = new QLabel("  \u5c4f\u5e55\u5171\u4eab\uff1a\u5173\u95ed", this);
    for (auto *lbl : {micHint, camHint, scrHint}) {
        lbl->setObjectName("statusLabel");
    }
    hintLayout->addWidget(hintTitle);
    hintLayout->addWidget(micHint);
    hintLayout->addWidget(camHint);
    hintLayout->addWidget(scrHint);
    root->addWidget(hintBox);

    root->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);
    m_cancelBtn  = new QPushButton("\u53d6\u6d88", this);
    m_cancelBtn->setObjectName("smallOutlineBtn");
    m_confirmBtn = new QPushButton("\u786e\u8ba4\u521b\u5efa", this);
    m_confirmBtn->setObjectName("smallBtn");
    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_confirmBtn);
    root->addLayout(btnRow);

    connect(m_passwordToggle, &QCheckBox::checkStateChanged,
            this, [this](Qt::CheckState state) {
                onPasswordToggleChanged(static_cast<int>(state));
            });
    connect(m_confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn,  &QPushButton::clicked, this, &QDialog::reject);
}
