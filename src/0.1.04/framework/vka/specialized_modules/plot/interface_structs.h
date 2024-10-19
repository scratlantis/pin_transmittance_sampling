#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include <imgui.h>
using namespace glm;
namespace vka
{
	namespace shader_plot
	{
		#include <vka/shaders/lib/plot/interface_structs.glsl>
	}

	template <typename T>
    struct render_plot;

	template <>
    struct render_plot<shader_plot::GLSLYListPlot>
	{
	    void operator()(const shader_plot::GLSLYListPlot &plot, void *plotData)
	    {
			std::string label = "none";
		    if (plot.count > 0)
		    {
			    switch (plot.dataType)
			    {
				    case PLOT_DATA_TYPE_FLOAT:
					    if (ImPlot::BeginPlot("Line Plots"))
					    {
						    ImPlot::SetupAxes("x", "y");
							ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
						    ImPlot::PlotShaded("f(x)", static_cast<float *>(plotData), static_cast<int>(plot.count), -INFINITY, plot.stride);
						    ImPlot::PopStyleVar();
						    ImPlot::PlotLine("f(x)", static_cast<float *>(plotData), static_cast<int>(plot.count), plot.stride);
						    ImPlot::EndPlot();
					    }
					    break;
				    default:
					    DEBUG_BREAK;
					    break;
			    }
		    }
	    }
	};
}		// namespace vka