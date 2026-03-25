/**
 * @file MeetingController.cpp
 * @brief 实现会议控制业务流程。
 */
#include "domain/service/MeetingController.h"
#include "domain/service/ParticipantRepository.h"
#include "network/NetworkFacade.h"
#include "common/Constants.h"
#include <QJsonObject>

MeetingController::MeetingController(QObject *parent)
    : QObject(parent)
{}

void MeetingController::setNetworkFacade(NetworkFacade *facade)
{
    m_network = facade;
    if (!m_network) return;

    // 连接所有 NetworkFacade 信号到本控制器对应槽。
    connect(m_network, &NetworkFacade::serverConnected,
            this,      &MeetingController::onServerConnected);
    connect(m_network, &NetworkFacade::serverReconnectFailed,
            this,      &MeetingController::onServerReconnectFailed);
    connect(m_network, &NetworkFacade::serverConnectTimeout,
            this,      &MeetingController::onServerConnectTimeout);
    connect(m_network, &NetworkFacade::createRoomAck,
            this,      &MeetingController::onCreateRoomAck);
    connect(m_network, &NetworkFacade::joinRoomAck,
            this,      &MeetingController::onJoinRoomAck);
    connect(m_network, &NetworkFacade::memberJoined,
            this,      &MeetingController::onMemberJoinFromNetwork);
    connect(m_network, &NetworkFacade::memberLeft,
            this,      &MeetingController::onMemberLeaveFromNetwork);
    connect(m_network, &NetworkFacade::mediaStateSynced,
            this,      &MeetingController::onMediaStateSyncFromNetwork);
    connect(m_network, &NetworkFacade::roomClosed,
            this,      &MeetingController::onRoomClosedFromNetwork);
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
    if (opts.hasPassword && opts.password.length() < 4) {
        emit errorOccurred("参数错误", "会议密码长度不能少于 4 位。");
        return;
    }

    m_pendingAction = 1;
    m_pendingCreate = opts;
    setState(MeetingState::Connecting);

    // 发起 TCP 连接；连接成功后由 onServerConnected 发送 CREATE_ROOM 请求。
    if (m_network) {
        m_network->connectToServer(QLatin1String(Constants::DEFAULT_SERVER_HOST),
                                   Constants::TCP_SIGNAL_PORT);
    }
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

    m_pendingAction = 2;
    m_pendingJoin   = opts;
    setState(MeetingState::Connecting);

    // 发起 TCP 连接；连接成功后由 onServerConnected 发送 JOIN_ROOM 请求。
    if (m_network) {
        m_network->connectToServer(QLatin1String(Constants::DEFAULT_SERVER_HOST),
                                   Constants::TCP_SIGNAL_PORT);
    }
}

void MeetingController::leaveRoom()
{
    if (m_state != MeetingState::InMeeting) return;

    if (m_network) {
        m_network->sendLeaveRoom();
        m_network->disconnectFromServer();
    }
    if (m_repo) {
        m_repo->clearAll();
    }
    m_currentRoom = RoomInfo{};
    m_localUserId.clear();
    setState(MeetingState::Idle);
    emit meetingExited();
}

void MeetingController::onRoomClosed()
{
    onRoomClosedFromNetwork();
}

void MeetingController::onMemberJoin(const Participant &p)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;
    m_repo->upsertParticipant(p);
}

void MeetingController::onMemberLeave(const QString &userId)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;
    m_repo->removeParticipant(userId);
}

void MeetingController::onMediaStateSync(const QString &userId,
                                          bool camera, bool mic, bool screen)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;
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
QString MeetingController::localUserId() const { return m_localUserId; }

// ─── private slots ────────────────────────────────────────────────────────────

void MeetingController::onServerConnected()
{
    if (m_pendingAction == 1) {
        // 发送创建会议请求。
        QJsonObject payload;
        payload["max_members"]  = m_pendingCreate.maxMembers;
        payload["has_password"] = m_pendingCreate.hasPassword;
        payload["password"]     = m_pendingCreate.password;
        payload["nickname"]     = m_pendingCreate.nickname;
        payload["avatar_base64"]= m_pendingCreate.avatarBase64;
        m_network->sendCreateRoom(payload);
    } else if (m_pendingAction == 2) {
        // 发送加入会议请求。
        QJsonObject payload;
        payload["room_id"]      = m_pendingJoin.roomId;
        payload["password"]     = m_pendingJoin.password;
        payload["nickname"]     = m_pendingJoin.nickname;
        payload["avatar_base64"]= m_pendingJoin.avatarBase64;
        m_network->sendJoinRoom(payload);
    }
}

