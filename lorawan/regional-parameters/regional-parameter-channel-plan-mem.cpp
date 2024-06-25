#include "regional-parameter-channel-plan-mem.h"
#include "lorawan/lorawan-string.h"
#include "lorawan/lorawan-conv.h"
#include "lorawan/lorawan-error.h"
#include "lorawan/lorawan-msg.h"

RegionalParameterChannelPlanMem::RegionalParameterChannelPlanMem()
{

}

RegionalParameterChannelPlanMem::~RegionalParameterChannelPlanMem()
{
	done();
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanMem::get(
    const std::string &name
) const
{
    for (auto it(storage.bands.begin()); it != storage.bands.end(); it++) {
        if (it->name == name)
            return &*it;
    }
    return nullptr;
}

const RegionalParameterChannelPlan *RegionalParameterChannelPlanMem::get(int id) const
{
    for (auto it(storage.bands.begin()); it != storage.bands.end(); it++) {
        if (it->id == id)
            return &*it;
    }
    return nullptr;
}

int RegionalParameterChannelPlanMem::init(
	const std::string &option, 
	void *data
)
{
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

std::string RegionalParameterChannelPlanMem::getErrorDescription(int &subCode) const
{
    return "";
}
