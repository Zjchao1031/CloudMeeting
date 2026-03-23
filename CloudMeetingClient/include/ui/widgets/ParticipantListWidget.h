#pragma once
#include "domain/model/Participant.h"
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

/**
 * @file ParticipantListWidget.h
 * @brief 声明参会者列表组件。
 */

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
     * @brief 根据业务层参会者数据刷新列表。
     * @param[in] participants 已排序的参会者列表。
     */
    void updateFromParticipants(const QList<Participant> &participants);

private:
    /**
     * @brief 根据当前数据重建参会者列表项。
     */
    void buildList();

    /**
     * @brief 创建单个参会者条目控件。
     * @param[in] p 待展示的参会者数据。
     * @return 列表项控件指针。
     */
    QWidget* makeItem(const Participant &p);

    QVBoxLayout      *m_listLayout   = nullptr; ///< 参会者列表布局。
    QList<Participant> m_participants;           ///< 当前参会者展示数据。
};