void MeetingController::onServerReconnectFailed()
{
    if (m_repo) m_repo->clearAll();
    m_currentRoom = RoomInfo{};
    m_localUserId.clear();
    m_pendingAction = 0;
    setState(MeetingState::Idle);
    emit errorOccurred("网络错误", "与服务器的连接已断开，多次重连失败，请检查网络后重试。");
}

void MeetingController::onServerConnectTimeout()
{
    // 首次连接超时，回滚状态并提示用户。
    m_pendingAction = 0;
    setState(MeetingState::Idle);
    emit errorOccurred("连接超时", "无法连接到服务器，请检查服务器是否已启动或网络是否正常。");
}

void MeetingController::onCreateRoomAck(bool success, const QString &roomId,
                                         const QString &errorMsg)
{
    m_pendingAction = 0;
    if (!success) {
        if (m_network) m_network->disconnectFromServer();
        setState(MeetingState::Idle);
        emit errorOccurred("创建会议失败", errorMsg);
        return;
    }

    // 服务器分配的 userId 通过 payload["user_id"] 传回，这里通过 roomId 附带的 payload 获取；
    // 当前协议中 user_id 由 JOIN_ROOM_ACK 或 CREATE_ROOM_ACK 下发。
    m_currentRoom.roomId      = roomId;
    m_currentRoom.maxMembers  = m_pendingCreate.maxMembers;
    m_currentRoom.hasPassword = m_pendingCreate.hasPassword;
    m_currentRoom.isHost      = true;

    // 将自己加入参会者列表（userId 由服务器随后的 MEMBER_JOIN 通知带入，
    // 此处先用临时 ID 占位，MEMBER_JOIN 到达后会被 upsert 覆盖）。
    if (m_repo) {
        Participant self;
        self.userId       = m_localUserId.isEmpty() ? "local_host" : m_localUserId;
        self.nickname     = m_pendingCreate.nickname;
        self.avatarBase64 = m_pendingCreate.avatarBase64;
        self.isHost       = true;
        m_currentRoom.hostUserId = self.userId;
        m_repo->upsertParticipant(self);
    }

    setState(MeetingState::InMeeting);
    emit meetingEntered(m_currentRoom);
}

void MeetingController::onJoinRoomAck(bool success, const QString &roomId,
                                       const QString &hostUserId, const QString &errorMsg)
{
    m_pendingAction = 0;
    if (!success) {
        if (m_network) m_network->disconnectFromServer();
        setState(MeetingState::Idle);
        emit errorOccurred("加入会议失败", errorMsg);
        return;
    }

    m_currentRoom.roomId      = roomId;
    m_currentRoom.hostUserId  = hostUserId;
    m_currentRoom.isHost      = false;

    if (m_repo) {
        Participant self;
        self.userId       = m_localUserId.isEmpty() ? "local_user" : m_localUserId;
        self.nickname     = m_pendingJoin.nickname;
        self.avatarBase64 = m_pendingJoin.avatarBase64;
        self.isHost       = false;
        m_repo->upsertParticipant(self);
    }

    setState(MeetingState::InMeeting);
    emit meetingEntered(m_currentRoom);
}

void MeetingController::onMemberJoinFromNetwork(QJsonObject payload)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;
    Participant p;
    p.userId       = payload["user_id"].toString();
    p.nickname     = payload["nickname"].toString();
    p.avatarBase64 = payload["avatar_base64"].toString();
    p.isHost       = payload["is_host"].toBool();
    m_repo->upsertParticipant(p);
}

void MeetingController::onMemberLeaveFromNetwork(const QString &userId)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;
    m_repo->removeParticipant(userId);
}

void MeetingController::onMediaStateSyncFromNetwork(QJsonObject payload)
{
    if (m_state != MeetingState::InMeeting || !m_repo) return;
    const QString uid    = payload["user_id"].toString();
    const bool    camera = payload["camera"].toBool();
    const bool    mic    = payload["microphone"].toBool();
    const bool    screen = payload["screen_share"].toBool();
    onMediaStateSync(uid, camera, mic, screen);
}

void MeetingController::onRoomClosedFromNetwork()
{
    if (m_state == MeetingState::Idle) return;
    if (m_network) m_network->disconnectFromServer();
    if (m_repo)    m_repo->clearAll();
    m_currentRoom = RoomInfo{};
    m_localUserId.clear();
    m_pendingAction = 0;
    setState(MeetingState::Idle);
    emit roomClosed();
}

// ─── private ──────────────────────────────────────────────────────────────────

void MeetingController::setState(MeetingState s)
{
    if (m_state != s) {
        m_state = s;
        emit stateChanged(m_state);
    }
}
