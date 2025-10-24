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


using namespace godot;

#ifndef _USING_EDITOR
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
#include "gen/cr__cpg__pre_alloc.spv.h"
#include "gen/cr__cpg__allocation.spv.h"
#include "gen/cr__cpg__scatter.spv.h"
#include "gen/cr__cpg__second_commander.spv.h"
#include "gen/cr__cpg__hard_depth_test.vert.spv.h"
#include "gen/cr__cpg__hard_depth_test.frag.spv.h"
#include "gen/cr__cpg__decode.spv.h"
#include "gen/cc__peg__first_commander.spv.h"
#include "gen/cc__peg__generation.spv.h"
#include "gen/cc__lb__init.spv.h"
#include "gen/cc__lb__wyllie.spv.h"
#include "gen/cc__lb__scatter.spv.h"
#include "gen/cc__lr__init.spv.h"
#include "gen/cc__lr__wyllie.spv.h"
#include "gen/cc__lr__scatter.spv.h"
#include "gen/debug__display_contour_fragments.spv.h"
#include "gen/debug__display_contour_pixels.spv.h"
#include "gen/debug__display_sparse_pixel_edges.spv.h"


void const *hard_depth_test_embedded_data_stages[2] = {
	&SHADER_SPV_cr__cpg__hard_depth_test__vert,
	&SHADER_SPV_cr__cpg__hard_depth_test__frag,
};

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
	&SHADER_SPV_cr__cpg__pre_alloc,
	&SHADER_SPV_cr__cpg__allocation,
	&SHADER_SPV_cr__cpg__scatter,
	&SHADER_SPV_cr__cpg__second_commander,
	hard_depth_test_embedded_data_stages,
	&SHADER_SPV_cr__cpg__decode,
	&SHADER_SPV_cc__peg__first_commander,
	&SHADER_SPV_cc__peg__generation,
	&SHADER_SPV_cc__lb__init,
	&SHADER_SPV_cc__lb__wyllie,
	&SHADER_SPV_cc__lb__scatter,
	&SHADER_SPV_cc__lr__init,
	&SHADER_SPV_cc__lr__wyllie,
	&SHADER_SPV_cc__lr__scatter,
	&SHADER_SPV_debug__display_contour_fragments,
	&SHADER_SPV_debug__display_contour_pixels,
	&SHADER_SPV_debug__display_sparse_pixel_edges,
};

#endif // !_USING_EDITOR


void GdstrokeEffect::bind_sets(RenderingDevice *p_rd, int64_t p_compute_list) const {
	this->     scene_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->      mesh_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->   contour_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
	this->pixel_edge_interface_set.bind_to_compute_list(p_rd, p_compute_list, this->_compiled_shaders[Shader::SHADER_DUMMY]);
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

void GdstrokeEffect::_compile_draw_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name) {
	this->_compiled_shaders[p_shader] = create_draw_shader_from_embedded_spirv(p_rd, *(((EmbeddedData const**)shader_to_embedded_data[p_shader])[0]), *(((EmbeddedData const**)shader_to_embedded_data[p_shader])[1]), p_name);
}

#define COMPILE_SHADER(p_rd, p_shader) \
	_compile_shader(p_rd, p_shader, _STR(p_shader))

