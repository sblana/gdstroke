#include "gdstroke_effect.hpp"

#include <godot_cpp/classes/render_scene_data.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include "gdstroke_server.hpp"

#include "gen/test_buffer_sizes.spv.h"


using namespace godot;

void GdstrokeEffect::_bind_methods() {}

GdstrokeEffect::GdstrokeEffect() {
	this->set_effect_callback_type(EffectCallbackType::EFFECT_CALLBACK_TYPE_POST_OPAQUE);
	this->_ready = false;
	this->scene_interface_set = {};
}

void GdstrokeEffect::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();

	if (!this->_ready) {
		if (GdstrokeServer::get_singleton()->get_contour_instance() == nullptr)
			return;
		_ready = true;
		PackedByteArray spirv_data = PackedByteArray();
		for (int i = 0; i < SHADER_SPV_test_buffer_sizes_LENGTH; ++i) {
			spirv_data.append(SHADER_SPV_test_buffer_sizes[i]);
		}
		RDShaderSPIRV *spirv = memnew(RDShaderSPIRV);
		spirv->set_stage_bytecode(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE, spirv_data);
		this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES] = rd->shader_create_from_spirv(spirv, _STR(SHADER_TEST_BUFFER_SIZES));

		this->_pipelines[Shader::SHADER_TEST_BUFFER_SIZES] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES]);

		this->scene_interface_set.create_resources(rd, p_render_data);
		this->mesh_interface_set.create_resources(rd, p_render_data);
	}

	this->scene_interface_set.update_resources(rd, p_render_data);
	this->scene_interface_set.make_bindings();
	ERR_FAIL_COND(!this->scene_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES]).is_valid());

	this->mesh_interface_set.update_resources(rd, p_render_data);
	this->mesh_interface_set.make_bindings();
	ERR_FAIL_COND(!this->mesh_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES]).is_valid());


	int64_t list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_TEST_BUFFER_SIZES]);
	this->scene_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES]);
	this->mesh_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_TEST_BUFFER_SIZES]);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();
}
