/**
 * @file UserProfileDialog.cpp
 * @brief 实现用户资料编辑对话框。
 */
#include "ui/dialogs/UserProfileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QLabel>
#include <QValidator>

/**
 * @brief 将头像裁剪为圆形图像。
 * @param[in] src 原始头像图像。
 * @param[in] size 输出图像边长尺寸，单位：像素。
 * @return 裁剪后的圆形头像图像。
 */
static QPixmap makeCircularPixmap(const QPixmap &src, int size)
{
    if (src.isNull()) return QPixmap();
    QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath clipPath;
    clipPath.addEllipse(QRectF(0.0, 0.0, size, size));
    p.setClipPath(clipPath);
    int ox = (scaled.width() - size) / 2;
    int oy = (scaled.height() - size) / 2;
    p.drawPixmap(QRect(0, 0, size, size), scaled, QRect(ox, oy, size, size));
    return result;
}

UserProfileDialog::UserProfileDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    m_avatarPixmap = QPixmap(":/avatars/default.jpg");
    m_avatarLabel->setPixmap(makeCircularPixmap(m_avatarPixmap, 80));
}

QString UserProfileDialog::nickname() const
{
    return m_nicknameEdit->text().trimmed();
}

QPixmap UserProfileDialog::avatar() const
{
    return m_avatarPixmap;
}

void UserProfileDialog::setNickname(const QString &name)
{
    m_nicknameEdit->setText(name);
}

void UserProfileDialog::setAvatar(const QPixmap &pix)
{
    m_avatarPixmap = pix;
    m_avatarLabel->setPixmap(makeCircularPixmap(m_avatarPixmap, 80));
}

void UserProfileDialog::onSelectAvatarClicked()
{
    QString path = QFileDialog::getOpenFileName(
        this, "\u9009\u62e9\u5934\u50cf", QString(),
        "\u56fe\u7247\u6587\u4ef6 (*.jpg *.jpeg *.png)");
    if (path.isEmpty()) return;
    QPixmap raw(path);
    if (raw.isNull()) return;
    m_avatarPixmap = raw;
    m_avatarLabel->setPixmap(makeCircularPixmap(raw, 80));
}

void UserProfileDialog::setupUi()
{
    setWindowTitle("\u7528\u6237\u8bbe\u7f6e");
    setFixedSize(360, 320);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 24);
    root->setSpacing(20);

    auto *titleLabel = new QLabel("\u4e2a\u4eba\u4fe1\u606f\u8bbe\u7f6e", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    root->addWidget(titleLabel);

    auto *avatarRow = new QHBoxLayout;
    avatarRow->setSpacing(16);
    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setFixedSize(80, 80);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    m_avatarLabel->setStyleSheet(
        "border: 2px solid #4F8EF7; border-radius: 40px; background: #2A2A3E;");
    avatarRow->addWidget(m_avatarLabel);

    auto *avatarRight = new QVBoxLayout;
    avatarRight->setSpacing(6);
    auto *avatarHint = new QLabel("\u652f\u6301 JPG / PNG\n\u5efa\u8bae\u5c3a\u5bf8 64x64 \u50cf\u7d20", this);
    avatarHint->setObjectName("hintLabel");
    m_avatarBtn = new QPushButton("\u9009\u62e9\u5934\u50cf", this);
    m_avatarBtn->setObjectName("smallBtn");
    m_avatarBtn->setFixedWidth(100);
    connect(m_avatarBtn, &QPushButton::clicked, this, &UserProfileDialog::onSelectAvatarClicked);
    avatarRight->addWidget(avatarHint);
    avatarRight->addWidget(m_avatarBtn);
    avatarRow->addLayout(avatarRight);
    root->addLayout(avatarRow);

    auto *nickRow = new QVBoxLayout;
    nickRow->setSpacing(6);
    auto *nickLabel = new QLabel("\u6635\u79f0", this);
    nickLabel->setObjectName("accentLabel");
    m_nicknameEdit = new QLineEdit(this);
    m_nicknameEdit->setPlaceholderText("\u8bf7\u8f93\u5165\u6635\u79f0\uff08\u6700\u591a 32 \u4e2a\u5b57\u7b26\uff09");
    m_nicknameEdit->setMaxLength(32);
    nickRow->addWidget(nickLabel);
    nickRow->addWidget(m_nicknameEdit);
    root->addLayout(nickRow);

    root->addStretch();

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(12);
    m_cancelBtn = new QPushButton("\u53d6\u6d88", this);
    m_cancelBtn->setObjectName("smallOutlineBtn");
    m_saveBtn = new QPushButton("\u4fdd\u5b58", this);
    m_saveBtn->setObjectName("smallBtn");
    btnRow->addStretch();
    btnRow->addWidget(m_cancelBtn);
    btnRow->addWidget(m_saveBtn);
    root->addLayout(btnRow);

    connect(m_saveBtn,   &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}
