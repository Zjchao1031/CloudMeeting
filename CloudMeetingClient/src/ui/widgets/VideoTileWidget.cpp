#include "ui/widgets/VideoTileWidget.h"

VideoTileWidget::VideoTileWidget(QWidget *parent)
    : QWidget(parent)
{}

void VideoTileWidget::updateFrame(const QImage &frame)
{
    Q_UNUSED(frame)
}

void VideoTileWidget::setWatermark(const QString &nickname)
{
    m_nickname = nickname;
}
