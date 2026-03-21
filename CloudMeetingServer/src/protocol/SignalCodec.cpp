#include "protocol/SignalCodec.h"
#include <cstring>
#include <arpa/inet.h>

std::vector<uint8_t> SignalCodec::encode(SignalType type, const std::string &jsonPayload)
{
    uint32_t len = htonl(static_cast<uint32_t>(jsonPayload.size()));
    std::vector<uint8_t> buf;
    buf.reserve(5 + jsonPayload.size());
    buf.push_back(static_cast<uint8_t>(type));
    uint8_t lenBytes[4];
    std::memcpy(lenBytes, &len, 4);
    buf.insert(buf.end(), lenBytes, lenBytes + 4);
    buf.insert(buf.end(), jsonPayload.begin(), jsonPayload.end());
    return buf;
}

bool SignalCodec::decode(const uint8_t *buf, size_t len,
                         SignalPacket &pkt, size_t &consumed)
{
    if (len < 5) return false;
    pkt.type = static_cast<SignalType>(buf[0]);
    uint32_t payloadLen;
    std::memcpy(&payloadLen, buf + 1, 4);
    payloadLen = ntohl(payloadLen);
    if (len < 5 + payloadLen) return false;
    pkt.payloadLen = payloadLen;
    pkt.payload.assign(reinterpret_cast<const char*>(buf + 5), payloadLen);
    consumed = 5 + payloadLen;
    return true;
}
