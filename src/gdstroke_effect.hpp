#pragma once

#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/compositor_effect.hpp>

using namespace godot;

class GdstrokeEffect : public CompositorEffect {
	GDCLASS(GdstrokeEffect, CompositorEffect)

	enum Shader : int32_t {
		SHADER_ = 0,
		SHADER_MAX,
	};

protected:
	static void _bind_methods();

public:
	GdstrokeEffect() = default;
	~GdstrokeEffect() override = default;

	void _render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) override;

private:
	RID compiled_shaders[Shader::SHADER_MAX];
	RID pipelines[Shader::SHADER_MAX];
};
