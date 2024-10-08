#pragma once
#include <vka/vka.h>
#include <medium/Medium.h>
#include "config.h"


extern std::vector<GVar *> gVars;

typedef uint32_t  uint;
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;
typedef glm::mat4 mat4;
//#include "shaders/interface.glsl"
#include "shaders/interface_structs.glsl"

static GLSLFrame defaultFrame(VkExtent2D extent, uint32_t frameIdx)
{
	GLSLFrame frame;
	if (gvar_fixed_seed.val.v_bool)
	{
		frame.idx = gvar_seed.val.v_uint;
	}
	else if (gvar_path_sampling_event.val.v_bool)
	{
		frame.idx = gvar_screen_cursor_seed.val.v_uint;
	}
	else
	{
		frame.idx = frameIdx;
	}
	frame.width         = extent.width;
	frame.height        = extent.height;
	frame.projection    = glm::perspective(glm::radians(60.0f), (float) extent.width / (float) extent.height, 0.1f, 500.0f);
	frame.invProjection = glm::inverse(frame.projection);
	return frame;
}

static GLSLView cameraView(Camera* cam)
{
	GLSLView view;
	view.mat    = cam->getViewMatrix();
	view.invMat = glm::inverse(view.mat);
	view.pos    = vec4(cam->getPosition(), 0.0);
	return view;
}

static uint getShaderKeyState(uint key)
{
	if (gState.io.keyPressedEvent[key])
	{
		return BUTTON_PRESS_EVENT;
	}
	if (gState.io.keyReleasedEvent[key])
	{
		return BUTTON_RELEASE_EVENT;
	}
	if (gState.io.keyPressed[key])
	{
		return BUTTON_PRESSED;
	}
	return BUTTON_RELEASED;
}

static uint getShaderMouseLeftKeyState()
{
	if (gState.io.mouse.leftEvent && gState.io.mouse.leftPressed)
	{
		return BUTTON_PRESS_EVENT;
	}
	if (gState.io.mouse.leftEvent && !gState.io.mouse.leftPressed)
	{
		return BUTTON_RELEASE_EVENT;
	}
	if (gState.io.mouse.leftPressed)
	{
		return BUTTON_PRESSED;
	}
	return BUTTON_RELEASED;
}

static uint getShaderMouseRightKeyState()
{
	if (gState.io.mouse.rightEvent)
	{
		return BUTTON_PRESS_EVENT;
	}
	if (gState.io.mouse.rightPressed)
	{
		return BUTTON_PRESSED;
	}
	return BUTTON_RELEASED;
}

static GLSLParams guiParams(glm::vec2 cursorPos)
{
	GLSLParams params{};
	params.leftMB    = getShaderMouseLeftKeyState();
	params.controlKEY = getShaderKeyState(GLFW_KEY_LEFT_CONTROL);
	params.cursorPos = cursorPos;
	params.pinSampleLocation = gvar_pin_sample_location.val.v_float;
	params.pathSampling = gvar_path_sampling_event.val.bool32() | gvar_continuous_path_sampling.val.bool32();
	params.jitterPinSampleLocation = gvar_jitter_sampling_location.val.bool32();
	params.jitterPinSamplePos = gvar_jitter_sampling_pos.val.v_float;
	return params;
}

class ShaderConst
{
  public:
	Buffer ubo_frame;
	Buffer ubo_view;
	Buffer ubo_params;

	void alloc()
	{
		ubo_frame  = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLFrame));
		ubo_view   = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLFrame));
		ubo_params = createBuffer(gState.heap, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, sizeof(GLSLParams));
	}

	void free()
	{
		ubo_frame->garbageCollect();
		ubo_view->garbageCollect();
		ubo_params->garbageCollect();
	}

	void write(CmdBuffer cmdBuf, ComputeCmd &cmd, VkExtent2D extent, Camera* cam, uint32_t frameIdx, glm::vec2 cursorPos)
	{
		GLSLFrame ptFrame = defaultFrame(extent, frameIdx);
		cmdWriteCopy(cmdBuf, ubo_frame, &ptFrame, sizeof(GLSLFrame));

		GLSLView ptView = cameraView(cam);
		cmdWriteCopy(cmdBuf, ubo_view, &ptView, sizeof(GLSLView));

		GLSLParams params = guiParams(cursorPos);
		cmdWriteCopy(cmdBuf, ubo_params, &params, sizeof(GLSLParams));
	}
};

