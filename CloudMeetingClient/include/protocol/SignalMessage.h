#pragma once
#include "protocol/SignalType.h"
#include <QJsonObject>

struct SignalMessage
{
    SignalType  type;
    QJsonObject payload;
};
