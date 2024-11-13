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
	class ImageEstimatorComparator
	{
	  public:
	    ImageEstimatorComparator() = default;
	    ~ImageEstimatorComparator() = default;
	    ImageEstimatorComparator(VkFormat format, float relWidth, float relHeight);

		void cmdReset(CmdBuffer cmdBuf);

		template <class EstimatorArgs>
	    void cmdRun(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs argsLeft, EstimatorArgs argsRight, float *timigsLeft, float *timigsRight);

		template <class EstimatorArgs>
	    void cmdRunEqualTime(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator, EstimatorArgs argsLeft, EstimatorArgs argsRight, float *timigsLeft, float *timigsRight);


	    void  showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea, IECToneMappingArgs toneMappingArgs = {});
	    void     showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea);
	    void     showDiff(CmdBuffer cmdBuf, Image target);
	    float getMSE();
		float* getMSEData();
		uint32_t getMSEDataSize();
	  private:

		private:
		Image localTargetLeft, localTargetRight, localAccumulationTargetLeft, localAccumulationTargetRight;
		TimeQueryManager tqManager;
	    bool             timeQueryFinished = true;
	    MSEComputeResources mseResources;
		Buffer mseBuffer;

		float totalTimingLeft = 0.0f;
		float totalTimingRight = 0.0f;

		uint32_t invocationCountLeft = 0;
		uint32_t invocationCountRight = 0;

		std::vector<float> mse;
    };

	template <class EstimatorArgs>
    inline void ImageEstimatorComparator::cmdRunEqualTime(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator,
                                                 EstimatorArgs argsLeft, EstimatorArgs argsRight,
                                                 float *timigsLeft = nullptr, float *timigsRight = nullptr)
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

	    cmdComputeMSE(cmdBuf, localAccumulationTargetLeft, localAccumulationTargetRight, mseBuffer, &mseResources);
	    mse.push_back(getMSE());
    }

    template <class EstimatorArgs>
    inline void ImageEstimatorComparator::cmdRun(CmdBuffer cmdBuf, std::function<void(CmdBuffer, Image, EstimatorArgs)> estimator,
                                                 EstimatorArgs argsLeft, EstimatorArgs argsRight,
                                                 float *timigsLeft = nullptr, float *timigsRight = nullptr)
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

		cmdComputeMSE(cmdBuf, localAccumulationTargetLeft, localAccumulationTargetRight, mseBuffer, &mseResources);
    }

    }        // namespace vka