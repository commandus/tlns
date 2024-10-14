#ifndef REGIONAL_PARAMETER_CHANNEL_PLANS_H_
#define REGIONAL_PARAMETER_CHANNEL_PLANS_H_ 1

#include <string>
#include "regional-parameter-channel-plan.h"

/**
 * Class keep channel plans in the storage.
 * Implementations:
 *  - in-memory storage RegionalParameterChannelPlanMem (file lorawan/regional-parameters/regional-parameter-channel-plan-mem.h)
 *  - JSON file storage RegionalParameterChannelPlanFileJson (file lorawan/regional-parameters/regional-parameter-channel-plan-file-json.h)
 */

class RegionalParameterChannelPlans {
    public:
        virtual ~RegionalParameterChannelPlans() {};
        // get plan by name in accordance to RP002-1.0.3 2.1 Regional Parameter Channel Plan Common Names
        virtual const RegionalParameterChannelPlan *get(const std::string &name) const = 0;
        // get plan by channel plan identifier in accordance to RP002-1.0.3 2.1 Regional Parameter Channel Plan Common Names
        virtual const RegionalParameterChannelPlan *get(int id) const = 0;

        virtual int init(const std::string &option, void *data) = 0;
        virtual void flush() = 0;
        virtual void done() = 0;
        virtual std::string toJsonString() const = 0;
        virtual std::string getErrorDescription(int &subCode) const = 0;
};

#endif
