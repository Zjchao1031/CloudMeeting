#pragma once
#include "domain/model/ChatMessage.h"
#include <QObject>
#include <QVector>
#include <QString>
#include <QJsonObject>

class NetworkFacade;

/**
 * @file ChatService.h
 * @brief 声明聊天消息业务服务。
 */

/**
 * @class ChatService
 * @brief 负责聊天消息校验、发送与本地缓存管理。
 */
class ChatService : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造聊天服务。
     * @param[in] parent 父对象指针。
     */
    explicit ChatService(QObject *parent = nullptr);

    /**
     * @brief 设置底层网络门面对象。
     * @param[in] facade 负责实际网络通信的门面指针。
     */
    void setNetworkFacade(NetworkFacade *facade);

    /**
     * @brief 校验消息的 UTF-8 字节长度是否合规。
     * @param[in] text 待校验的消息文本。
     * @return 若消息长度未超过限制则返回 `true`，否则返回 `false`。
     */
    bool validateMessageUtf8Bytes(const QString &text) const;

    /**
     * @brief 发送一条聊天消息。
     * @param[in] text 待发送的消息文本。
     */
    void sendMessage(const QString &text);

    /**
     * @brief 追加一条收到的聊天消息并通知 UI。
     * @param[in] msg 待加入本地缓存的消息对象。
     */
    void appendIncomingMessage(const ChatMessage &msg);

    /**
     * @brief 清空所有缓存的聊天消息。
     */
    void clearMessages();

    /**
     * @brief 获取当前缓存的消息列表。
     * @return 聊天消息缓存的常量引用。
     */
    const QVector<ChatMessage> &messages() const;

signals:
    /**
     * @brief 收到新消息时发出该信号。
     * @param[in] msg 新收到的消息对象。
     */
    void newMessageReceived(const ChatMessage &msg);

    /**
     * @brief 消息发送校验失败时发出该信号。
     * @param[in] reason 失败原因描述。
     */
    void sendFailed(const QString &reason);

private slots:
    /**
     * @brief 处理来自 NetworkFacade 的聊天广播通知。
     * @param[in] payload 包含消息信息的 JSON 载荷。
     */
    void onChatBroadcast(QJsonObject payload);

private:
    NetworkFacade       *m_network = nullptr; ///< 当前绑定的网络通信门面。
    QVector<ChatMessage> m_messages;          ///< 按接收顺序保存的聊天消息列表。
};
