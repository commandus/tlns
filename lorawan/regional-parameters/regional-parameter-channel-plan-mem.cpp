#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/regional-parameters/regional-parameter-channel-plan-mem.h"

#include <lorawan/lorawan-error.h>
#include <algorithm>

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
    std::string upperName(name);
    std::transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);
    for (auto it(storage.bands.begin()); it != storage.bands.end(); it++) {
        std::string valUpperCN(it->value.cn);
        std::transform(valUpperCN.begin(), valUpperCN.end(), valUpperCN.begin(), ::toupper);
        std::string valUpperName(it->value.name);
        std::transform(valUpperName.begin(), valUpperName.end(), valUpperName.begin(), ::toupper);
        if (valUpperCN.find(upperName) != std::string::npos)
            return &*it;
        if (valUpperName.find(upperName) != std::string::npos)
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
