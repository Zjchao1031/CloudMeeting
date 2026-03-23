/**
 * @file ParticipantListWidget.cpp
 * @brief 实现参会者列表组件。
 */
#include "ui/widgets/ParticipantListWidget.h"
#include <QScrollArea>
#include <QLabel>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

/**
 * @brief 将 QImage 裁剪为圆形头像。
 * @param[in] src 原始图像。
 * @param[in] size 输出尺寸。
 * @return 圆形头像 QPixmap。
 */
static QPixmap makeCircularAvatar(const QImage &src, int size)
{
    if (src.isNull()) return QPixmap();
    QPixmap scaled = QPixmap::fromImage(src).scaled(
        size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath clip;
    clip.addEllipse(QRectF(0, 0, size, size));
    p.setClipPath(clip);
    int ox = (scaled.width()  - size) / 2;
    int oy = (scaled.height() - size) / 2;
    p.drawPixmap(0, 0, size, size, scaled, ox, oy, size, size);
    return result;
}

/**
 * @brief 生成参会者头像占位图（无头像时使用首字符）。
 * @param[in] initial 头像显示用的首字符文本。
 * @param[in] size 头像边长尺寸，单位：像素。
 * @param[in] isHost 标识是否为主持人。
 * @return 生成的圆形头像图像。
 */
static QPixmap makeAvatarPix(const QString &initial, int size, bool isHost)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QColor bg = isHost ? QColor("#4F8EF7") : QColor("#363650");
    p.setBrush(bg);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, size, size);
    p.setPen(QColor("#FFFFFF"));
    QFont f = p.font();
    f.setPixelSize(size / 2);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter,
               initial.isEmpty() ? "?" : initial.left(1).toUpper());
    return pix;
}

ParticipantListWidget::ParticipantListWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    // 顶部标题区域。
    auto *header = new QLabel("参会人员", this);
    header->setStyleSheet(
        "color: #8888A8; font-size: 12px; font-weight: 600;"
        "padding: 8px 16px; background: #1E1E2E; border-bottom: 1px solid #2E2E44;"
    );
    outerLayout->addWidget(header);

    // 列表滚动区域。
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *container = new QWidget(scroll);
    container->setStyleSheet("background: transparent;");
    m_listLayout = new QVBoxLayout(container);
    m_listLayout->setContentsMargins(0, 4, 0, 4);
    m_listLayout->setSpacing(2);
    m_listLayout->addStretch();

    scroll->setWidget(container);
    outerLayout->addWidget(scroll);
}

void ParticipantListWidget::updateFromParticipants(const QList<Participant> &participants)
{
    m_participants = participants;
    buildList();
}

void ParticipantListWidget::buildList()
{
    // 移除旧列表项，保留末尾弹性项。
    while (m_listLayout->count() > 1) {
        auto *item = m_listLayout->takeAt(0);
        if (item->widget()) delete item->widget();
        delete item;
    }

    for (const auto &p : m_participants) {
        m_listLayout->insertWidget(m_listLayout->count() - 1, makeItem(p));
    }
}

QWidget* ParticipantListWidget::makeItem(const Participant &p)
{
    auto *item = new QWidget(this);
    item->setFixedHeight(52);
    item->setStyleSheet(
        "QWidget { background: transparent; border-radius: 8px; }"
        "QWidget:hover { background: rgba(79,142,247,0.08); }"
    );

    auto *row = new QHBoxLayout(item);
    row->setContentsMargins(12, 6, 12, 6);
    row->setSpacing(10);

    // 头像区域：优先使用真实头像，否则使用首字符占位。
    auto *avatarLabel = new QLabel(item);
    avatarLabel->setFixedSize(36, 36);
    if (!p.avatar.isNull()) {
        avatarLabel->setPixmap(makeCircularAvatar(p.avatar, 36));
    } else {
        avatarLabel->setPixmap(makeAvatarPix(p.nickname, 36, p.isHost));
    }
    row->addWidget(avatarLabel);

    // 昵称与主持人标识区域。
    auto *nameCol = new QVBoxLayout;
    nameCol->setSpacing(2);
    auto *nameRow = new QHBoxLayout;
    nameRow->setSpacing(6);

    auto *nameLabel = new QLabel(p.nickname, item);
    nameLabel->setStyleSheet("color: #E8E8F0; font-size: 13px; font-weight: 500;");
    nameRow->addWidget(nameLabel);

    if (p.isHost) {
        auto *badge = new QLabel("主持人", item);
        badge->setStyleSheet(
            "background: rgba(79,142,247,0.18); color: #4F8EF7;"
            "font-size: 10px; font-weight: 600; border-radius: 4px; padding: 1px 5px;"
        );
        nameRow->addWidget(badge);
    }
    nameRow->addStretch();
    nameCol->addLayout(nameRow);
    row->addLayout(nameCol);

    row->addStretch();

    // 媒体状态图标区域。
    auto *micLabel = new QLabel(item);
    micLabel->setText(p.micOn ? "🎤" : "🔇");
    micLabel->setStyleSheet(p.micOn
        ? "color: #4CAF82; font-size: 14px;"
        : "color: #8888A8; font-size: 14px;");
    micLabel->setToolTip(p.micOn ? "麦克风已开启" : "麦克风已关闭");

    auto *camLabel = new QLabel(item);
    camLabel->setText(p.cameraOn ? "📷" : "📵");
    camLabel->setStyleSheet(p.cameraOn
        ? "color: #4CAF82; font-size: 14px;"
        : "color: #8888A8; font-size: 14px;");
    camLabel->setToolTip(p.cameraOn ? "摄像头已开启" : "摄像头已关闭");

    row->addWidget(micLabel);
    row->addWidget(camLabel);

    return item;
}
