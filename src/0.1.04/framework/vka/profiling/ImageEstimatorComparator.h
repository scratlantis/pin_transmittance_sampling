#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include "TimeQueryManager.h"
#include <vka/specialized_utility/image_metrics.h>

namespace vka
{
	struct IECToneMappingArgs
	{
		bool  useTonemapping;
		float whitePoint;
		float exposure;
	};
	enum IECTarget
	{
		IEC_TARGET_LEFT = 0,
		IEC_TARGET_RIGHT = 1
	};
	enum IECRunFlagBits
	{
		IEC_RUN_NO_MSE = 0x01,
	};
	class ImageEstimatorComparator
	{
	  public:
	    ImageEstimatorComparator() = default;
	    ~ImageEstimatorComparator() = default;
	    ImageEstimatorComparator(VkFormat format, float relWidth, float relHeight);

	    ImageEstimatorComparator(VkFormat format, VkExtent2D extent);

	    void garbageCollect();

		void cmdReset(CmdBuffer cmdBuf, Image imgLeft = nullptr, Image imgRight = nullptr);

		template <class EstimatorArgs>
	    void cmdRunEqualRate(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs argsLeft, EstimatorArgs argsRight,
	                         float *timigsLeft = nullptr, float *timigsRight = nullptr, uint32_t flags = 0);

		template <class EstimatorArgs>
	    void cmdRun(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs args, IECTarget target, float *timings = nullptr, uint32_t flags = 0);

		template <class EstimatorArgs>
	    void cmdRunEqualTime(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs argsLeft, EstimatorArgs argsRight,
	                         float *timigsLeft = nullptr, float *timigsRight = nullptr, uint32_t flags = 0);

