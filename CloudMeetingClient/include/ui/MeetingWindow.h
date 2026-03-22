#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSplitter>

class ParticipantListWidget;
class ChatPanel;
class VideoTileWidget;
class ToolBarPanel;

/**
 * @file MeetingWindow.h
 * @brief 声明会议进行中的主界面窗口。
 */

/**
 * @class MeetingWindow
 * @brief 展示视频区、参会者列表、聊天区和会议工具栏。
 */
class MeetingWindow : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造会议窗口。
     * @param[in] parent 父窗口指针。
     */
    explicit MeetingWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构会议窗口。
     */
    ~MeetingWindow();

    /**
     * @brief 绑定会议窗口内部组件的信号与槽。
     */
    void bindSignals();

    /**
     * @brief 刷新会议窗口中的动态布局内容。
     */
    void refreshLayout();

    /**
     * @brief 弹出会议已关闭提示并触发离开流程。
     */
    void showRoomClosedDialog();

signals:
    /**
     * @brief 当用户确认离开会议时发出该信号。
     */
    void leaveRequested();

private:
    /**
     * @brief 初始化会议窗口界面。
     */
    void setupUi();

    /**
     * @brief 创建会议窗口顶部栏。
     * @return 顶部栏控件指针。
     */
    QWidget* makeTopBar();

    /**
     * @brief 创建会议视频区域。
     * @return 视频区域控件指针。
     */
    QWidget* makeVideoArea();

    QLabel                *m_titleLabel      = nullptr; ///< 顶部栏会议标题标签。
    QLabel                *m_roomIdLabel     = nullptr; ///< 顶部栏房间号标签。
    QPushButton           *m_copyBtn         = nullptr; ///< 复制房间号按钮。
    QScrollArea           *m_videoScrollArea = nullptr; ///< 视频区域滚动容器。
    QVBoxLayout           *m_videoLayout     = nullptr; ///< 视频区域布局管理器。
    ParticipantListWidget *m_participantList = nullptr; ///< 参会者列表组件。
    ChatPanel             *m_chatPanel       = nullptr; ///< 聊天面板组件。
    ToolBarPanel          *m_toolBar         = nullptr; ///< 底部工具栏组件。
};
