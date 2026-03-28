/**
 * @file ParticipantRepository.cpp
 * @brief 实现参会者数据仓库。
 */
#include "domain/service/ParticipantRepository.h"
#include <algorithm>
#include <QByteArray>

ParticipantRepository::ParticipantRepository(QObject *parent)
    : QObject(parent)
{}

void ParticipantRepository::upsertParticipant(const Participant &p)
{
    Participant stored = p;
    if (stored.avatar.isNull() && !stored.avatarBase64.isEmpty()) {
        QByteArray raw = QByteArray::fromBase64(stored.avatarBase64.toUtf8());
        stored.avatar.loadFromData(raw, "PNG");
    }
    m_participants[p.userId] = stored;
    emit participantsChanged();
}

void ParticipantRepository::removeParticipant(const QString &userId)
{
    if (m_participants.remove(userId)) {
        emit participantsChanged();
    }
}

QList<Participant> ParticipantRepository::sortedParticipants() const
{
    QList<Participant> list = m_participants.values();
    // 主持人置顶，其余按昵称升序
    std::sort(list.begin(), list.end(), [](const Participant &a, const Participant &b) {
        if (a.isHost != b.isHost) return a.isHost > b.isHost;
        return a.nickname < b.nickname;
    });
    return list;
}

void ParticipantRepository::clearAll()
{
    if (!m_participants.isEmpty()) {
        m_participants.clear();
        emit participantsChanged();
    }
}
