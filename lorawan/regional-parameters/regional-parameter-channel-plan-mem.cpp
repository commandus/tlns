#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/regional-parameters/regional-parameter-channel-plan-mem.h"

#include <lorawan/lorawan-error.h>

RegionalParameterChannelPlanMem::RegionalParameterChannelPlanMem()
{

}

RegionalParameterChannelPlanMem::RegionalParameterChannelPlanMem(
    const std::vector<REGIONAL_PARAMETER_CHANNEL_PLAN> &bands
)
    : storage(bands)
{

}

RegionalParameterChannelPlanMem::~RegionalParameterChannelPlanMem()
{
	done();
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanMem::getDefault() const
{
    for (auto it(storage.bands.begin()); it != storage.bands.end(); it++) {
        if (it->value.defaultRegion)
            return &*it;
    }
    return nullptr;
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanMem::get(
    const std::string &name
) const
{
    for (auto it(storage.bands.begin()); it != storage.bands.end(); it++) {
        if (it->value.name == name)
            return &*it;
    }
    return getDefault();
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanMem::get(int id) const
{
    for (auto it(storage.bands.begin()); it != storage.bands.end(); it++) {
        if (it->value.id == id)
            return &*it;
    }
    return getDefault();;
}

int RegionalParameterChannelPlanMem::init(
	const std::string &option, 
	void *data
)
{
    return CODE_OK;
}

void RegionalParameterChannelPlanMem::flush()
{
}

void RegionalParameterChannelPlanMem::done()
{
}

std::string RegionalParameterChannelPlanMem::toJsonString() const {
    return storage.toString();
}

std::string RegionalParameterChannelPlanMem::getErrorDescription(
    int &subCode
) const
{
    return "";
}
