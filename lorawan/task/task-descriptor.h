#ifndef TASK_DESCRIPTOR_H_
#define TASK_DESCRIPTOR_H_	1

#include <string>
#include <vector>

#include "lorawan/storage/network-identity.h"
#include "lorawan/storage/gateway-identity.h"

typedef enum {
    TASK_STAGE_RECEIVED = 0,                // just received
    TASK_STAGE_GATEWAY_REQUEST,             // Request gateway identifier
    TASK_STAGE_GATEWAY_RESPONSE,            // Gateway identifier received
    TASK_STAGE_IDENTIFIED,                  // got device identifier: network / app keys
    TASK_STAGE_DECIPHERED,                  // deciphered

    TASK_STAGE_MAC_PROCESS,                 // MAC command process initiated
    TASK_STAGE_MAC_RESPONSE_SEND,           // MAC command response require to be send
    TASK_STAGE_MAC_RESPONSE_GW_SELECT,      // Select best gateway and getUplink adress
    TASK_STAGE_MAC_RESPONSE_GW_SELECTED,    // Best gateway seleted
    TASK_STAGE_MAC_RESPONSE_SENT,           // MAC command response sent

    TASK_STAGE_PAYLOAD_PROCESS,             // MAC command process initiated
    TASK_STAGE_PAYLOAD_SEND,                // payload need to be send to the app service
    TASK_STAGE_PAYLOAD_SENT,                // payload sent to the app service

    TASK_STAGE_FINISHED                     // accepted or declined (sent to app server or to error log)
} TaskStage;

typedef enum {
    TASK_STATE_RETURNED = 0,                // returned to the Dispatcher
    TASK_STATE_IN_PROGRESS                  // task execution in progress
} TaskState;

class TaskDescriptor {
public:
    TaskStage stage;
    TaskState state;
    int errorCode;
    int repeats;

    NetworkIdentity deviceId;               // device keys
    GatewayIdentity gatewayId;              // best gateway address

    TaskDescriptor();
    TaskDescriptor(const TaskDescriptor &value);
    virtual ~TaskDescriptor();
    TaskDescriptor& operator=(const TaskDescriptor&);
};

#endif
