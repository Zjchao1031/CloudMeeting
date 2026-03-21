#pragma once
#include <QByteArray>

// 封装 FFmpeg Opus 音频解码逻辑
class AudioDecoder
{
public:
    AudioDecoder();
    ~AudioDecoder();

    bool open(int sampleRate = 48000, int channels = 1);
    void close();
    // 输入 Opus 帧，输出 PCM 原始数据
    QByteArray decode(const QByteArray &opusData);

private:
    bool m_opened = false;
};
