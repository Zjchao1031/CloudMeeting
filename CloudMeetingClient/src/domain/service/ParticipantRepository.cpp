#include "domain/service/ParticipantRepository.h"
#include <algorithm>

void ParticipantRepository::upsertParticipant(const Participant &p)
{
    m_participants[p.userId] = p;
}

void ParticipantRepository::removeParticipant(const QString &userId)
{
    m_participants.remove(userId);
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
