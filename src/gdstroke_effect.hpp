#pragma once

#include "gdstroke_shader_interface.hpp"
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>
#define SHADERS_DECLARATIONS_ONLY
#include "gpu/shaders.hpp"

using namespace godot;

class GdstrokeEffect : public CompositorEffect {
	GDCLASS(GdstrokeEffect, CompositorEffect)

public:
	enum RasterMethod : int64_t {
		RASTER_METHOD_BRESENHAM = 0,
		RASTER_METHOD_DDA,
		RASTER_METHOD_MAX
	};

private:
	int64_t _id = 0;
	RasterMethod _raster_method = RasterMethod::RASTER_METHOD_BRESENHAM;
	bool _ready = false;
	bool _debug_enabled = true;
	RID _compiled_shaders[Shader::SHADER_MAX];
	RID _pipelines[Shader::SHADER_MAX];
	GdstrokeShaderInterface::SceneInterfaceSet _scene_interface_set = {};
	GdstrokeShaderInterface::CommandInterfaceSet _command_interface_set = {};
	GdstrokeShaderInterface::MeshInterfaceSet _mesh_interface_set = {};
	GdstrokeShaderInterface::ContourInterfaceSet _contour_interface_set = {};
	GdstrokeShaderInterface::PixelEdgeInterfaceSet _pixel_edge_interface_set = {};

	GdstrokeShaderInterface::ShaderAPIInterfaceSet _shader_api_interface_set = {};

	GdstrokeShaderInterface::DebugInterfaceSet _debug_interface_set = {};

	GdstrokeShaderInterface::HardDepthTestResources _hard_depth_test_resources = {};
	GdstrokeShaderInterface::StrokeRenderingResources _stroke_rendering_resources = {};

	using DispatchIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DispatchIndirectCommands;
	using DrawIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DrawIndirectCommands;

	void _bind_sets(RenderingDevice *p_rd, int64_t p_compute_list) const;
	void _bind_sets_commander(RenderingDevice *p_rd, int64_t p_compute_list) const;
	void _bind_sets_debug(RenderingDevice *p_rd, int64_t p_compute_list) const;

	void _compile_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name);

protected:
	static void _bind_methods();

public:
	GdstrokeEffect();
	~GdstrokeEffect() override = default;

	void _render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) override;

	int64_t get_id() const;
	void    set_id(int64_t p_value);

	RasterMethod get_raster_method() const;
	void         set_raster_method(RasterMethod p_value);
	float get_config_depth_bias() const;
	void  set_config_depth_bias(float p_value);
	bool get_config_use_soft_depth_test_modification() const;
	void set_config_use_soft_depth_test_modification(bool p_value);
	uint32_t get_config_min_segment_length() const;
	void set_config_min_segment_length(uint32_t p_value);

	RID     get_stroke_shader_uniform_set_rid();
	int64_t get_stroke_shader_uniform_set_slot() const;

	void draw_indirect_stroke_shader(RenderingDevice *p_rd, int64_t p_draw_list);
};

VARIANT_ENUM_CAST(GdstrokeEffect::RasterMethod)
