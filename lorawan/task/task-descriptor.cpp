#include "task-descriptor.h"
#include "lorawan/lorawan-error.h"

TaskDescriptor::TaskDescriptor()
    : errorCode(CODE_OK), repeats(0)
{
}

TaskDescriptor::~TaskDescriptor() = default;
