#pragma once
#include <framework/vka/render_model/common.h>
#include "DataStructs.h"
#include <random>

using namespace vka;


class GaussianBuffer : public BufferVma
{
  public:
	GaussianBuffer(uint32_t count, float margin,
	               VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(Gaussian) * count, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
		std::mt19937                          gen32(42);
		std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
		gaussiansData.resize(count);
		float                 coef = 0.3;
		for (size_t i = 0; i < count; i++)
		{
			gaussiansData[i].mean.x   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].mean.y   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].mean.z   = (1.0 - margin) / 2.0 + margin * unormDistribution(gen32);
			gaussiansData[i].variance = 0.5 * margin * unormDistribution(gen32);
		}
	}

	size_t size() const
	{
		return gaussiansData.size();
	}

	void upload(CmdBuffer& cmdBuf)
	{
		cmdBuf.uploadData(*this, 0, gaussiansData.data(), sizeof(Gaussian) * gaussiansData.size());
	}
	~GaussianBuffer();

  private:
	std::vector<Gaussian> gaussiansData;
};

class PinBuffer : public BufferVma
{
  public:
	PinBuffer(uint32_t count,
	               VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
	    BufferVma(&gState.heap, sizeof(Pin) * count, bufferUsageFlags | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
		std::mt19937                          gen32(42);
		std::uniform_real_distribution<float> unormDistribution(0.0, 1.0);
		pins.resize(count);
		for (size_t i = 0; i < pins.size(); i++)
		{
			pins[i].theta.x = 2.0 * PI * unormDistribution(gen32);
			pins[i].theta.y = 2.0 * PI * unormDistribution(gen32);
			pins[i].phi.x   = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
			pins[i].phi.y   = glm::acos(1.0 - 2.0 * unormDistribution(gen32));
		}
	}
	size_t size() const
	{
		return pins.size();
	}

	void upload(CmdBuffer &cmdBuf)
	{
		cmdBuf.uploadData(*this, 0, pins.data(), sizeof(Pin) * pins.size());
	}
	~PinBuffer();

  private:
	std::vector<Pin> pins;
};


class GaussianNN_M : public Material
{
  public:
	GaussianNN_M(const GaussianBuffer *gaussianBuf, const PinBuffer *pinBuf) :
	    gaussianBuf(gaussianBuf), pinBuf(pinBuf){};
	~GaussianNN_M(){};
	const GaussianBuffer *gaussianBuf;
	const PinBuffer      *pinBuf;
  private:
};


class Gaussian_M : public Material
{
  public:
	Gaussian_M(const GaussianBuffer *gaussianBuf) :
	    gaussianBuf(gaussianBuf){};
	~Gaussian_M(){};

	const GaussianBuffer* gaussianBuf;
  private:
};


class GaussianNNGrid_M : public Material
{
  public:
	GaussianNNGrid_M(const GaussianBuffer *gaussianBuf, const PinBuffer *pinBuf) :
		gaussianBuf(gaussianBuf), pinBuf(pinBuf){};
	~GaussianNNGrid_M(){};
	const GaussianBuffer *gaussianBuf;
	const PinBuffer      *pinBuf;
  private:
};
