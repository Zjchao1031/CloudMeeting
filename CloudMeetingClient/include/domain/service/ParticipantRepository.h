#pragma once
#include "domain/model/Participant.h"
#include <QObject>
#include <QHash>
#include <QList>
#include <QString>

/**
 * @file ParticipantRepository.h
 * @brief 声明参会者数据仓库。
 */

/**
 * @class ParticipantRepository
 * @brief 管理会议参与者的增删改查与排序输出。
 */
class ParticipantRepository : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造参会者仓库。
     * @param[in] parent 父对象指针。
     */
    explicit ParticipantRepository(QObject *parent = nullptr);

    /**
     * @brief 新增或更新参会者信息。
     * @param[in] p 待写入的参会者对象。
     */
    void upsertParticipant(const Participant &p);

    /**
     * @brief 按用户标识移除参会者。
     * @param[in] userId 待移除参会者的用户标识。
     */
    void removeParticipant(const QString &userId);

    /**
     * @brief 获取排序后的参会者列表。
     * @return 按主持人优先、昵称升序排列的参会者列表。
     */
    QList<Participant> sortedParticipants() const;

    /**
     * @brief 清空所有参会者数据。
     */
    void clearAll();

signals:
    /**
     * @brief 参会者列表发生变更时发出该信号。
     */
    void participantsChanged();

private:
    QHash<QString, Participant> m_participants; ///< 以用户标识为键的参会者缓存。
};
