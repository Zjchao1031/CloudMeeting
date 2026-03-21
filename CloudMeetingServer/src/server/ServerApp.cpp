#include "server/ServerApp.h"
#include "server/TcpSignalingServer.h"
#include "server/UdpMediaServer.h"
#include "common/Constants.h"
#include "common/Logger.h"

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
    Logger::info("ServerApp starting...");
    m_udpServer->start(Constants::UDP_AUDIO_UP_PORT, Constants::UDP_VIDEO_UP_PORT,
                       Constants::UDP_AUDIO_DN_PORT, Constants::UDP_VIDEO_DN_PORT);
    m_tcpServer->start(Constants::TCP_SIGNAL_PORT);
}

void ServerApp::stop()
{
    m_tcpServer->stop();
    m_udpServer->stop();
}
