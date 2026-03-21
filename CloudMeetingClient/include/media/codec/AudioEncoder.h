#pragma once
#include <QByteArray>

// 封装 FFmpeg Opus 音频编码逻辑
class AudioEncoder
{
public:
    AudioEncoder();
    ~AudioEncoder();

    bool open(int sampleRate = 48000, int channels = 1);
    void close();
    // 输入 PCM 原始数据，输出 Opus 编码帧
    QByteArray encode(const QByteArray &pcmData);

private:
    bool m_opened = false;
};
