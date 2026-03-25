#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QByteArray>
#include <QHash>
#include <QPair>
#include <QTimer>

/**
 * @file MediaUdpClient.h
 * @brief 声明 UDP 媒体传输客户端。
 */

/**
 * @class MediaUdpClient
 * @brief 负责通过 UDP 发送与接收音视频媒体数据。
 */
class MediaUdpClient : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造 UDP 媒体客户端。
     * @param[in] parent 父对象指针。
     */
    explicit MediaUdpClient(QObject *parent = nullptr);

    /**
     * @brief 析构 UDP 媒体客户端。
     */
    ~MediaUdpClient();

    /**
     * @brief 初始化媒体服务器地址与端口。
     * @param[in] serverHost 媒体服务器主机地址。
     * @param[in] audioPort 音频传输端口。
     * @param[in] videoPort 视频传输端口。
     */
    void init(const QString &serverHost, quint16 audioPort, quint16 videoPort);

    /**
     * @brief 发送一帧音频数据。
     * @param[in] userId 音频所属用户标识。
     * @param[in] opusData 待发送的 Opus 音频数据。
     */
    void sendAudioFrame(quint32 userId, const QByteArray &opusData);

    /**
     * @brief 发送视频帧分片数据。
     * @param[in] userId 视频所属用户标识。
     * @param[in] h264Data 待发送的 H.264 数据。
     * @param[in] isCamera 标识数据是否来自摄像头流。
     */
    void sendVideoFrameFragments(quint32 userId, const QByteArray &h264Data, bool isCamera);

signals:
    /**
     * @brief 当收到音频数据时发出该信号。
     * @param[in] userId 音频数据所属用户标识。
     * @param[in] data 收到的音频载荷数据。
     */
    void audioDataReceived(quint32 userId, QByteArray data);

    /**
     * @brief 当收到视频数据时发出该信号。
     * @param[in] userId 视频数据所属用户标识。
     * @param[in] data 收到的视频载荷数据。
     * @param[in] isCamera 标识该视频数据是否来自摄像头流。
     */
    void videoDataReceived(quint32 userId, QByteArray data, bool isCamera);

private slots:
    /**
     * @brief 处理音频套接字的可读事件。
     */
    void onAudioDatagram();

    /**
     * @brief 处理视频套接字的可读事件。
     */
    void onVideoDatagram();

    /**
     * @brief 定期清理超时未完成的分片重组缓冲条目。
     */
    void onGcTimer();

private:
    /**
     * @brief 单路视频分片重组缓冲元素。
     */
    struct FragBuf {
        QHash<quint8, QByteArray> frags;   ///< 分片索引 -> 分片数据。
        quint8  totalFrags = 0;            ///< 应收分片总数（0 表示未知）。
        qint64  arrivedAt  = 0;            ///< 首片到达的毫秒时间戳，用于超时清理。
    };

    QUdpSocket m_audioUdp;          ///< 音频传输使用的 UDP 套接字。
    QUdpSocket m_videoUdp;          ///< 视频传输使用的 UDP 套接字。
    QString    m_serverHost;        ///< 媒体服务器主机地址。
    quint16    m_audioPort = 9001;  ///< 音频上行端口。
    quint16    m_videoPort = 9002;  ///< 视频上行端口。
    quint16    m_audioSeq  = 0;     ///< 音频包序号计数器。
    quint16    m_videoSeq  = 0;     ///< 视频包序号计数器。

    // 视频分片重组缓冲区： key = (userId, seq)
    QHash<QPair<quint32, quint16>, FragBuf> m_videoFragBuf;
    QTimer m_gcTimer; ///< 定期清理已超时的不完整分片条目。
};
