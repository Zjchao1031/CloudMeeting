#pragma once
#include <QWidget>
#include <QString>
#include <QImage>
#include <QShortcut>

class VideoTileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoTileWidget(QWidget *parent = nullptr);

    void updateFrame(const QImage &frame);
    void setWatermark(const QString &nickname);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QString m_userId;
    QString m_nickname;
    QImage  m_frame;
};
