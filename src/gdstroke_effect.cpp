#include "gdstroke_effect.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/render_scene_data.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rendering_server.hpp>

#include "rd_util.hpp"
#include "gdstroke_server.hpp"

#include "gen/dummy.spv.h"
#include "gen/cr__ced__face_orientation.spv.h"
#include "gen/cr__ced__detection.spv.h"
#include "gen/cr__ced__allocation.spv.h"
#include "gen/cr__ced__scatter.spv.h"


using namespace godot;

void GdstrokeEffect::_bind_methods() {}

GdstrokeEffect::GdstrokeEffect() {
	this->set_effect_callback_type(EffectCallbackType::EFFECT_CALLBACK_TYPE_POST_OPAQUE);
	this->_ready = false;
	this->scene_interface_set = {};
	this->mesh_interface_set = {};
	this->contour_interface_set = {};
}

void GdstrokeEffect::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();

	if (!this->_ready) {
		if (GdstrokeServer::get_singleton()->get_contour_instance() == nullptr)
			return;
		_ready = true;

		this->_compiled_shaders[Shader::SHADER_DUMMY] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_dummy, _STR(SHADER_DUMMY));

		this->_compiled_shaders[Shader::SHADER_CR_CED_FACE_ORIENTATION] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__face_orientation, _STR(SHADER_CR_CED_FACE_ORIENTATION));
		this->_compiled_shaders[Shader::SHADER_CR_CED_DETECTION       ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__detection,        _STR(SHADER_CR_CED_DETECTION       ));
		this->_compiled_shaders[Shader::SHADER_CR_CED_ALLOCATION      ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__allocation,       _STR(SHADER_CR_CED_ALLOCATION      ));
		this->_compiled_shaders[Shader::SHADER_CR_CED_SCATTER         ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__scatter,          _STR(SHADER_CR_CED_SCATTER         ));


		this->_pipelines[Shader::SHADER_DUMMY] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_DUMMY]);

		this->_pipelines[Shader::SHADER_CR_CED_FACE_ORIENTATION] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_FACE_ORIENTATION]);
		this->_pipelines[Shader::SHADER_CR_CED_DETECTION       ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_DETECTION       ]);
		this->_pipelines[Shader::SHADER_CR_CED_ALLOCATION      ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_ALLOCATION      ]);
		this->_pipelines[Shader::SHADER_CR_CED_SCATTER         ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_SCATTER         ]);

		this->scene_interface_set.create_resources(rd, p_render_data);
		this->mesh_interface_set.create_resources(rd, p_render_data);
		this->contour_interface_set.create_resources(rd, p_render_data);
	}

	this->scene_interface_set.update_resources(rd, p_render_data);
	this->scene_interface_set.make_bindings();
	ERR_FAIL_COND(!this->scene_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->mesh_interface_set.update_resources(rd, p_render_data);
	this->mesh_interface_set.make_bindings();
	ERR_FAIL_COND(!this->mesh_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->contour_interface_set.update_resources(rd, p_render_data);
	this->contour_interface_set.make_bindings();
	ERR_FAIL_COND(!this->contour_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());


	uint32_t num_verts = GdstrokeServer::get_singleton()->get_contour_mesh().vertex_buffer.size();
	uint32_t num_edges = GdstrokeServer::get_singleton()->get_contour_mesh().edge_to_vertex_buffer.size();
	uint32_t num_faces = GdstrokeServer::get_singleton()->get_contour_mesh().face_to_vertex_buffer.size();

	int64_t list;
	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DUMMY]);
	this->  scene_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   mesh_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->contour_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_FACE_ORIENTATION]);
	this->  scene_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   mesh_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->contour_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	rd->compute_list_dispatch(list, udiv_ceil(num_faces, 64), 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_DETECTION]);
	this->  scene_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   mesh_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->contour_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	rd->compute_list_dispatch(list, udiv_ceil(num_edges, 64), 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_ALLOCATION]);
	this->  scene_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   mesh_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->contour_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_SCATTER]);
	this->  scene_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   mesh_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->contour_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	rd->compute_list_dispatch(list, udiv_ceil(num_edges, 64), 1, 1);
	rd->compute_list_end();
}
