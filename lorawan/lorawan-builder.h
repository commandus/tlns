#ifndef LORAWAN_BUILDER_H
#define LORAWAN_BUILDER_H

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"
#include "lorawan/task/task-descriptor.h"

class MessageBuilder {
public:
    LORAWAN_MESSAGE_STORAGE msg; // minimum 12 bytes
    const TaskDescriptor &taskDescriptor;
    explicit MessageBuilder(const TaskDescriptor &taskDescriptor);
    size_t get(void *buffer, size_t size) const;
    std::string base64() const;
};

/**
 *
 */
class ConfirmationMessage : public MessageBuilder {
public:
    explicit ConfirmationMessage(
        const LORAWAN_MESSAGE_STORAGE &message2confirm,
        const TaskDescriptor &taskDescriptor
    );
};

#endif
