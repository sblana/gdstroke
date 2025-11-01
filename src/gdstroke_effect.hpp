#pragma once

#include "gdstroke_shader_interface.hpp"
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>

using namespace godot;

class GdstrokeEffect : public CompositorEffect {
	GDCLASS(GdstrokeEffect, CompositorEffect)

	enum Shader : int32_t {
		SHADER_DUMMY = 0,
		SHADER_DUMMY_COMMANDER,
		SHADER_DUMMY_DEBUG,
		SHADER_CR_CED_FACE_ORIENTATION,
		SHADER_CR_CED_DETECTION,
		SHADER_CR_CED_ALLOCATION,
		SHADER_CR_CED_SCATTER,
		SHADER_CR_FG_FIRST_COMMANDER,
		SHADER_CR_FG_FRAG_COUNTS,
		SHADER_CR_FG_ALLOCATION,
		SHADER_CR_FG_SCATTER,
		SHADER_CR_CPG_FIRST_COMMANDER,
		SHADER_CR_CPG_SOFT_DEPTH_TEST,
		SHADER_CR_CPG_PRE_ALLOC,
		SHADER_CR_CPG_ALLOCATION,
		SHADER_CR_CPG_SCATTER,
		SHADER_CR_CPG_SECOND_COMMANDER,
		SHADER_CR_CPG_HARD_DEPTH_TEST,
		SHADER_CR_CPG_DECODE,
		SHADER_CC_PEG_FIRST_COMMANDER,
		SHADER_CC_PEG_GENERATION,
		SHADER_CC_LB_INIT,
		SHADER_CC_LB_WYLLIE,
		SHADER_CC_LB_SCATTER,
		SHADER_CC_LR_INIT,
		SHADER_CC_LR_WYLLIE,
		SHADER_CC_LR_SCATTER,
		SHADER_CC_D_INIT,
		SHADER_CC_D_HEAD_ALLOCATION,
		SHADER_CC_D_COMPACTED_ALLOCATION,
		SHADER_CC_D_COMPACTED_SCATTER,
		SHADER_CC_D_FIRST_COMMANDER,
		SHADER_SE_MF_INIT,
		SHADER_SE_MF_SMOOTHING,
		SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS,
		SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS,
		SHADER_DEBUG_DISPLAY_SPARSE_PIXEL_EDGES,
		SHADER_DEBUG_DISPLAY_COMPACTED_PIXEL_EDGES,
		SHADER_MAX,
	};

private:
	static void const *shader_to_embedded_data[GdstrokeEffect::Shader::SHADER_MAX];
	bool _ready;
	bool _debug_enabled = true;
	RID _compiled_shaders[Shader::SHADER_MAX];
	RID _pipelines[Shader::SHADER_MAX];
	GdstrokeShaderInterface::SceneInterfaceSet scene_interface_set = {};
	GdstrokeShaderInterface::CommandInterfaceSet command_interface_set = {};
	GdstrokeShaderInterface::MeshInterfaceSet mesh_interface_set = {};
	GdstrokeShaderInterface::ContourInterfaceSet contour_interface_set = {};
	GdstrokeShaderInterface::PixelEdgeInterfaceSet pixel_edge_interface_set = {};

	GdstrokeShaderInterface::DebugInterfaceSet debug_interface_set = {};

	GdstrokeShaderInterface::HardDepthTestResources hard_depth_test_resources = {};

	using DispatchIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DispatchIndirectCommands;
	using DrawIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DrawIndirectCommands;

	void bind_sets(RenderingDevice *p_rd, int64_t p_compute_list) const;
	void bind_sets_commander(RenderingDevice *p_rd, int64_t p_compute_list) const;
	void bind_sets_debug(RenderingDevice *p_rd, int64_t p_compute_list) const;

	void _compile_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name);
	void _compile_draw_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name);

protected:
	static void _bind_methods();

public:
	GdstrokeEffect();
	~GdstrokeEffect() override = default;

	void _render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) override;

	float get_config_depth_bias() const;
	void  set_config_depth_bias(float p_value);
	bool get_config_use_soft_depth_test_modification() const;
	void set_config_use_soft_depth_test_modification(bool p_value);
};
