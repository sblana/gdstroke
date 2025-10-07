#pragma once

#include "gdstroke_shader_interface.hpp"
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>

using namespace godot;

class GdstrokeEffect : public CompositorEffect {
	GDCLASS(GdstrokeEffect, CompositorEffect)

	enum Shader : int32_t {
		SHADER_TEST_BUFFER_SIZES = 0,
		SHADER_MAX,
	};

private:
	bool _ready;
	RID _compiled_shaders[Shader::SHADER_MAX];
	RID _pipelines[Shader::SHADER_MAX];
	GdstrokeShaderInterface::SceneInterfaceSet scene_interface_set;
	GdstrokeShaderInterface::MeshInterfaceSet mesh_interface_set;

protected:
	static void _bind_methods();

public:
	GdstrokeEffect();
	~GdstrokeEffect() override = default;

	void _render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) override;
};
