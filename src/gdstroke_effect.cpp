#include "gdstroke_effect.hpp"

#include <godot_cpp/classes/render_scene_data.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include "gen/test_buffer_sizes.spv.h"


using namespace godot;

void GdstrokeEffect::_bind_methods() {}

GdstrokeEffect::GdstrokeEffect() {
	this->set_effect_callback_type(EffectCallbackType::EFFECT_CALLBACK_TYPE_POST_OPAQUE);
	this->_ready = false;
}

void GdstrokeEffect::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();

	if (!this->_ready) {
		_ready = true;
		PackedByteArray spirv_data = PackedByteArray();
		for (int i = 0; i < SHADER_SPV_test_buffer_sizes_LENGTH; ++i) {
			spirv_data.append(SHADER_SPV_test_buffer_sizes[i]);
		}
		RDShaderSPIRV *spirv = memnew(RDShaderSPIRV);
		spirv->set_stage_bytecode(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE, spirv_data);
		this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES] = rd->shader_create_from_spirv(spirv, _STR(SHADER_TEST_BUFFER_SIZES));

		this->_pipelines[Shader::SHADER_TEST_BUFFER_SIZES] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES]);

		config_uniform = rd->uniform_buffer_create(sizeof(float) * 4);
	}

	Ref<RDUniform> uniform;
	TypedArray<Ref<RDUniform>> set0_uniforms;
	{
		// SceneDataUniform
		uniform = Ref(memnew(RDUniform));
		uniform->set_binding(0);
		uniform->set_uniform_type(RenderingDevice::UniformType::UNIFORM_TYPE_UNIFORM_BUFFER);
		uniform->add_id(p_render_data->get_render_scene_data()->get_uniform_buffer());
		set0_uniforms.append(uniform);

		// ConfigUniform
		uniform = Ref(memnew(RDUniform));
		uniform->set_binding(1);
		uniform->set_uniform_type(RenderingDevice::UniformType::UNIFORM_TYPE_UNIFORM_BUFFER);
		uniform->add_id(this->config_uniform);
		set0_uniforms.append(uniform);
	}

	int64_t list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_TEST_BUFFER_SIZES]);
	rd->compute_list_bind_uniform_set(list, UniformSetCacheRD::get_cache(this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES], 0, set0_uniforms), 0);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();
}
