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

		

	    void  showSplitView(CmdBuffer cmdBuf, Image target, float splittCoef, VkRect2D_OP targetArea, IECToneMappingArgs toneMappingArgs = {});
		void showDiff(CmdBuffer cmdBuf, Image target, VkRect2D_OP targetArea);
	    float getMSE();

		private:
		Image localTargetLeft, localTargetRight, localAccumulationTargetLeft, localAccumulationTargetRight;
		TimeQueryManager tqManager;
	    bool             timeQueryFinished = true;
	    MSEComputeResources mseResources;
		Buffer mseBuffer;
    };

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