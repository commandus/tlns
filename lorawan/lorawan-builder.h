#ifndef LORAWAN_BUILDER_H
#define LORAWAN_BUILDER_H

#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/task/task-descriptor.h"

class MessageBuilder {
public:
    LORAWAN_MESSAGE_STORAGE msg; // minimum 12 bytes
    const TaskDescriptor &taskDescriptor;
    explicit MessageBuilder(const TaskDescriptor &taskDescriptor);
    size_t get(void *buffer, size_t size) const;
    size_t size() const;
    std::string base64() const;
};

/**
 * Called from MessageTaskDispatcher::sendQueue() if node requests confirmation
 */
class ConfirmationMessage : public MessageBuilder {
public:
    explicit ConfirmationMessage(
        const LORAWAN_MESSAGE_STORAGE &message2confirm,
        const TaskDescriptor &taskDescriptor
    );
};

/**
 * 3.2. Downlink message is sent by the network server to only one end-device and is relayed by a single gateway.
 */
class DownlinkMessage : public MessageBuilder {
public:
    explicit DownlinkMessage(
        uint8_t fport,
        const void *payload, // up to 255 bytes, can be NULL
        uint8_t payloadSize,
        const void *fopts, // up to 15 bytes, can be NULL
        uint8_t foptsSize,
        const TaskDescriptor &taskDescriptor    // contain NetworkIdentity and best gateway address
    );
};

#endif
