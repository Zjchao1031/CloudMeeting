#pragma once
#include "domain/model/Participant.h"
#include <QHash>
#include <QList>
#include <QString>

class ParticipantRepository
{
public:
    void upsertParticipant(const Participant &p);
    void removeParticipant(const QString &userId);
    QList<Participant> sortedParticipants() const;

private:
    QHash<QString, Participant> m_participants;
};
