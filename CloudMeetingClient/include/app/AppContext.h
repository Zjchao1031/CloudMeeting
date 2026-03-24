#pragma once

class UserProfileService;
class ParticipantRepository;
class MeetingController;
class ChatService;
class NetworkFacade;

/**
 * @file AppContext.h
 * @brief 声明应用上下文单例。
 */

/**
 * @class AppContext
 * @brief 管理应用级服务实例与初始化流程的全局上下文。
 */
class AppContext
{
public:
    /**
     * @brief 获取应用上下文单例实例。
     * @return 应用上下文的全局唯一实例引用。
     */
    static AppContext& instance();

    /**
     * @brief 执行应用上下文初始化，创建并连接所有服务。
     */
    void setup();

    /**
     * @brief 获取用户资料服务。
     * @return 用户资料服务指针。
     */
    UserProfileService* userProfileService() const;

    /**
     * @brief 获取参会者仓库。
     * @return 参会者仓库指针。
     */
    ParticipantRepository* participantRepository() const;

    /**
     * @brief 获取会议控制器。
     * @return 会议控制器指针。
     */
    MeetingController* meetingController() const;

    /**
     * @brief 获取聊天服务。
     * @return 聊天服务指针。
     */
    ChatService* chatService() const;

    /**
     * @brief 获取网络门面。
     * @return 网络门面指针。
     */
    NetworkFacade* networkFacade() const;

private:
    AppContext() = default;

    UserProfileService    *m_profileService  = nullptr; ///< 用户资料服务。
    ParticipantRepository *m_participantRepo = nullptr; ///< 参会者仓库。
    MeetingController     *m_meetingCtrl     = nullptr; ///< 会议控制器。
    ChatService           *m_chatService     = nullptr; ///< 聊天服务。
    NetworkFacade         *m_networkFacade   = nullptr; ///< 网络门面。
};
