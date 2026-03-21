#pragma once

enum class ErrorCode
{
    Ok               = 0,
    RoomNotFound     = 1,
    WrongPassword    = 2,
    RoomFull         = 3,
    NetworkError     = 4,
    CodecError       = 5,
    DeviceError      = 6,
    MessageTooLong   = 7,
};
