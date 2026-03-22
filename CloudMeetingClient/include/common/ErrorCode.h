#pragma once

/**
 * @file ErrorCode.h
 * @brief 定义客户端业务错误码。
 */

/**
 * @enum ErrorCode
 * @brief 表示客户端在会议流程中可能出现的错误类型。
 */
enum class ErrorCode
{
    Ok             = 0, ///< 操作成功。
    RoomNotFound   = 1, ///< 指定会议房间不存在。
    WrongPassword  = 2, ///< 会议密码错误。
    RoomFull       = 3, ///< 会议房间人数已满。
    NetworkError   = 4, ///< 网络通信出现异常。
    CodecError     = 5, ///< 编解码模块发生错误。
    DeviceError    = 6, ///< 音视频设备不可用。
    MessageTooLong = 7, ///< 聊天消息长度超出限制。
};
