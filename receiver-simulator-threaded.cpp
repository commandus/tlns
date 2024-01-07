#include "receiver-simulator-threaded.h"

void ReceiverSimulatorThreaded::onReceive(
        MessageQueueItem *value
) {
    TaskResponse::onReceive(value);
}

void ReceiverSimulatorThreaded::onResponse(
        MessageQueueItem* value
)
{
    TaskResponse::onResponse(value);
}

ReceiverSimulatorThreaded::ReceiverSimulatorThreaded()
    : TaskResponse()
{

}

ReceiverSimulatorThreaded::~ReceiverSimulatorThreaded()
{

}
