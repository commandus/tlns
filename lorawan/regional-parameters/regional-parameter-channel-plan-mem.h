#ifndef REGIONAL_PARAMETER_CHANNEL_PLAN_MEM_H_
#define REGIONAL_PARAMETER_CHANNEL_PLAN_MEM_H_ 1

#include "lorawan/regional-parameters/regional-parameter-channel-plans.h"

class RegionalParameterChannelPlanMem : public RegionalParameterChannelPlans {
    private:
        const RegionalParameterChannelPlan *getDefault() const;
	public:
        RegionBands storage;
        RegionalParameterChannelPlanMem();
		~RegionalParameterChannelPlanMem() override;

        virtual const RegionalParameterChannelPlan *get(const std::string &name) const override;
        virtual const RegionalParameterChannelPlan *get(int id) const override;

        virtual int init(const std::string &option, void *data) override;
        virtual void flush() override;
        virtual void done() override;
        virtual std::string toJsonString() const override;
        virtual std::string getErrorDescription(int &subCode) const override;
};

#endif
