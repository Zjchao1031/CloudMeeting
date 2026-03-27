#include "server/ServerApp.h"
#include "server/TcpSignalingServer.h"
#include "server/UdpMediaServer.h"
#include "common/Constants.h"
#include "common/Logger.h"
#include <csignal>
#include <mutex>
#include <condition_variable>

static std::mutex              g_shutdownMutex;
static std::condition_variable g_shutdownCv;
static bool                    g_shutdown = false;

static void signalHandler(int)
{
    std::lock_guard<std::mutex> lk(g_shutdownMutex);
    g_shutdown = true;
    g_shutdownCv.notify_all();
}

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

    if (!m_udpServer->start(Constants::UDP_AUDIO_UP_PORT, Constants::UDP_VIDEO_UP_PORT,
                             Constants::UDP_AUDIO_DN_PORT, Constants::UDP_VIDEO_DN_PORT)) {
        Logger::error("UdpMediaServer failed to start");
        return;
    }
    if (!m_tcpServer->start(Constants::TCP_SIGNAL_PORT)) {
        Logger::error("TcpSignalingServer failed to start");
        return;
    }

    Logger::info("Server is running. Press Ctrl+C to stop.");

    std::signal(SIGINT,  signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::unique_lock<std::mutex> lk(g_shutdownMutex);
    g_shutdownCv.wait(lk, []{ return g_shutdown; });

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
