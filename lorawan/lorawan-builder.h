#ifndef LORAWAN_BUILDER_H
#define LORAWAN_BUILDER_H

#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/task/task-descriptor.h"

class MessageBuilder {
public:
    LORAWAN_MESSAGE_STORAGE msg; // minimum 12 bytes
    const TaskDescriptor &taskDescriptor;
    explicit MessageBuilder(const TaskDescriptor &taskDescriptor);
    MessageBuilder(const TaskDescriptor &taskDescriptor, const LORAWAN_MESSAGE_STORAGE &msg);
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
class DownlinkMessageBuilder : public MessageBuilder {
public:
    uint8_t payloadSize;
    /**
     * @param taskDescriptor. Provide task to reply to or create a new task
     * @param fport >0 if payload exists
     * @param payload optional, can be NULL
     * @param payloadSize payload size
     * @param fopts optional, up to 15 bytes, can be NULL
     * @param foptsSize
     */
    explicit DownlinkMessageBuilder(
        const TaskDescriptor &taskDescriptor,    // contain time to send, NetworkIdentity and best gateway address
        uint8_t fport,
        const void *payload, // up to 255 bytes, can be NULL
        uint8_t payloadSize,
        const void *fopts, // up to 15 bytes, can be NULL
        uint8_t foptsSize
    );
};

#endif
