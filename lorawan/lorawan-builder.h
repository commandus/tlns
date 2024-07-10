#ifndef LORAWAN_BUILDER_H
#define LORAWAN_BUILDER_H

#include "lorawan/lorawan-types.h"
#include "lorawan/lorawan-packet-storage.h"

class MessageBuilder {
public:
    LORAWAN_MESSAGE_STORAGE msg; // minimum 12 bytes
    MessageBuilder();
};

/**
 *
 */
class ConfirmationMessage : public MessageBuilder {
public:
    explicit ConfirmationMessage(
        const LORAWAN_MESSAGE_STORAGE &message2confirm
    );
};

#endif
