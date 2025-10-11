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
#include "gen/dummy_commander.spv.h"
#include "gen/dummy_debug.spv.h"
#include "gen/cr__ced__face_orientation.spv.h"
#include "gen/cr__ced__detection.spv.h"
#include "gen/cr__ced__allocation.spv.h"
#include "gen/cr__ced__scatter.spv.h"
#include "gen/cr__fg__first_commander.spv.h"
#include "gen/cr__fg__frag_counts.spv.h"
#include "gen/cr__fg__allocation.spv.h"
#include "gen/cr__fg__scatter.spv.h"
#include "gen/cr__cpg__first_commander.spv.h"
#include "gen/cr__cpg__soft_depth_test.spv.h"
#include "gen/debug__display_contour_fragments.spv.h"


using namespace godot;

void const *GdstrokeEffect::shader_to_embedded_data[Shader::SHADER_MAX] = {
	&SHADER_SPV_dummy,
	&SHADER_SPV_dummy_commander,
	&SHADER_SPV_dummy_debug,
	&SHADER_SPV_cr__ced__face_orientation,
	&SHADER_SPV_cr__ced__detection,
	&SHADER_SPV_cr__ced__allocation,
	&SHADER_SPV_cr__ced__scatter,
	&SHADER_SPV_cr__fg__first_commander,
	&SHADER_SPV_cr__fg__frag_counts,
	&SHADER_SPV_cr__fg__allocation,
	&SHADER_SPV_cr__fg__scatter,
	&SHADER_SPV_cr__cpg__first_commander,
	&SHADER_SPV_cr__cpg__soft_depth_test,
	&SHADER_SPV_debug__display_contour_fragments,
};

void GdstrokeEffect::bind_sets(RenderingDevice *p_rd, int64_t p_compute_list) const {
	this->  scene_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   mesh_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->contour_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
}

void GdstrokeEffect::bind_sets_commander(RenderingDevice *p_rd, int64_t p_compute_list) const {
	this->command_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY_COMMANDER]);
}

void GdstrokeEffect::bind_sets_debug(RenderingDevice *p_rd, int64_t p_compute_list) const {
	this->debug_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY_DEBUG]);
}

void GdstrokeEffect::_compile_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name) {
	this->_compiled_shaders[p_shader] = create_comp_shader_from_embedded_spirv(p_rd, *(EmbeddedData const*)shader_to_embedded_data[p_shader], p_name);
}

#define COMPILE_SHADER(p_rd, p_shader) \
	_compile_shader(p_rd, p_shader, _STR(p_shader))

void GdstrokeEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_config_depth_bias", "p_value"), &GdstrokeEffect::set_config_depth_bias);
	ClassDB::bind_method(D_METHOD("get_config_depth_bias"),            &GdstrokeEffect::get_config_depth_bias);
	ClassDB::bind_method(D_METHOD("set_config_use_soft_depth_test_modification", "p_value"), &GdstrokeEffect::set_config_use_soft_depth_test_modification);
	ClassDB::bind_method(D_METHOD("get_config_use_soft_depth_test_modification"),            &GdstrokeEffect::get_config_use_soft_depth_test_modification);

	ADD_PROPERTY(
		PropertyInfo(
			Variant::FLOAT, "depth_bias"
		),
		"set_config_depth_bias",
		"get_config_depth_bias"
	);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::BOOL, "use_soft_depth_test_modification"
		),
		"set_config_use_soft_depth_test_modification",
		"get_config_use_soft_depth_test_modification"
	);
}

GdstrokeEffect::GdstrokeEffect() {
	this->set_effect_callback_type(EffectCallbackType::EFFECT_CALLBACK_TYPE_POST_OPAQUE);
	this->_ready = false;
	this->scene_interface_set = {};
	this->command_interface_set = {};
	this->mesh_interface_set = {};
	this->contour_interface_set = {};
}

