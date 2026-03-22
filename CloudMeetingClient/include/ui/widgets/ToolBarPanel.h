#pragma once
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

/**
 * @file ToolBarPanel.h
 * @brief 声明会议底部工具栏组件。
 */

/**
 * @class ToolBarPanel
 * @brief 提供摄像头、麦克风、屏幕共享和音量控制操作入口。
 */
class ToolBarPanel : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造会议工具栏组件。
     * @param[in] parent 父窗口指针。
     */
    explicit ToolBarPanel(QWidget *parent = nullptr);

signals:
    /**
     * @brief 当摄像头状态切换时发出该信号。
     * @param[in] on 当前摄像头开关状态。
     */
    void cameraToggled(bool on);

    /**
     * @brief 当麦克风状态切换时发出该信号。
     * @param[in] on 当前麦克风开关状态。
     */
    void micToggled(bool on);

    /**
     * @brief 当屏幕共享状态切换时发出该信号。
     * @param[in] on 当前屏幕共享开关状态。
     */
    void screenShareToggled(bool on);

    /**
     * @brief 当采集音量变化时发出该信号。
     * @param[in] value 当前采集音量值。
     */
    void captureVolumeChanged(int value);

    /**
     * @brief 当播放音量变化时发出该信号。
     * @param[in] value 当前播放音量值。
     */
    void playbackVolumeChanged(int value);

    /**
     * @brief 当用户请求离开会议时发出该信号。
     */
    void leaveRequested();

private slots:
    /**
     * @brief 处理摄像头按钮点击事件。
     */
    void onCameraClicked();

    /**
     * @brief 处理麦克风按钮点击事件。
     */
    void onMicClicked();

    /**
     * @brief 处理屏幕共享按钮点击事件。
     */
    void onScreenShareClicked();

private:
    /**
     * @brief 初始化工具栏界面。
     */
    void setupUi();

    /**
     * @brief 根据启用状态刷新按钮样式。
     * @param[in] btn 待更新样式的按钮。
     * @param[in] active 当前按钮对应功能是否处于激活状态。
     */
    void updateButtonStyle(QPushButton *btn, bool active);

    QPushButton *m_cameraBtn      = nullptr; ///< 摄像头开关按钮。
    QPushButton *m_micBtn         = nullptr; ///< 麦克风开关按钮。
    QPushButton *m_screenShareBtn = nullptr; ///< 屏幕共享开关按钮。
    QPushButton *m_leaveBtn       = nullptr; ///< 离开会议按钮。
    QSlider     *m_captureSlider  = nullptr; ///< 采集音量滑块。
    QSlider     *m_playbackSlider = nullptr; ///< 播放音量滑块。
    bool         m_cameraOn       = false;   ///< 摄像头当前开关状态。
    bool         m_micOn          = false;   ///< 麦克风当前开关状态。
    bool         m_screenShareOn  = false;   ///< 屏幕共享当前开关状态。
};
