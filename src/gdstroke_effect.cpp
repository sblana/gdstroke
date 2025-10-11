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
#include "gen/debug__display_contour_fragments.spv.h"


using namespace godot;

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

void GdstrokeEffect::_bind_methods() {}

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


		this->_compiled_shaders[Shader::SHADER_DUMMY          ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_dummy,           _STR(SHADER_DUMMY          ));
		this->_compiled_shaders[Shader::SHADER_DUMMY_COMMANDER] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_dummy_commander, _STR(SHADER_DUMMY_COMMANDER));
		this->_compiled_shaders[Shader::SHADER_DUMMY_DEBUG    ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_dummy_debug,     _STR(SHADER_DUMMY_DEBUG    ));

		this->_compiled_shaders[Shader::SHADER_CR_CED_FACE_ORIENTATION] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__face_orientation, _STR(SHADER_CR_CED_FACE_ORIENTATION));
		this->_compiled_shaders[Shader::SHADER_CR_CED_DETECTION       ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__detection,        _STR(SHADER_CR_CED_DETECTION       ));
		this->_compiled_shaders[Shader::SHADER_CR_CED_ALLOCATION      ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__allocation,       _STR(SHADER_CR_CED_ALLOCATION      ));
		this->_compiled_shaders[Shader::SHADER_CR_CED_SCATTER         ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__ced__scatter,          _STR(SHADER_CR_CED_SCATTER         ));

		this->_compiled_shaders[Shader::SHADER_CR_FG_FIRST_COMMANDER ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__fg__first_commander, _STR(SHADER_CR_FG_FIRST_COMMANDER ));
		this->_compiled_shaders[Shader::SHADER_CR_FG_FRAG_COUNTS     ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__fg__frag_counts,     _STR(SHADER_CR_FG_FRAG_COUNTS     ));
		this->_compiled_shaders[Shader::SHADER_CR_FG_ALLOCATION      ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__fg__allocation,      _STR(SHADER_CR_FG_ALLOCATION      ));
		this->_compiled_shaders[Shader::SHADER_CR_FG_SCATTER         ] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__fg__scatter,         _STR(SHADER_CR_FG_SCATTER         ));

		this->_compiled_shaders[Shader::SHADER_CR_CPG_FIRST_COMMANDER] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_cr__cpg__first_commander, _STR(SHADER_CR_CPG_FIRST_COMMANDER));

		this->_compiled_shaders[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS] = create_comp_shader_from_embedded_spirv(rd, SHADER_SPV_debug__display_contour_fragments, _STR(SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS));


		this->_pipelines[Shader::SHADER_DUMMY          ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_DUMMY          ]);
		this->_pipelines[Shader::SHADER_DUMMY_COMMANDER] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_DUMMY_COMMANDER]);
		this->_pipelines[Shader::SHADER_DUMMY_DEBUG    ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_DUMMY_DEBUG    ]);

		this->_pipelines[Shader::SHADER_CR_CED_FACE_ORIENTATION] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_FACE_ORIENTATION]);
		this->_pipelines[Shader::SHADER_CR_CED_DETECTION       ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_DETECTION       ]);
		this->_pipelines[Shader::SHADER_CR_CED_ALLOCATION      ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_ALLOCATION      ]);
		this->_pipelines[Shader::SHADER_CR_CED_SCATTER         ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CED_SCATTER         ]);

		this->_pipelines[Shader::SHADER_CR_FG_FIRST_COMMANDER ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_FG_FIRST_COMMANDER ]);
		this->_pipelines[Shader::SHADER_CR_FG_FRAG_COUNTS     ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_FG_FRAG_COUNTS     ]);
		this->_pipelines[Shader::SHADER_CR_FG_ALLOCATION      ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_FG_ALLOCATION      ]);
		this->_pipelines[Shader::SHADER_CR_FG_SCATTER         ] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_FG_SCATTER         ]);

		this->_pipelines[Shader::SHADER_CR_CPG_FIRST_COMMANDER] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_CR_CPG_FIRST_COMMANDER]);

		this->_pipelines[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS] = rd->compute_pipeline_create(this->_compiled_shaders[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS]);


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
	rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS]);
	this->bind_sets(rd, list);
	this->bind_sets_debug(rd, list);
	this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
	rd->compute_list_end();
}