void GdstrokeEffect::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();

	if (!this->_ready) {
		if (GdstrokeServer::get_singleton()->get_contour_instance() == nullptr)
			return;
		_ready = true;


		COMPILE_SHADER(rd, Shader::SHADER_DUMMY);
		COMPILE_SHADER(rd, Shader::SHADER_DUMMY_COMMANDER);
		COMPILE_SHADER(rd, Shader::SHADER_DUMMY_DEBUG);

		COMPILE_SHADER(rd, Shader::SHADER_CR_CED_FACE_ORIENTATION);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CED_DETECTION);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CED_ALLOCATION);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CED_SCATTER);

		COMPILE_SHADER(rd, Shader::SHADER_CR_FG_FIRST_COMMANDER);
		COMPILE_SHADER(rd, Shader::SHADER_CR_FG_FRAG_COUNTS);
		COMPILE_SHADER(rd, Shader::SHADER_CR_FG_ALLOCATION);
		COMPILE_SHADER(rd, Shader::SHADER_CR_FG_SCATTER);

		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_FIRST_COMMANDER);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_SOFT_DEPTH_TEST);

		COMPILE_SHADER(rd, Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS);


		for (int i = 0; i < Shader::SHADER_MAX; ++i) {
			this->_pipelines[Shader(i)] = rd->compute_pipeline_create(this->_compiled_shaders[Shader(i)]);
		}

		this->scene_interface_set.create_resources(rd, p_render_data);
		this->command_interface_set.create_resources(rd, p_render_data);
		this->mesh_interface_set.create_resources(rd, p_render_data);
		this->contour_interface_set.create_resources(rd, p_render_data);
		this->debug_interface_set.create_resources(rd, p_render_data);
	}

	this->scene_interface_set.update_resources(rd, p_render_data);
	this->scene_interface_set.make_bindings();
	ERR_FAIL_COND(!this->scene_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->command_interface_set.update_resources(rd, p_render_data);
	this->command_interface_set.make_bindings();
	ERR_FAIL_COND(!this->command_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY_COMMANDER]).is_valid());

	this->mesh_interface_set.update_resources(rd, p_render_data);
	this->mesh_interface_set.make_bindings();
	ERR_FAIL_COND(!this->mesh_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->contour_interface_set.update_resources(rd, p_render_data);
	this->contour_interface_set.make_bindings();
	ERR_FAIL_COND(!this->contour_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->debug_interface_set.update_resources(rd, p_render_data);
	this->debug_interface_set.make_bindings();
	ERR_FAIL_COND(!this->debug_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY_DEBUG]).is_valid());


	uint32_t num_verts = GdstrokeServer::get_singleton()->get_contour_mesh().vertex_buffer.size();
	uint32_t num_edges = GdstrokeServer::get_singleton()->get_contour_mesh().edge_to_vertex_buffer.size();
	uint32_t num_faces = GdstrokeServer::get_singleton()->get_contour_mesh().face_to_vertex_buffer.size();

	int64_t list;
	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DUMMY]);
	this->bind_sets(rd, list);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DUMMY]);
	this->bind_sets(rd, list);
	this->bind_sets_commander(rd, list);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_FACE_ORIENTATION]);
	this->bind_sets(rd, list);
	rd->compute_list_dispatch(list, udiv_ceil(num_faces, 64), 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_DETECTION]);
	this->bind_sets(rd, list);
	rd->compute_list_dispatch(list, udiv_ceil(num_edges, 64), 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_ALLOCATION]);
	this->bind_sets(rd, list);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CED_SCATTER]);
	this->bind_sets(rd, list);
	rd->compute_list_dispatch(list, udiv_ceil(num_edges, 64), 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_FIRST_COMMANDER]);
	this->bind_sets(rd, list);
	this->bind_sets_commander(rd, list);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_FRAG_COUNTS]);
	this->bind_sets(rd, list);
	this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_ALLOCATION]);
	this->bind_sets(rd, list);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_SCATTER]);
	this->bind_sets(rd, list);
	this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_CONTOUR_EDGES);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_FIRST_COMMANDER]);
	this->bind_sets(rd, list);
	this->bind_sets_commander(rd, list);
	rd->compute_list_dispatch(list, 1, 1, 1);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_SOFT_DEPTH_TEST]);
	this->bind_sets(rd, list);
	this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
	rd->compute_list_end();

	list = rd->compute_list_begin();
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS]);
	this->bind_sets(rd, list);
	this->bind_sets_debug(rd, list);
	this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
	rd->compute_list_end();
}

float GdstrokeEffect::get_config_depth_bias() const {
	return scene_interface_set.config_data.depth_bias;
}

void  GdstrokeEffect::set_config_depth_bias(float p_value) {
	scene_interface_set.config_data.depth_bias = p_value;
}

bool GdstrokeEffect::get_config_use_soft_depth_test_modification() const {
	return scene_interface_set.config_data.use_soft_depth_test_modification;
}

void GdstrokeEffect::set_config_use_soft_depth_test_modification(bool p_value) {
	scene_interface_set.config_data.use_soft_depth_test_modification = uint32_t(p_value);
}
