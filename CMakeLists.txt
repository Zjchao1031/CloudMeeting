cmake_minimum_required(VERSION 3.19)
project(CloudMeeting LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_CLIENT "Build CloudMeeting client (Windows)" ON)
option(BUILD_SERVER "Build CloudMeeting server (Linux)" ON)

if(WIN32 AND BUILD_CLIENT)
    add_subdirectory(CloudMeetingClient)
endif()

if(UNIX AND NOT APPLE AND BUILD_SERVER)
    add_subdirectory(CloudMeetingServer)
endif()
