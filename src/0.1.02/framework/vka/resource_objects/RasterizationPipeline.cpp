#include "RasterizationPipeline.h"
namespace vka
{
// Overrides start
hash_t RestarizationPipelineDefinition::hash() const
{
	hash_t hash;
	return hash;
}

bool RestarizationPipelineDefinition::_equals(ResourceIdentifier const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<RestarizationPipelineDefinition const &>(other);
		return this->equals(other_);
	}
}
bool RestarizationPipelineDefinition::equals(RestarizationPipelineDefinition const &other) const
{
	return false;
}

hash_t RasterizationPipeline::hash() const
{
	return (hash_t) handle;
}

bool RasterizationPipeline::_equals(Resource const &other) const
{
	if (typeid(*this) != typeid(other))
		return false;
	else
	{
		auto &other_ = static_cast<RasterizationPipeline const &>(other);
		return this->handle == other_.handle;
	}
}
void RasterizationPipeline::free()
{
	vkDestroyPipeline(gState.device.logical, handle, nullptr);
}
// Overrides end

VkPipeline RasterizationPipeline::getHandle() const
{
	return handle;
}

RasterizationPipeline::RasterizationPipeline(RestarizationPipelineDefinition const &definition)
{
}

}        // namespace vka