extern ShaderConst sConst;

// see template.glsl

static void bind_shader_const(ComputeCmd &cmd, const ShaderConst &sConst)
{
	cmd.pipelineDef.shaderDef.args.push_back({"SHADER_CONST_BINDING_OFFSET", static_cast<uint32_t>(cmd.descriptors.size())});
	cmd.pushDescriptor(sConst.ubo_frame, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pushDescriptor(sConst.ubo_view, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	cmd.pushDescriptor(sConst.ubo_params, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}
static void bind_scene(ComputeCmd &cmd, const USceneData* pScene)
{
	cmd.pipelineDef.shaderDef.args.push_back({"PT_USCENE_BINDING_OFFSET", static_cast<uint32_t>(cmd.descriptors.size())});
	cmd.pushDescriptor(pScene->vertexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->indexBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->modelOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->surfaceOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->materialBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->areaLightBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->tlas);
	cmd.pushDescriptor(pScene->instanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pScene->instanceOffsetBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(SamplerDefinition());
	cmd.pushDescriptor(pScene->textures, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	cmd.pushDescriptor(pScene->envMap, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	cmd.pushDescriptor(pScene->envMapPdfBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	cmd.pipelineDef.shaderDef.args.push_back({"ENVMAP_PDF_BINS_X", pScene->envMapSubdivisions.x});
	cmd.pipelineDef.shaderDef.args.push_back({"ENVMAP_PDF_BINS_Y", pScene->envMapSubdivisions.y});
	cmd.pipelineDef.shaderDef.args.push_back({"AREA_LIGHT_COUNT", pScene->areaLightCount});
}

static void bind_medium(ComputeCmd &cmd, const Medium *pMedium)
{
	cmd.pipelineDef.shaderDef.args.push_back({"PT_SCALAR_FIELD_MEDIUM_BINDING_OFFSET", static_cast<uint32_t>(cmd.descriptors.size())});
	cmd.pushDescriptor(pMedium->volumeGrid, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	cmd.pipelineDef.shaderDef.args.push_back({"PT_PIN_MEDIUM_BINDING_OFFSET", static_cast<uint32_t>(cmd.descriptors.size())});
	cmd.pushDescriptor(pMedium->pinGrid, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(pMedium->pinTransmittance, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	cmd.pipelineDef.shaderDef.args.push_back({"RAY_MARCHE_STEP_SIZE", std::to_string(gvar_raymarche_step_size.val.v_float)});
	cmd.pipelineDef.shaderDef.args.push_back({"VOLUME_RESOLUTION", std::to_string(gvar_image_resolution.val.v_uint)});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_GRID_SIZE", std::to_string(gvar_pin_grid_size.val.v_uint)});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_COUNT_PER_GRID_CELL", std::to_string(gvar_pin_count_per_grid_cell.val.v_uint)});
	cmd.pipelineDef.shaderDef.args.push_back({"PIN_TRANSMITTANCE_VALUE_COUNT", std::to_string(gvar_pin_transmittance_value_count.val.v_uint)});
}

struct ProfilerOutput
{
	Buffer lineSegmentBuffer;
	Buffer plotBuffer;
	Buffer fluxBuffer;
};

static void bind_profiler(ComputeCmd &cmd, const ProfilerOutput &output)
{
	cmd.pipelineDef.shaderDef.args.push_back({"PT_PROFILING_BINDING_OFFSET", static_cast<uint32_t>(cmd.descriptors.size())});
	cmd.pushDescriptor(output.lineSegmentBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(output.plotBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	cmd.pushDescriptor(output.fluxBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	if (gvar_write_flux.val.v_bool)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"WRITE_PIN_FLUX", ""});
	}

}

static void begin_local_descriptors(ComputeCmd &cmd)
{
	cmd.pipelineDef.shaderDef.args.push_back({"LOCAL_BINDING_OFFSET", static_cast<uint32_t>(cmd.descriptors.size())});
}

static void bind_target(ComputeCmd &cmd, Image target)
{
	cmd.pushDescriptor(target, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	cmd.pipelineDef.shaderDef.args.push_back({"FORMAT1", getGLSLFormat(target->getFormat())});
}

static void set_general_params(ComputeCmd &cmd)
{
	cmd.pipelineDef.shaderDef.args.push_back({"MAX_BOUNCES", gvar_max_bounce.val.v_uint});
	cmd.pipelineDef.shaderDef.args.push_back({"MIN_PIN_BOUNCE", gvar_min_pin_bounce.val.v_uint});
	cmd.pipelineDef.shaderDef.args.push_back({"PLOT_OFFSET_PINS", 0});
	cmd.pipelineDef.shaderDef.args.push_back({"PLOT_OFFSET_SCALAR_FIELD", MAX_PLOT_POINTS});
	if (gvar_medium_xray_line_segments.val.v_bool)
	{
		cmd.pipelineDef.shaderDef.args.push_back({"XRAY_LINE_VISION", ""});
	}
}

namespace vka
{
template <>
struct vertex_type<GLSLVertex>
{
	VertexDataLayout data_layout()
	{
		VertexDataLayout layout{};
		layout.formats =
		    {
		        VK_FORMAT_R32G32B32_SFLOAT,
		        VK_FORMAT_R32G32B32_SFLOAT,
		        VK_FORMAT_R32G32_SFLOAT,
		    };
		layout.offsets =
		    {
		        offsetof(GLSLVertex, pos),
		        offsetof(GLSLVertex, normal),
		        offsetof(GLSLVertex, uv)};
		layout.stride = sizeof(GLSLVertex);
		return layout;
	}
	void load_obj(Buffer vertexBuffer, const std::vector<ObjVertex> &vertexList)
	{
		vertexBuffer->changeSize(vertexList.size() * sizeof(GLSLVertex));
		vertexBuffer->changeMemoryType(VMA_MEMORY_USAGE_CPU_ONLY);
		vertexBuffer->recreate();
		GLSLVertex *vertexData = static_cast<GLSLVertex *>(vertexBuffer->map());
		for (size_t i = 0; i < vertexList.size(); i++)
		{
			vertexData[i].pos    = vertexList[i].v;
			vertexData[i].normal = vertexList[i].vn;
			vertexData[i].uv     = vertexList[i].vt;
		}
	}
};
}        // namespace vka

namespace vka
{
namespace pbr
{
template <>
struct material_type<GLSLMaterial>
{
	GLSLMaterial load_mtl(WavefrontMaterial mtl, std::unordered_map<std::string, uint32_t> &textureIndexMap)
	{
		GLSLMaterial material{};
		material.albedo    = mtl.diffuse;
		material.specular  = mtl.specular;
		material.roughness = mtl.roughness;
		material.emission  = mtl.emission;
		material.f0        = 0.1;        // How do we get this?
		return material;
	}
};

template <>
struct instance_type<GLSLInstance>
{
	ComputeCmd get_cmd_write_tlas_instance(Buffer instanceBuffer, Buffer tlasInstanceBuffer, uint32_t instanceCount)
	{
		ComputeCmd cmd(instanceCount, shaderPath + "misc/instance_to_tlas_instance.comp", {{"INPUT_SIZE", instanceCount}});
		cmd.pushDescriptor(instanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		cmd.pushDescriptor(tlasInstanceBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		return cmd;
	}
};

}        // namespace pbr
}        // namespace vka