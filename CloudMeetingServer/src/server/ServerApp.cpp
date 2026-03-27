#include "server/ServerApp.h"
#include "server/TcpSignalingServer.h"
#include "server/UdpMediaServer.h"
#include "common/Constants.h"
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
