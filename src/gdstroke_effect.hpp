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
		SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS,
		SHADER_MAX,
	};

private:
	static void const *shader_to_embedded_data[GdstrokeEffect::Shader::SHADER_MAX];
	bool _ready;
	bool _debug_enabled = true;
	RID _compiled_shaders[Shader::SHADER_MAX];
	RID _pipelines[Shader::SHADER_MAX];
	GdstrokeShaderInterface::SceneInterfaceSet scene_interface_set;
	GdstrokeShaderInterface::CommandInterfaceSet command_interface_set;
	GdstrokeShaderInterface::MeshInterfaceSet mesh_interface_set;
	GdstrokeShaderInterface::ContourInterfaceSet contour_interface_set;

	GdstrokeShaderInterface::DebugInterfaceSet debug_interface_set;

	using DispatchIndirectCommands = GdstrokeShaderInterface::CommandInterfaceSet::DispatchIndirectCommands;

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
};
