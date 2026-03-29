cmake_minimum_required(VERSION 3.19)
project(CloudMeetingClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Multimedia Network)

set(FFMPEG_DIR "F:/ffmpeg-master-latest-win64-lgpl-shared")
include_directories(${FFMPEG_DIR}/include)
link_directories(${FFMPEG_DIR}/lib)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

qt_standard_project_setup()

set(CLIENT_SOURCES
    src/main.cpp
    src/app/AppContext.cpp

    # UI - Main windows
    src/ui/MainWindow.cpp
    include/ui/MainWindow.h
    src/ui/MeetingWindow.cpp
    include/ui/MeetingWindow.h

    # UI - Dialogs
    src/ui/dialogs/UserProfileDialog.cpp
    include/ui/dialogs/UserProfileDialog.h
    src/ui/dialogs/CreateMeetingDialog.cpp
    include/ui/dialogs/CreateMeetingDialog.h
    src/ui/dialogs/JoinMeetingDialog.cpp
    include/ui/dialogs/JoinMeetingDialog.h
    src/ui/dialogs/ConfirmDialog.cpp
    include/ui/dialogs/ConfirmDialog.h

    # UI - Widgets
    src/ui/widgets/VideoTileWidget.cpp
    include/ui/widgets/VideoTileWidget.h
    src/ui/widgets/ParticipantListWidget.cpp
    include/ui/widgets/ParticipantListWidget.h
    src/ui/widgets/ChatPanel.cpp
    include/ui/widgets/ChatPanel.h
    src/ui/widgets/ToolBarPanel.cpp
    include/ui/widgets/ToolBarPanel.h

    # Domain services
    src/domain/service/MeetingController.cpp
    include/domain/service/MeetingController.h
    src/domain/service/ParticipantRepository.cpp
    include/domain/service/ParticipantRepository.h
    src/domain/service/ChatService.cpp
    include/domain/service/ChatService.h
    src/domain/service/UserProfileService.cpp

    # Network
    src/network/NetworkFacade.cpp
    include/network/NetworkFacade.h
    src/network/SignalingClient.cpp
    include/network/SignalingClient.h
    src/network/MediaUdpClient.cpp
    include/network/MediaUdpClient.h
    src/network/PacketCodec.cpp

    # Media
    src/media/MediaEngine.cpp
    include/media/MediaEngine.h
    src/media/capture/CameraCaptureStrategy.cpp
    src/media/capture/ScreenCaptureStrategy.cpp
    src/media/codec/VideoEncoder.cpp
    src/media/codec/VideoDecoder.cpp
    src/media/codec/AudioEncoder.cpp
    src/media/codec/AudioDecoder.cpp
    src/media/device/DeviceManager.cpp

    # Common
    src/common/Logger.cpp
)

qt_add_executable(CloudMeetingClient
    WIN32
    ${CLIENT_SOURCES}
    avatars.qrc
)


target_link_libraries(CloudMeetingClient
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Network
        avcodec
        avformat
        avdevice
        avutil
        swscale
        swresample
)

include(GNUInstallDirs)
install(TARGETS CloudMeetingClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
