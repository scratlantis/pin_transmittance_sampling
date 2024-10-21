#ifndef GENERATE_PT_STATE_HISTOGRAM_H
#define GENERATE_PT_STATE_HISTOGRAM_H

#extension GL_EXT_debug_printf : enable

#ifdef PT_PLOT_INTERFACE_SMD_H
#ifdef HISTOGRAM_SMD_H
#ifdef PT_SHADER_STATE_H

//#include "../../modules/pt_plot/pt_plot_interface_smd.glsl"

void testGetter()
{
	GLSLPtPlotOptions options = ptPlotGetOptions();
	if(options.writeTotalContribution != 0)
	{
		//debugPrintfEXT("Was here!\n");
	}

	initHist(200, 10000, 0.0, 5.0);
	if(pt_state.bounce > 1)
	{
		float t = distance(indirect_bounces[0].bouncePos, indirect_bounces[1].bouncePos);
		setHistValue(t, getFrameIdx());
	}

	GLSLPtPlot plot;
	plot.totalContribution = 0;
	ptPlotSetPlot(plot);
	ptPlot.totalContribution = 0;
}

#endif
#endif
#endif



#endif