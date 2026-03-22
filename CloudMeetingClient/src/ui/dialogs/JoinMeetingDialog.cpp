#include "ui/dialogs/JoinMeetingDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

JoinMeetingDialog::JoinMeetingDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

QString JoinMeetingDialog::roomId() const
{
    return m_roomIdEdit->text().trimmed();
}

QString JoinMeetingDialog::password() const
{
    return m_password;
}

bool JoinMeetingDialog::promptPassword()
{
    // Inline password sub-dialog
    QDialog dlg(this);
    dlg.setWindowTitle("输入会议密码");
    dlg.setFixedSize(320, 200);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *root = new QVBoxLayout(&dlg);
    root->setContentsMargins(24, 24, 24, 20);
    root->setSpacing(16);

    auto *label = new QLabel("该会议需要密码才能加入", &dlg);
    label->setObjectName("hintLabel");
    label->setAlignment(Qt::AlignCenter);

    auto *edit = new QLineEdit(&dlg);
    edit->setEchoMode(QLineEdit::Password);
    edit->setPlaceholderText("请输入会议密码");
    edit->setMaxLength(16);

    auto *btnRow = new QHBoxLayout;
    auto *cancelBtn = new QPushButton("取消", &dlg);
    cancelBtn->setObjectName("smallOutlineBtn");
    auto *okBtn = new QPushButton("确认", &dlg);
    okBtn->setObjectName("smallBtn");
    btnRow->addStretch();
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(okBtn);

    root->addWidget(label);
    root->addWidget(edit);
    root->addStretch();
    root->addLayout(btnRow);

    QObject::connect(okBtn,     &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        m_password = edit->text();
        return true;
    }
    return false;
}

void JoinMeetingDialog::onJoinClicked()
{
    if (m_roomIdEdit->text().trimmed().isEmpty()) {
        m_roomIdEdit->setPlaceholderText("房间号不能为空！");
        return;
    }
    accept();
}

void JoinMeetingDialog::setupUi()
{
    setWindowTitle("加入会议");
    setFixedSize(380, 260);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(32, 28, 32, 24);
    root->setSpacing(20);

    auto *title = new QLabel("加入会议", this);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    root->addWidget(title);

    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    root->addWidget(sep);

    auto *roomLabel = new QLabel("房间号", this);
    roomLabel->setObjectName("accentLabel");
    root->addWidget(roomLabel);

    m_roomIdEdit = new QLineEdit(this);
    m_roomIdEdit->setPlaceholderText("请输入 6 位房间号");
    m_roomIdEdit->setMaxLength(16);
    root->addWidget(m_roomIdEdit);

    root->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);
    m_cancelBtn = new QPushButton("取消", this);
    m_cancelBtn->setObjectName("smallOutlineBtn");
    m_joinBtn   = new QPushButton("加入", this);
    m_joinBtn->setObjectName("smallBtn");
    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_joinBtn);
    root->addLayout(btnRow);

    connect(m_joinBtn,   &QPushButton::clicked, this, &JoinMeetingDialog::onJoinClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}
