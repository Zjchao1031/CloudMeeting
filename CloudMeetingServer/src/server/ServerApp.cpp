#include "server/ServerApp.h"
#include "server/TcpSignalingServer.h"
#include "server/UdpMediaServer.h"
#include "common/Constants.h"
#include "common/IniConfig.h"
#include "common/Logger.h"
#include <csignal>
#include <thread>
#include <chrono>

static volatile sig_atomic_t g_shutdown = 0;

static void signalHandler(int) { g_shutdown = 1; }

ServerApp::ServerApp()
    : m_tcpServer(new TcpSignalingServer)
    , m_udpServer(new UdpMediaServer)
{}

ServerApp::~ServerApp()
{
    stop();
    delete m_tcpServer;
    delete m_udpServer;
}

void ServerApp::run()
{
    Logger::info("CloudMeetingServer starting...");

    // 从 profile.ini 加载端口配置，文件不存在时使用 Constants:: 默认值。
    IniConfig cfg;
    cfg.load(CLOUDMEETING_SERVER_DIR "/profile.ini");

    uint16_t tcpPort    = cfg.getUInt16("Server", "tcpPort",      Constants::TCP_SIGNAL_PORT);
    uint16_t audioUp    = cfg.getUInt16("Server", "audioUpPort",  Constants::UDP_AUDIO_UP_PORT);
    uint16_t videoUp    = cfg.getUInt16("Server", "videoUpPort",  Constants::UDP_VIDEO_UP_PORT);
    uint16_t audioDn    = cfg.getUInt16("Server", "audioDnPort",  Constants::UDP_AUDIO_DN_PORT);
    uint16_t videoDn    = cfg.getUInt16("Server", "videoDnPort",  Constants::UDP_VIDEO_DN_PORT);

    Logger::info("Config: tcpPort=" + std::to_string(tcpPort)
                 + " audioUp=" + std::to_string(audioUp)
                 + " videoUp=" + std::to_string(videoUp)
                 + " audioDn=" + std::to_string(audioDn)
                 + " videoDn=" + std::to_string(videoDn));

    if (!m_udpServer->start(audioUp, videoUp, audioDn, videoDn)) {
        Logger::error("UdpMediaServer failed to start");
        return;
    }
    if (!m_tcpServer->start(tcpPort)) {
        Logger::error("TcpSignalingServer failed to start");
        return;
    }

    Logger::info("Server is running. Press Ctrl+C to stop.");

    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    while (!g_shutdown)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Logger::info("Shutdown signal received, stopping...");
    stop();
}

void ServerApp::stop()
{
    if (m_stopped.exchange(true)) return; // 已停止，直接返回
    m_tcpServer->stop();
    m_udpServer->stop();
    Logger::info("CloudMeetingServer stopped.");
}
