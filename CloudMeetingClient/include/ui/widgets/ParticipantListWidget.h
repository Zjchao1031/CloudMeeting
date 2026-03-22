#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

/**
 * @file ParticipantListWidget.h
 * @brief 声明参会者列表组件。
 */

/**
 * @class ParticipantInfo
 * @brief 描述参会者列表中的展示信息。
 */
class ParticipantInfo {
public:
    QString userId;          ///< 参会者用户标识。
    QString nickname;        ///< 参会者显示昵称。
    bool    isHost   = false; ///< 是否为主持人。
    bool    cameraOn = false; ///< 摄像头是否开启。
    bool    micOn    = false; ///< 麦克风是否开启。
};

/**
 * @class ParticipantListWidget
 * @brief 展示会议参会者列表及其媒体状态。
 */
class ParticipantListWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造参会者列表组件。
     * @param[in] parent 父窗口指针。
     */
    explicit ParticipantListWidget(QWidget *parent = nullptr);

    /**
     * @brief 刷新参会者列表显示。
     */
    void refresh();

    /**
     * @brief 加载用于界面演示的模拟参会者数据。
     */
    void loadMockData();

private:
    /**
     * @brief 根据当前数据重建参会者列表项。
     */
    void buildList();

    /**
     * @brief 创建单个参会者条目控件。
     * @param[in] info 待展示的参会者信息。
     * @return 列表项控件指针。
     */
    QWidget* makeItem(const ParticipantInfo &info);

    QVBoxLayout            *m_listLayout   = nullptr; ///< 参会者列表布局。
    QList<ParticipantInfo>  m_participants;           ///< 当前参会者展示数据。
};
