/**
 * @file ConfirmDialog.cpp
 * @brief 实现通用确认、提示与错误弹窗。
 */
#include "ui/dialogs/ConfirmDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QShortcut>
#include <QKeySequence>

ConfirmDialog::ConfirmDialog(const QString &title,
                             const QString &message,
                             Type type,
                             QWidget *parent)
    : QDialog(parent)
{
    setupUi(title, message, type);
}

void ConfirmDialog::showInfo(const QString &title, const QString &message, QWidget *parent)
{
    ConfirmDialog dlg(title, message, Type::Info, parent);
    dlg.exec();
}

void ConfirmDialog::showError(const QString &title, const QString &message, QWidget *parent)
{
    ConfirmDialog dlg(title, message, Type::Error, parent);
    dlg.exec();
}

bool ConfirmDialog::showConfirm(const QString &title, const QString &message, QWidget *parent)
{
    ConfirmDialog dlg(title, message, Type::Confirm, parent);
    return dlg.exec() == QDialog::Accepted;
}

bool ConfirmDialog::showDanger(const QString &title, const QString &message,
                               const QString &confirmText, QWidget *parent)
{
    ConfirmDialog dlg(title, message, Type::Danger, parent);
    if (auto *btn = dlg.findChild<QPushButton*>("confirmActionBtn")) {
        btn->setText(confirmText);
    }
    return dlg.exec() == QDialog::Accepted;
}

void ConfirmDialog::setupUi(const QString &title, const QString &message,
                            Type type, const QString &)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedWidth(400);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 24);
    root->setSpacing(16);

    // 图标与标题区域。
    QString icon;
    switch (type) {
        case Type::Danger: icon = "[!]  "; break;
        case Type::Error:  icon = "[x]  "; break;
        case Type::Info:   icon = "[i]  "; break;
        default:           icon = "";      break;
    }

    auto *titleLabel = new QLabel(icon + title, this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setWordWrap(true);
    root->addWidget(titleLabel);

    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    root->addWidget(sep);

    auto *msgLabel = new QLabel(message, this);
    msgLabel->setWordWrap(true);
    msgLabel->setStyleSheet("color: #C8C8D8; font-size: 14px;");
    root->addWidget(msgLabel);

    root->addSpacing(8);

    // 操作按钮区域。
    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);
    btnRow->addStretch();

    bool isPureInfo = (type == Type::Info || type == Type::Error);

    if (!isPureInfo) {
        auto *cancelBtn = new QPushButton("\u53d6\u6d88", this);
        cancelBtn->setObjectName("smallOutlineBtn");
        connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
        btnRow->addWidget(cancelBtn);
    }

    auto *confirmBtn = new QPushButton("\u786e\u5b9a", this);
    if (type == Type::Danger) {
        confirmBtn->setObjectName("confirmActionBtn");
        confirmBtn->setStyleSheet(
            "QPushButton { background-color: #E05C5C; color: #FFF; border: none;"
            "  border-radius: 6px; padding: 6px 20px; font-size: 13px; font-weight: 600; min-height: 32px; }"
            "QPushButton:hover { background-color: #F07070; }"
        );
    } else {
        confirmBtn->setObjectName("smallBtn");
    }
    connect(confirmBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(confirmBtn);

    root->addLayout(btnRow);

    // 为非危险弹窗启用 Esc 快捷关闭。
    if (type != Type::Danger) {
        auto *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
        connect(esc, &QShortcut::activated, this, &QDialog::reject);
    }

    adjustSize();
}
