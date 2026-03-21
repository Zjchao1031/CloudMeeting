cmake_minimum_required(VERSION 3.16)
project(CloudMeetingServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SERVER_SOURCES
    src/main.cpp
    src/server/ServerApp.cpp
    src/server/TcpSignalingServer.cpp
    src/server/UdpMediaServer.cpp
    src/server/EpollReactor.cpp
    src/domain/Room.cpp
    src/domain/ClientSession.cpp
    src/domain/RoomManager.cpp
    src/domain/SessionManager.cpp
    src/protocol/SignalCodec.cpp
    src/protocol/MediaPacket.cpp
    src/handler/CreateRoomHandler.cpp
    src/handler/JoinRoomHandler.cpp
    src/handler/LeaveRoomHandler.cpp
    src/handler/MediaStateHandler.cpp
    src/handler/ChatMessageHandler.cpp
    src/handler/HeartbeatHandler.cpp
    src/handler/RequestKeyframeHandler.cpp
    src/service/RoomService.cpp
    src/service/BroadcastService.cpp
    src/service/MediaForwardService.cpp
    src/common/Logger.cpp
    src/common/TimeUtil.cpp
    src/common/IdGenerator.cpp
)

add_executable(CloudMeetingServer ${SERVER_SOURCES})

target_link_libraries(CloudMeetingServer
    PRIVATE
        pthread
)
