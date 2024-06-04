#include "Sampler.h"
namespace vka
{
// Overrides start
hash_t SamplerDefinition::hash() const
{
	return shallowHashStructure(this);
}

bool SamplerDefinition::_equals(ResourceIdentifier const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<SamplerDefinition const &>(other);
		return this->equals(other_);
	}
}
bool SamplerDefinition::equals(SamplerDefinition const &other) const
{
	return shallowCmpStructure(this, &other);
}

hash_t Sampler::hash() const
{
	return (hash_t) handle;
}

bool Sampler::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<Sampler const &>(other);
		return this->handle == other_.handle;
	}
}
void Sampler::free()
{
	vkDestroySampler(gState.device.logical, handle, nullptr);
}
// Overrides end

VkSampler Sampler::getHandle() const
{
	return handle;
}

Sampler::Sampler(SamplerDefinition const &definition)
{
	VK_CHECK(vkCreateSampler(gState.device.logical, &definition, nullptr, &handle));
}

}        // namespace vka