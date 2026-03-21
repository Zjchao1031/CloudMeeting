#include "media/codec/AudioDecoder.h"

AudioDecoder::AudioDecoder() {}
AudioDecoder::~AudioDecoder() { close(); }

bool AudioDecoder::open(int sampleRate, int channels)
{
    Q_UNUSED(sampleRate) Q_UNUSED(channels)
    m_opened = true;
    return true;
}

void AudioDecoder::close() { m_opened = false; }

QByteArray AudioDecoder::decode(const QByteArray &opusData)
{
    Q_UNUSED(opusData)
    return {}; // TODO: FFmpeg Opus 解码
}
