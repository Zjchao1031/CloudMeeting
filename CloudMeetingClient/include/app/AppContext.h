#pragma once
#include <memory>

class UserProfileService;
class ParticipantRepository;
class MeetingController;
class ChatService;
class NetworkFacade;
class MediaEngine;
class DeviceManager;

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

    /**
     * @brief 获取媒体引擎。
     * @return 媒体引擎指针。
     */
    MediaEngine* mediaEngine() const;

    /**
     * @brief 获取设备管理器。
     * @return 设备管理器指针。
     */
    DeviceManager* deviceManager() const;

private:
    AppContext() = default;

    /**
     * @brief 析构应用上下文，按创建逆序释放所有服务对象。
     */
    ~AppContext();

    // 声明顺序决定析构逆序，须与 setup() 的创建顺序相反，以保证依赖方先于被依赖方销毁。
    std::unique_ptr<UserProfileService>    m_profileService;  ///< 用户资料服务。
    std::unique_ptr<ParticipantRepository> m_participantRepo; ///< 参会者仓库。
    std::unique_ptr<MeetingController>     m_meetingCtrl;     ///< 会议控制器。
    std::unique_ptr<ChatService>           m_chatService;     ///< 聊天服务。
    std::unique_ptr<NetworkFacade>         m_networkFacade;   ///< 网络门面。
    std::unique_ptr<DeviceManager>         m_deviceManager;   ///< 音视频设备管理器。
    std::unique_ptr<MediaEngine>           m_mediaEngine;     ///< 媒体能力调度引擎（最先析构）。
};
