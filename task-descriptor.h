#include <string>
#include <vector>

class DeviceId {};
class GatewayId {};
class GatewayMetadata {};

typedef enum TaskStage {
    TASK_STAGE_RECEIVED = 0,                // just received
    TASK_STAGE_IDENTIFIED,                  // got device identifier: network / app keys
    TASK_STAGE_DECIPHERED,                  // deciphered

    TASK_STAGE_MAC_PROCESS,                 // MAC command process initiated
    TASK_STAGE_MAC_RESPONSE_SEND,           // MAC command response require to be send
    TASK_STAGE_MAC_RESPONSE_GW_SELECT,      // Select best gateway and get adress
    TASK_STAGE_MAC_RESPONSE_GW_SELECTED,    // Best gatewsy seleted
    TASK_STAGE_MAC_RESPONSE_SENT,           // MAC command response sent

    TASK_STAGE_PAYLOAD_PROCESS,             // MAC command process initiated
    TASK_STAGE_PAYLOAD_SEND,                // payload need to be send to the app service
    TASK_STAGE_PAYLOAD_SENT,                // payload sent to the app service

    TASK_STAGE_FINISHED                     // accepted or declined (sent to app server or to error log)
};

/*
typedef enum TaskState {
    TASK_STATE_SUCCESS = 0,                 // no errors
    TASK_STATE_ABORT,                       // critical errors occured, abort entire task
    TASK_STATE_RETRY                        // critical errors occured, try again
};
*/

class TaskDescriptor {
public:
    TaskStage stage;
    // TaskState state;
    int errorCode;
    int repeats;

    DeviceId deviceId;                      // device keys
    GatewayId gatewayId;                    // best gateway address
    std::vector <GatewayMetadata> metadatas;
};
