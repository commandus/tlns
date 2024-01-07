#include "task-response.h"

TaskResponse::TaskResponse() {

}

TaskResponse::TaskResponse(
    const TaskResponse &value
)
    : queue(value.queue)
{

}

TaskResponse::~TaskResponse() = default;

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

