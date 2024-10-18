#include "task-descriptor.h"
#include "lorawan/lorawan-error.h"

TaskDescriptor::TaskDescriptor()
    : errorCode(CODE_OK), repeats(0)
{
}

TaskDescriptor::TaskDescriptor(
    const TaskDescriptor &value
)
    : stage(value.stage), state(value.state), errorCode(value.errorCode),
        repeats(value.repeats), deviceId(value.deviceId), gatewayId(value.gatewayId)
{

}

TaskDescriptor& TaskDescriptor::operator=(
    const TaskDescriptor& value
)
{
    stage = value.stage;
    state = value.state;
    errorCode = value.errorCode;
    repeats = value.repeats;
    deviceId = value.deviceId;
    gatewayId = value.gatewayId;
    return *this;
}

TaskDescriptor::~TaskDescriptor() = default;
