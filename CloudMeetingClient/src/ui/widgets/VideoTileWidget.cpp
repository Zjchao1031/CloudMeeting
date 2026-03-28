/**
 * @file VideoTileWidget.cpp
 * @brief 实现会议视频画面展示控件。
 */
#include "ui/widgets/VideoTileWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

VideoTileWidget::VideoTileWidget(QWidget *parent)
    : QWidget(parent)
{
    // 启用 heightForWidth 模式，布局系统将自动按 16:9 分配高度，
    // 无需在 resizeEvent 中调用 setFixedHeight（会引发递归布局循环导致崩溃）。
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setMinimumHeight(60);
}

QSize VideoTileWidget::sizeHint() const
{
    int w = width() > 0 ? width() : 320;
    return QSize(w, heightForWidth(w));
}

bool VideoTileWidget::hasHeightForWidth() const
{
    return true;
}

int VideoTileWidget::heightForWidth(int w) const
{
    return qMax(w * 9 / 16, 60);
}

void VideoTileWidget::updateFrame(const QImage &frame)
{
    m_frame = frame;
    update();
}

void VideoTileWidget::setWatermark(const QString &nickname)
{
    m_nickname = nickname;
    update();
}

void VideoTileWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRect r = rect();
    // 尺寸过小时跳过绘制，避免在窗口过渡状态下产生无效渲染。
    if (r.width() < 4 || r.height() < 4) return;
    int radius = 12;

    // 设置圆角裁剪区域。
    QPainterPath clip;
    clip.addRoundedRect(r, radius, radius);
    p.setClipPath(clip);

    // 绘制背景色。
    p.fillRect(r, QColor("#1A1A28"));

    if (!m_frame.isNull()) {
        // 绘制按填充模式缩放的视频帧。
        QImage scaled = m_frame.scaled(r.size(), Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);
        int ox = (scaled.width()  - r.width())  / 2;
        int oy = (scaled.height() - r.height()) / 2;
        p.drawImage(-ox, -oy, scaled);
    } else {
        // 绘制无视频时的人像占位图。
        p.setBrush(QColor("#2A2A3E"));
        p.setPen(Qt::NoPen);
        p.fillRect(r, QColor("#1A1A28"));

        // 绘制头像圆形占位区域。
        int avatarSize = qMin(r.width(), r.height()) / 3;
        QRect avatarRect(
            r.center().x() - avatarSize / 2,
            r.center().y() - avatarSize / 2 - 10,
            avatarSize, avatarSize
        );
        p.setBrush(QColor("#2E2E44"));
        p.drawEllipse(avatarRect);

        // 绘制人像轮廓。
        p.setBrush(QColor("#4A4A68"));
        int headR = avatarSize / 3;
        p.drawEllipse(avatarRect.center().x() - headR / 2,
                      avatarRect.top() + avatarSize / 6,
                      headR, headR);
    }

    // 绘制边框。
    p.setClipping(false);
    QPen borderPen(QColor("#3A3A52"), 1);
    p.setPen(borderPen);
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r.adjusted(0, 0, -1, -1), radius, radius);

    // 在左下角绘制昵称水印条。
    if (!m_nickname.isEmpty()) {
        QFont font = p.font();
        font.setPixelSize(13);
        p.setFont(font);
        QFontMetrics fm(font);
        int textW = fm.horizontalAdvance(m_nickname) + 16;
        int barH  = 26;
        QRect barRect(8, r.height() - barH - 8, textW, barH);

        // 绘制半透明背景。
        p.setClipPath(clip);
        p.setBrush(QColor(0, 0, 0, 140));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(barRect, 4, 4);

        p.setPen(QColor("#E8E8F0"));
        p.drawText(barRect, Qt::AlignCenter, m_nickname);
    }
}
