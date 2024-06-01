#include "Sampler.h"

vka::Sampler::Sampler(ResourceTracker *pTracker, const VkSamplerCreateInfo &ci) :
    UniqueResource<VkSampler>(pTracker), ci(ci)
{
}

vka::Sampler::~Sampler()
{
}
