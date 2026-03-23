/**
 * @file MeetingController.cpp
 * @brief 实现会议控制业务流程。
 */
#include "domain/service/MeetingController.h"
#include "domain/service/ParticipantRepository.h"
#include "network/NetworkFacade.h"
#include <QJsonObject>
#include <QRandomGenerator>

MeetingController::MeetingController(QObject *parent)
    : QObject(parent)
{}

void MeetingController::setNetworkFacade(NetworkFacade *facade)
{
    m_network = facade;
}

void MeetingController::setParticipantRepository(ParticipantRepository *repo)
{
    m_repo = repo;
}

void MeetingController::createRoom(const CreateRoomOptions &opts)
{
    if (m_state != MeetingState::Idle) {
        emit errorOccurred("操作无效", "当前已在会议中或正在连接。");
        return;
    }

    // 校验参数。
    if (opts.hasPassword && opts.password.length() < 4) {
        emit errorOccurred("参数错误", "会议密码长度不能少于 4 位。");
        return;
    }

    setState(MeetingState::Connecting);

    // 阶段二暂用 Mock：直接模拟创建成功。
    m_currentRoom.roomId      = QString::number(100000 + (QRandomGenerator::global()->bounded(900000)));
    m_currentRoom.maxMembers  = opts.maxMembers;
    m_currentRoom.hasPassword = opts.hasPassword;
    m_currentRoom.hostUserId  = "local_user";
    m_currentRoom.isHost      = true;

    // 将自己添加为参会者。
    if (m_repo) {
        Participant self;
        self.userId   = "local_user";
        self.nickname = opts.nickname;
        self.avatarBase64 = opts.avatarBase64;
        self.isHost   = true;
        m_repo->upsertParticipant(self);
    }

    setState(MeetingState::InMeeting);
    emit meetingEntered(m_currentRoom);
}

void MeetingController::joinRoom(const JoinRoomOptions &opts)
{
    if (m_state != MeetingState::Idle) {
        emit errorOccurred("操作无效", "当前已在会议中或正在连接。");
        return;
    }

    if (opts.roomId.trimmed().isEmpty()) {
        emit errorOccurred("参数错误", "房间号不能为空。");
        return;
    }

    setState(MeetingState::Connecting);

    // 阶段二暂用 Mock：直接模拟加入成功。
    m_currentRoom.roomId      = opts.roomId;
    m_currentRoom.maxMembers  = 10;
    m_currentRoom.hasPassword = false;
    m_currentRoom.hostUserId  = "mock_host";
    m_currentRoom.isHost      = false;

    // 将自己添加为参会者。
    if (m_repo) {
        Participant self;
        self.userId   = "local_user";
        self.nickname = opts.nickname;
        self.avatarBase64 = opts.avatarBase64;
        self.isHost   = false;
        m_repo->upsertParticipant(self);

        // 模拟房间内已有主持人。
        Participant host;
        host.userId   = "mock_host";
        host.nickname = "主持人";
        host.isHost   = true;
        m_repo->upsertParticipant(host);
    }

    setState(MeetingState::InMeeting);
    emit meetingEntered(m_currentRoom);
}

void MeetingController::leaveRoom()
{
    if (m_state != MeetingState::InMeeting) return;

    // 清理参会者数据。
    if (m_repo) {
        m_repo->clearAll();
    }

    m_currentRoom = RoomInfo{};
    setState(MeetingState::Idle);
    emit meetingExited();
}

void MeetingController::onRoomClosed()
{
    if (m_state != MeetingState::InMeeting) return;

    if (m_repo) {
        m_repo->clearAll();
    }

    m_currentRoom = RoomInfo{};
    setState(MeetingState::Idle);
    emit roomClosed();
}

void MeetingController::onMemberJoin(const Participant &p)
{
    if (m_state != MeetingState::InMeeting) return;
    if (m_repo) {
        m_repo->upsertParticipant(p);
    }
}

void MeetingController::onMemberLeave(const QString &userId)
{
    if (m_state != MeetingState::InMeeting) return;
    if (m_repo) {
        m_repo->removeParticipant(userId);
    }
}

void MeetingController::onMediaStateSync(const QString &userId,
                                          bool camera, bool mic, bool screen)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;

    // 获取现有参会者并更新媒体状态。
    auto list = m_repo->sortedParticipants();
    for (auto &p : list) {
        if (p.userId == userId) {
            p.cameraOn      = camera;
            p.micOn         = mic;
            p.screenShareOn = screen;
            m_repo->upsertParticipant(p);
            break;
        }
    }
}

MeetingState MeetingController::state() const { return m_state; }
const RoomInfo &MeetingController::currentRoom() const { return m_currentRoom; }

void MeetingController::setState(MeetingState s)
{
    if (m_state != s) {
        m_state = s;
        emit stateChanged(m_state);
    }
}