		void     cmdShow(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea, IECTarget src, IECToneMappingArgs toneMappingArgs = {});
	    void     showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea, IECToneMappingArgs toneMappingArgs = {});
	    void     showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea);
	    void     showDiff(CmdBuffer cmdBuf, Image target);
	    float    getMSE();
	    float   *getMSEData();
	    uint32_t getMSEDataSize();
	  private:

		private:
		Image localTargetLeft, localTargetRight, localAccumulationTargetLeft, localAccumulationTargetRight;
		TimeQueryManager tqManager;
	    bool             timeQueryFinished = true;
	    MSEComputeResources mseResources;
		Buffer mseBuffer;
		bool isInitialized = false;

		float totalTimingLeft = 0.0f;
		float totalTimingRight = 0.0f;

		uint32_t invocationCountLeft = 0;
		uint32_t invocationCountRight = 0;

		std::vector<float> mse;
		std::vector<float> mseTimings;
    };
    template <class EstimatorArgs>
	void cmdRun(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs args, IECTarget target, float* timings, uint32_t flags)
	{

		Image &localTarget = target == IEC_TARGET_LEFT ? localTargetLeft : localTargetRight;
		Image &localAccumulationTarget = target == IEC_TARGET_LEFT ? localAccumulationTargetLeft : localAccumulationTargetRight;
		float &totalTiming = target == IEC_TARGET_LEFT ? totalTimingLeft : totalTimingRight;
		uint32_t &invocationCount = target == IEC_TARGET_LEFT ? invocationCountLeft : invocationCountRight;

	    cmdFill(cmdBuf, localTarget, VK_IMAGE_LAYOUT_GENERAL, glm::vec4(0.0f));
	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
	    if (timeQueryFinished)
	    {
		    tqManager.cmdResetQueryPool(cmdBuf);
		    tqManager.startTiming(cmdBuf, target, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			estimator(cmdBuf, localTarget, args);
		    tqManager.endTiming(cmdBuf, target, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	    }
	    else
	    {
		    estimator(cmdBuf, localTarget, args);
	    }
	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	    timeQueryFinished = tqManager.updateTimings();
	    totalTiming += tqManager.timings[0];
	    invocationCount++;
	    getCmdAccumulate(localTarget, localAccumulationTarget, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
	    if (timings)
	    {
		    *timings = totalTiming / invocationCount;
	    }
	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		if (!(flags & IEC_RUN_NO_MSE))
		{
			cmdComputeMSE(cmdBuf, localTarget, localAccumulationTarget, mseBuffer, &mseResources);
			mse.push_back(getMSE());
		    mseTimings.push_back(totalTiming);
		}
	}

	template <class EstimatorArgs>
    inline void ImageEstimatorComparator::cmdRunEqualTime(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator,
                                                 EstimatorArgs argsLeft, EstimatorArgs argsRight,
                                                 float *timigsLeft, float *timigsRight, uint32_t flags)
    {
	    bool runLeft  = timeQueryFinished || totalTimingLeft <= totalTimingRight;
	    bool runRight = timeQueryFinished || totalTimingRight <= totalTimingLeft;

		cmdFill(cmdBuf, localTargetRight, VK_IMAGE_LAYOUT_GENERAL, glm::vec4(0.0f));
	    cmdFill(cmdBuf, localTargetLeft, VK_IMAGE_LAYOUT_GENERAL, glm::vec4(0.0f));
	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_WRITE_BIT);
	    if (timeQueryFinished)
	    {
		    tqManager.cmdResetQueryPool(cmdBuf);
		    tqManager.startTiming(cmdBuf, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		    if (runLeft)
		    {
				estimator(cmdBuf, localTargetLeft, argsLeft);
		    }

		    tqManager.endTiming(cmdBuf, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		    tqManager.startTiming(cmdBuf, 1, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			if (runRight)
		    {
				estimator(cmdBuf, localTargetRight, argsRight);
		    }

		    tqManager.endTiming(cmdBuf, 1, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	    }
	    else
	    {
		    if (runLeft)
		    {
			    estimator(cmdBuf, localTargetLeft, argsLeft);
		    }
		    if (runRight)
		    {
				estimator(cmdBuf, localTargetRight, argsRight);
		    }
	    }
	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	    timeQueryFinished = tqManager.updateTimings();

	    

		if (runLeft)
		{
		    totalTimingLeft += tqManager.timings[0];
			invocationCountLeft++;
			getCmdAccumulate(localTargetLeft, localAccumulationTargetLeft, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
		}
	    if (runRight)
		{
		    totalTimingRight += tqManager.timings[1];
			invocationCountRight++;
			getCmdAccumulate(localTargetRight, localAccumulationTargetRight, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
	    }

		if (timigsLeft)
	    {
		    if (invocationCountLeft > 0)
			{
			    *timigsLeft = totalTimingLeft / invocationCountLeft;
		    }
		    else
		    {
			    *timigsLeft = 0.0f;
		    }
	    }
	    if (timigsRight)
	    {
		    if (invocationCountRight > 0)
			{
			    *timigsRight = totalTimingRight / invocationCountRight;
		    }
		    else
		    {
			    *timigsRight = 0.0f;
		    }
	    }


	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	    if (!(flags & IEC_RUN_NO_MSE))
		{
			cmdComputeMSE(cmdBuf, localAccumulationTargetLeft, localAccumulationTargetRight, mseBuffer, &mseResources);
			mse.push_back(getMSE());
		    mseTimings.push_back((totalTimingLeft + totalTimingRight) * 0.5);
		}
    }

    template <class EstimatorArgs>
    inline void ImageEstimatorComparator::cmdRunEqualRate(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator,
                                                 EstimatorArgs argsLeft, EstimatorArgs argsRight,
                                                          float *timigsLeft, float *timigsRight, uint32_t flags)
    {
		if (timeQueryFinished)
		{
			tqManager.cmdResetQueryPool(cmdBuf);
			tqManager.startTiming(cmdBuf, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			estimator(cmdBuf, localTargetLeft, argsLeft);
			tqManager.endTiming(cmdBuf, 0, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

			tqManager.startTiming(cmdBuf, 1, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
			estimator(cmdBuf, localTargetRight, argsRight);
			tqManager.endTiming(cmdBuf, 1, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		}
		else
		{
		    estimator(cmdBuf, localTargetLeft, argsLeft);
		    estimator(cmdBuf, localTargetRight, argsRight);
		}
	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	    timeQueryFinished = tqManager.updateTimings();
		if (timigsLeft)
		{
		    *timigsLeft = tqManager.timings[0];
		}
	    if (timigsRight)
	    {
		    *timigsRight = tqManager.timings[1];
	    }
	    getCmdAccumulate(localTargetLeft, localAccumulationTargetLeft, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);
	    getCmdAccumulate(localTargetRight, localAccumulationTargetRight, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL).exec(cmdBuf);

	    cmdBarrier(cmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	    if (!(flags & IEC_RUN_NO_MSE))
		{
			cmdComputeMSE(cmdBuf, localAccumulationTargetLeft, localAccumulationTargetRight, mseBuffer, &mseResources);
			mse.push_back(getMSE());
		    mseTimings.push_back((totalTimingLeft + totalTimingRight) * 0.5);
		}
    }

    }        // namespace vka