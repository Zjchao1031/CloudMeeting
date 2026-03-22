#pragma once
#include <QWidget>
#include <QString>
#include <QImage>
#include <QShortcut>

/**
 * @file VideoTileWidget.h
 * @brief 声明会议视频画面展示控件。
 */

/**
 * @class VideoTileWidget
 * @brief 展示单个参会者视频画面或占位图的卡片控件。
 */
class VideoTileWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造视频卡片控件。
     * @param[in] parent 父窗口指针。
     */
    explicit VideoTileWidget(QWidget *parent = nullptr);

    /**
     * @brief 更新当前显示的视频帧。
     * @param[in] frame 待显示的视频图像帧。
     */
    void updateFrame(const QImage &frame);

    /**
     * @brief 设置视频卡片上的昵称水印。
     * @param[in] nickname 待显示的参会者昵称。
     */
    void setWatermark(const QString &nickname);

    /**
     * @brief 返回控件建议尺寸。
     * @return 按 16:9 比例计算的建议尺寸。
     */
    QSize sizeHint() const override;

protected:
    /**
     * @brief 处理视频卡片重绘事件。
     * @param[in] event 当前绘制事件对象。
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 处理尺寸变化事件并维持视频比例。
     * @param[in] event 当前尺寸变化事件对象。
     */
    void resizeEvent(QResizeEvent *event) override;

private:
    QString m_userId;   ///< 当前视频所属用户标识。
    QString m_nickname; ///< 当前显示的昵称水印文本。
    QImage  m_frame;    ///< 当前显示的视频图像帧。
};
