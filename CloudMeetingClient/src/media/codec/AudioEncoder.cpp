/**
 * @file AudioEncoder.cpp
 * @brief 实现音频编码器封装类。
 */
#include "media/codec/AudioEncoder.h"

AudioEncoder::AudioEncoder() {}
AudioEncoder::~AudioEncoder() { close(); }

bool AudioEncoder::open(int sampleRate, int channels)
{
    Q_UNUSED(sampleRate) Q_UNUSED(channels)
    m_opened = true;
    return true;
}

void AudioEncoder::close() { m_opened = false; }

QByteArray AudioEncoder::encode(const QByteArray &pcmData)
{
    Q_UNUSED(pcmData)
    return {}; // 待实现：FFmpeg Opus 编码。
}
