/**
 * @file JoinMeetingDialog.cpp
 * @brief 实现加入会议对话框。
 */
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
    return m_passwordEdit->text();
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
    setFixedSize(380, 360);
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

    auto *pwdLabel = new QLabel("密码（选填）", this);
    pwdLabel->setObjectName("accentLabel");
    root->addWidget(pwdLabel);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("无密码房间可留空");
    m_passwordEdit->setMaxLength(16);
    root->addWidget(m_passwordEdit);

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
