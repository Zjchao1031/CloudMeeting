/**
 * @file MeetingController.cpp
 * @brief 实现会议控制业务流程。
 */
#include "domain/service/MeetingController.h"
#include "network/NetworkFacade.h"

MeetingController::MeetingController() {}

void MeetingController::setNetworkFacade(NetworkFacade *facade)
{
    m_network = facade;
}

void MeetingController::createRoom(const CreateRoomOptions &opts)
{
    Q_UNUSED(opts)
}

void MeetingController::joinRoom(const JoinRoomOptions &opts)
{
    Q_UNUSED(opts)
}

void MeetingController::leaveRoom() {}

void MeetingController::onRoomClosed() {}
