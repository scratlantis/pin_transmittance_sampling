#pragma once
#include <vka/advanced_utility/complex_commands.h>
#include <implot.h>
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
		    std::string label = "Line Plot " + std::to_string(plot.plotID);
		    if (plot.count > 0)
		    {
			    switch (plot.dataType)
			    {
				    case PLOT_DATA_TYPE_FLOAT:
					    if (ImPlot::BeginPlot(label.c_str()))
					    {
						    ImPlot::SetupAxes("x", "y");
							ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
						    ImPlot::PlotShaded("f(x)", static_cast<float *>(plotData) + plot.offset, static_cast<int>(plot.count), -INFINITY, plot.stride, 0.0, 0, 0);
						    ImPlot::PopStyleVar();
						    ImPlot::PlotLine("f(x)", static_cast<float *>(plotData) + plot.offset, static_cast<int>(plot.count), plot.stride, 0.0, 0, 0);
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

	template <>
    struct render_plot<shader_plot::GLSLHistogram>
    {
	    void operator()(const shader_plot::GLSLHistogram &plot, void *plotData)
	    {
		    std::string label = "Line Plot " + std::to_string(plot.plotID);
		    if (plot.count > 0)
		    {
			    switch (plot.dataType)
			    {
				    case PLOT_DATA_TYPE_FLOAT:
					    if (ImPlot::BeginPlot(label.c_str()))
					    {
						    ImPlot::SetupAxes("x", "y", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
						    ImPlot::PlotHistogram("", static_cast<float *>(plotData) + plot.offset, static_cast<int>(plot.count), plot.bins, 1.0,
						                          plot.rMin != plot.rMax ? ImPlotRange(plot.rMin, plot.rMax) : ImPlotRange(), ImPlotHistogramFlags_NoOutliers);
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