#include "Sampler.h"
#include <vka/state_objects/global_state.h>
namespace vka
{
hash_t SamplerDefinition::hash() const
{
	//printVka("Sampler Hashfunction called:%d\n", byteHashPtr(this));
	// clang-format off
	return flags                  
	HASHC magFilter              
	HASHC minFilter              
	HASHC mipmapMode             
	HASHC addressModeU           
	HASHC addressModeV           
	HASHC addressModeW           
	HASHC mipLodBias             
	HASHC anisotropyEnable       
	HASHC maxAnisotropy          
	HASHC compareEnable          
	HASHC compareOp              
	HASHC minLod                 
	HASHC maxLod                 
	HASHC borderColor            
	HASHC unnormalizedCoordinates;
		// clang-format on
}

DEFINE_EQUALS_OVERLOAD(SamplerDefinition, ResourceIdentifier)

bool SamplerDefinition::operator==(const SamplerDefinition &other) const
{
	// clang-format off
	return magFilter			== other.magFilter         
	&& minFilter				== other.minFilter         
	&& mipmapMode				== other.mipmapMode
	&& addressModeU				== other.addressModeU
	&& addressModeV				== other.addressModeV
	&& addressModeW				== other.addressModeW
	&& mipLodBias				== other.mipLodBias
	&& anisotropyEnable			== other.anisotropyEnable
	&& maxAnisotropy			== other.maxAnisotropy
	&& compareEnable			== other.compareEnable
	&& compareOp				== other.compareOp
	&& minLod					== other.minLod
	&& maxLod					== other.maxLod
	&& borderColor				== other.borderColor
	&& unnormalizedCoordinates	== other.unnormalizedCoordinates;
	// clang-format on
	/*bool isSame = memcmpPtr(this, &other);
	if (!isSame)
	{
		printVka("samplers not equal\n");
	}
	else
	{
		printVka("samplers are equal");
	}
	return isSame;*/
}

void SamplerDefinition::writeDescriptorInfo(VkWriteDescriptorSet &write, VkDescriptorBufferInfo *&pBufferInfo, VkDescriptorImageInfo *&pImageInfos) const
{
	pImageInfos->sampler     = gState.cache->fetch(*this);
	write.pImageInfo         = pImageInfos;
	pImageInfos++;
}

void Sampler::free()
{
	vkDestroySampler(gState.device.logical, handle, nullptr);
}

Sampler::Sampler(IResourceCache *pCache, SamplerDefinition const &definition)
	: Cachable_T<VkSampler>(pCache)
{
	VK_CHECK(vkCreateSampler(gState.device.logical, &definition, nullptr, &handle));
}



}        // namespace vka