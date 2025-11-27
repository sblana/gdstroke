#pragma once

#include "gdstroke_shader_interface.hpp"
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#define SHADERS_DECLARATIONS_ONLY
#include "gpu/shaders.hpp"

using namespace godot;

class GdstrokeEffect : public CompositorEffect {
	GDCLASS(GdstrokeEffect, CompositorEffect)

private:
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
	GdstrokeShaderInterface::StrokeRenderingResources stroke_rendering_resources = {};

	using DispatchIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DispatchIndirectCommands;
	using DrawIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DrawIndirectCommands;

	void bind_sets(RenderingDevice *p_rd, int64_t p_compute_list) const;
	void bind_sets_commander(RenderingDevice *p_rd, int64_t p_compute_list) const;
	void bind_sets_debug(RenderingDevice *p_rd, int64_t p_compute_list) const;

	void _compile_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name);

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
	uint32_t get_config_min_segment_length() const;
	void set_config_min_segment_length(uint32_t p_value);
	float get_config_stroke_width() const;
	void  set_config_stroke_width(float p_value);
	float get_config_stroke_width_factor_start() const;
	void  set_config_stroke_width_factor_start(float p_value);
	float get_config_stroke_width_factor_end() const;
	void  set_config_stroke_width_factor_end(float p_value);
};
