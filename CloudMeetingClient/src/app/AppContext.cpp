/**
 * @file AppContext.cpp
 * @brief 实现应用上下文单例。
 */
#include "app/AppContext.h"
#include "domain/service/UserProfileService.h"
#include "domain/service/ParticipantRepository.h"
#include "domain/service/MeetingController.h"
#include "domain/service/ChatService.h"
#include "network/NetworkFacade.h"

AppContext &AppContext::instance()
{
    static AppContext ctx;
    return ctx;
}

void AppContext::setup()
{
    // 创建业务服务实例。
    m_profileService  = new UserProfileService;
    m_participantRepo = new ParticipantRepository;
    m_meetingCtrl     = new MeetingController;
    m_chatService     = new ChatService;

    // 创建网络门面。
    m_networkFacade   = new NetworkFacade;

    // 注入依赖：将 NetworkFacade 注入各业务服务。
    m_meetingCtrl->setParticipantRepository(m_participantRepo);
    m_meetingCtrl->setNetworkFacade(m_networkFacade);
    m_chatService->setNetworkFacade(m_networkFacade);

    // 加载用户资料（昵称 + 头像）。
    m_profileService->load();
}

UserProfileService*    AppContext::userProfileService()    const { return m_profileService; }
ParticipantRepository* AppContext::participantRepository() const { return m_participantRepo; }
MeetingController*     AppContext::meetingController()     const { return m_meetingCtrl; }
ChatService*           AppContext::chatService()           const { return m_chatService; }
NetworkFacade*         AppContext::networkFacade()         const { return m_networkFacade; }
