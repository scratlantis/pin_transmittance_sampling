#include "Sampler.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t SamplerDefinition::hash() const
{
	return byteHashPtr(this);
}

DEFINE_EQUALS_OVERLOAD(SamplerDefinition, ResourceIdentifier)

bool SamplerDefinition::operator==(const SamplerDefinition &other) const
{
	return memcmpPtr(this, &other);
}

void Sampler::free()
{
	vkDestroySampler(gState.device.logical, handle, nullptr);
}

Sampler::Sampler(SamplerDefinition const &definition)
{
	VK_CHECK(vkCreateSampler(gState.device.logical, &definition, nullptr, &handle));
}

}        // namespace vka