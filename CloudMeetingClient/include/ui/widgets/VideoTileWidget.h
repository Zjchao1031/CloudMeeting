#pragma once
#include <QWidget>
#include <QString>
#include <QImage>

class VideoTileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoTileWidget(QWidget *parent = nullptr);
    void updateFrame(const QImage &frame);
    void setWatermark(const QString &nickname);

private:
    QString m_userId;
    QString m_nickname;
};
