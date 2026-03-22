#pragma once
#include "domain/model/RoomInfo.h"

/**
 * @file MeetingController.h
 * @brief 声明会议控制业务类及其参数结构。
 */

/**
 * @struct CreateRoomOptions
 * @brief 描述创建会议时需要提交的参数。
 */
struct CreateRoomOptions
{
    int     maxMembers  = 10;  ///< 会议允许的最大参会人数。
    bool    hasPassword = false; ///< 是否启用会议密码。
    QString password;          ///< 会议密码内容。
    QString nickname;          ///< 创建者昵称。
};

/**
 * @struct JoinRoomOptions
 * @brief 描述加入会议时需要提交的参数。
 */
struct JoinRoomOptions
{
    QString roomId;            ///< 目标会议房间号。
    QString password;          ///< 房间密码。
    QString nickname;          ///< 加入会议时使用的昵称。
};

class NetworkFacade;

/**
 * @class MeetingController
 * @brief 负责封装会议创建、加入和离开等业务流程。
 */
class MeetingController
{
public:
    /**
     * @brief 构造会议控制器。
     */
    MeetingController();

    /**
     * @brief 设置底层网络门面对象。
     * @param[in] facade 负责实际网络通信的门面指针。
     */
    void setNetworkFacade(NetworkFacade *facade);

    /**
     * @brief 发起创建会议请求。
     * @param[in] opts 创建会议所需的参数集合。
     */
    void createRoom(const CreateRoomOptions &opts);

    /**
     * @brief 发起加入会议请求。
     * @param[in] opts 加入会议所需的参数集合。
     */
    void joinRoom(const JoinRoomOptions &opts);

    /**
     * @brief 发起离开会议操作。
     */
    void leaveRoom();

    /**
     * @brief 处理房间被关闭后的业务状态更新。
     */
    void onRoomClosed();

private:
    NetworkFacade *m_network = nullptr; ///< 当前绑定的网络通信门面。
    RoomInfo       m_currentRoom;       ///< 当前会议房间信息缓存。
};