#define COMPILE_DRAW_SHADER(p_rd, p_shader) \
	_compile_draw_shader(p_rd, p_shader, _STR(p_shader))

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

		this->scene_interface_set.create_resources(rd, p_render_data);
		this->command_interface_set.create_resources(rd, p_render_data);
		this->mesh_interface_set.create_resources(rd, p_render_data);
		this->contour_interface_set.create_resources(rd, p_render_data);
		this->pixel_edge_interface_set.create_resources(rd, p_render_data);
		this->debug_interface_set.create_resources(rd, p_render_data);

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
		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_PRE_ALLOC);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_ALLOCATION);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_SCATTER);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_SECOND_COMMANDER);
		COMPILE_DRAW_SHADER(rd, Shader::SHADER_CR_CPG_HARD_DEPTH_TEST);
		COMPILE_SHADER(rd, Shader::SHADER_CR_CPG_DECODE);

		COMPILE_SHADER(rd, Shader::SHADER_CC_PEG_FIRST_COMMANDER);
		COMPILE_SHADER(rd, Shader::SHADER_CC_PEG_GENERATION);

		COMPILE_SHADER(rd, Shader::SHADER_CC_LB_INIT);
		COMPILE_SHADER(rd, Shader::SHADER_CC_LB_WYLLIE);
		COMPILE_SHADER(rd, Shader::SHADER_CC_LB_SCATTER);

		COMPILE_SHADER(rd, Shader::SHADER_CC_LR_INIT);
		COMPILE_SHADER(rd, Shader::SHADER_CC_LR_WYLLIE);
		COMPILE_SHADER(rd, Shader::SHADER_CC_LR_SCATTER);

		COMPILE_SHADER(rd, Shader::SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS);
		COMPILE_SHADER(rd, Shader::SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS);
		COMPILE_SHADER(rd, Shader::SHADER_DEBUG_DISPLAY_SPARSE_PIXEL_EDGES);


		for (int i = 0; i < Shader::SHADER_MAX; ++i) {
			if (i != Shader::SHADER_CR_CPG_HARD_DEPTH_TEST) {
				this->_pipelines[Shader(i)] = rd->compute_pipeline_create(this->_compiled_shaders[Shader(i)]);
			}
		}

		_pipelines[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST] = hard_depth_test_resources.create_render_pipeline(rd, _compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);

		_ready = true;
	}

	this->hard_depth_test_resources.clear_attachments(rd, p_render_data);

	this->scene_interface_set.update_resources(rd, p_render_data);
	this->scene_interface_set.make_bindings();
	ERR_FAIL_COND(!this->scene_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->command_interface_set.update_resources(rd, p_render_data);
	this->command_interface_set.make_bindings();
	ERR_FAIL_COND(!this->command_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY_COMMANDER]).is_valid());

	this->mesh_interface_set.update_resources(rd, p_render_data);
	this->mesh_interface_set.make_bindings();
	ERR_FAIL_COND(!this->mesh_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->contour_interface_set.receive_hard_depth_test_attachments(this->hard_depth_test_resources.get_attachments(rd, p_render_data));
	this->contour_interface_set.update_resources(rd, p_render_data);
	this->contour_interface_set.make_bindings();
	ERR_FAIL_COND(!this->contour_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->pixel_edge_interface_set.update_resources(rd, p_render_data);
	this->pixel_edge_interface_set.make_bindings();
	ERR_FAIL_COND(!this->pixel_edge_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	this->debug_interface_set.update_resources(rd, p_render_data);
	this->debug_interface_set.make_bindings();
	ERR_FAIL_COND(!this->debug_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY_DEBUG]).is_valid());


	uint32_t num_verts = GdstrokeServer::get_singleton()->get_contour_mesh().vertex_buffer.size();
	uint32_t num_edges = GdstrokeServer::get_singleton()->get_contour_mesh().edge_to_vertex_buffer.size();
	uint32_t num_faces = GdstrokeServer::get_singleton()->get_contour_mesh().face_to_vertex_buffer.size();

	int64_t list;
	rd->draw_command_begin_label("dummy", Color(0.3, 0.3, 0.3));
	{
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
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Contour Edge Detection", Color(1.0, 0.3, 1.0));
	{
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
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Fragment Generation", Color(1.0, 0.3, 1.0));
	{
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
	}
	rd->draw_command_end_label();


	rd->draw_command_begin_label("Contour Pixel Generation", Color(1.0, 0.3, 1.0));
	{
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
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_PRE_ALLOC]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_ALLOCATION]);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_SECOND_COMMANDER]);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->draw_list_begin(this->hard_depth_test_resources.get_framebuffer(rd, p_render_data), RenderingDevice::DrawFlags::DRAW_CLEAR_ALL, {Color(0, 0, 0, 0)}, 0.0);
		rd->draw_list_bind_render_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		this->scene_interface_set.bind_to_draw_list(rd, list, this->_compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		this->mesh_interface_set.bind_to_draw_list(rd, list, this->_compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		this->contour_interface_set.bind_to_draw_list(rd, list, this->_compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		this->pixel_edge_interface_set.bind_to_draw_list(rd, list, this->_compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		this->command_interface_set.draw_indirect(rd, list, DrawIndirectCommands::DRAW_INDIRECT_COMMANDS_HARD_DEPTH_TEST);
		rd->draw_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_CPG_DECODE]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_PIXELS);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Pixel Edge Generation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_PEG_FIRST_COMMANDER]);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_PEG_GENERATION]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Loop Breaking", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_LB_INIT]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();

		int const num_steps = 20;
		for (int step = 0; step < num_steps; ++step) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_LB_WYLLIE]);
			rd->compute_list_set_push_constant(list, PackedInt32Array({ (step + 0) % 2, (step + 1) % 2, 0, 0 }).to_byte_array(), 16u);
			this->bind_sets(rd, list);
			this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
			rd->compute_list_end();
		}

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_LB_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("List Ranking", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_LR_INIT]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();

		int const num_steps = 20;
		for (int step = 0; step < num_steps; ++step) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_LR_WYLLIE]);
			rd->compute_list_set_push_constant(list, PackedInt32Array({ (step + 0) % 2, (step + 1) % 2, 0, 0 }).to_byte_array(), 16u);
			this->bind_sets(rd, list);
			this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
			rd->compute_list_end();
		}

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_LR_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("debug", Color(0.2, 0.2, 0.2));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS]);
		this->bind_sets(rd, list);
		this->bind_sets_debug(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_PIXELS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_DEBUG_DISPLAY_SPARSE_PIXEL_EDGES]);
		this->bind_sets(rd, list);
		this->bind_sets_debug(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();
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
