#include <algorithm>
#include "task-response.h"

TaskResponse::TaskResponse()
    : running(false)
{

}

TaskResponse::TaskResponse(
    const TaskResponse &value
)
    : running(value.running), queue(value.queue)
{

}

TaskResponse::~TaskResponse() = default;

bool TaskResponse::start()
{
    running = true;
    return true;
}

void TaskResponse::stop()
{
    if (!running)
        return;
    running = false;
}

void TaskResponse::onReceive(
    MessageQueueItem *value
) {
    queue.push_back(value);
}

void TaskResponse::onResponse(
    MessageQueueItem *value
) {
    auto it = std::find(queue.begin(), queue.end(), value);
    if (it != queue.end())
        queue.erase(it);
}
