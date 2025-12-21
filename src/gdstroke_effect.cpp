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
	if (shader_to_shader_info_map.at(p_shader).shader_stages & M_SHADER_STAGE_COMPUTE_BIT) {
		this->_compiled_shaders[p_shader] = create_comp_shader_from_embedded_spirv(p_rd, *shader_to_embedded_data_map[p_shader][M_SHADER_STAGE_COMPUTE], p_name);
	}
	else {
		this->_compiled_shaders[p_shader] = create_draw_shader_from_embedded_spirv(p_rd, *shader_to_embedded_data_map[p_shader][M_SHADER_STAGE_VERTEX], *shader_to_embedded_data_map[p_shader][M_SHADER_STAGE_FRAGMENT], p_name);
	}
}

#define COMPILE_SHADER(p_rd, p_shader) \
	_compile_shader(p_rd, p_shader, shader_to_shader_info_map.at(p_shader).name)

void GdstrokeEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_id", "p_value"), &GdstrokeEffect::set_id);
	ClassDB::bind_method(D_METHOD("get_id"),            &GdstrokeEffect::get_id);
	ClassDB::bind_method(D_METHOD("set_raster_method", "p_value"), &GdstrokeEffect::set_raster_method);
	ClassDB::bind_method(D_METHOD("get_raster_method"),            &GdstrokeEffect::get_raster_method);
	ClassDB::bind_method(D_METHOD("set_config_depth_bias", "p_value"), &GdstrokeEffect::set_config_depth_bias);
	ClassDB::bind_method(D_METHOD("get_config_depth_bias"),            &GdstrokeEffect::get_config_depth_bias);
	ClassDB::bind_method(D_METHOD("set_config_use_soft_depth_test_modification", "p_value"), &GdstrokeEffect::set_config_use_soft_depth_test_modification);
	ClassDB::bind_method(D_METHOD("get_config_use_soft_depth_test_modification"),            &GdstrokeEffect::get_config_use_soft_depth_test_modification);
	ClassDB::bind_method(D_METHOD("set_config_min_segment_length", "p_value"), &GdstrokeEffect::set_config_min_segment_length);
	ClassDB::bind_method(D_METHOD("get_config_min_segment_length"),            &GdstrokeEffect::get_config_min_segment_length);
	ClassDB::bind_method(D_METHOD("get_stroke_shader_uniform_set_rid"),  &GdstrokeEffect::get_stroke_shader_uniform_set_rid);
	ClassDB::bind_method(D_METHOD("get_stroke_shader_uniform_set_slot"), &GdstrokeEffect::get_stroke_shader_uniform_set_slot);
	ClassDB::bind_method(D_METHOD("draw_indirect_stroke_shader", "p_rd", "p_draw_list"), &GdstrokeEffect::draw_indirect_stroke_shader);

	BIND_ENUM_CONSTANT(RASTER_METHOD_BRESENHAM);
	BIND_ENUM_CONSTANT(RASTER_METHOD_DDA);
	BIND_ENUM_CONSTANT(RASTER_METHOD_MAX);

	ADD_PROPERTY(
		PropertyInfo(
			Variant::INT, "id"
		),
		"set_id",
		"get_id"
	);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::INT, "raster_method",
			PropertyHint::PROPERTY_HINT_ENUM, "Bresenham,DDA"
		),
		"set_raster_method",
		"get_raster_method"
	);
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
	ADD_PROPERTY(
		PropertyInfo(
			Variant::INT, "min_segment_length",
			PROPERTY_HINT_RANGE, "0,8192"
		),
		"set_config_min_segment_length",
		"get_config_min_segment_length"
	);
}

GdstrokeEffect::GdstrokeEffect() {
	this->set_effect_callback_type(EffectCallbackType::EFFECT_CALLBACK_TYPE_POST_OPAQUE);
	this->_id = 0;
	this->_raster_method = RasterMethod::RASTER_METHOD_BRESENHAM;
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
		this->shader_api_interface_set.create_resources(rd, p_render_data);
		this->debug_interface_set.create_resources(rd, p_render_data);

		for (int i = 0; i < Shader::SHADER_MAX; ++i) {
			COMPILE_SHADER(rd, Shader(i));
		}
		for (int i = 0; i < Shader::SHADER_MAX; ++i) {
			if (shader_to_shader_info_map.at(Shader(i)).shader_stages & M_SHADER_STAGE_COMPUTE_BIT)
				this->_pipelines[Shader(i)] = rd->compute_pipeline_create(this->_compiled_shaders[Shader(i)]);
		}

		_pipelines[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST] = hard_depth_test_resources.create_render_pipeline(rd, _compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		_pipelines[Shader::SHADER_SR_DEFAULT_SHADER] = stroke_rendering_resources.create_render_pipeline(rd, p_render_data, _compiled_shaders[Shader::SHADER_SR_DEFAULT_SHADER]);

		GdstrokeServer::get_singleton()->register_gdstroke_effect(_id, this);
		_ready = true;
	}

	this->hard_depth_test_resources.clear_color_attachments(rd, p_render_data);

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

	this->shader_api_interface_set.update_resources(rd, p_render_data);
	this->shader_api_interface_set.make_bindings();
	ERR_FAIL_COND(!this->shader_api_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_SR_UPDATE_API]).is_valid());

	this->debug_interface_set.update_resources(rd, p_render_data);
	this->debug_interface_set.make_bindings();
	ERR_FAIL_COND(!this->debug_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_DUMMY_DEBUG]).is_valid());


	uint32_t num_verts = GdstrokeServer::get_singleton()->get_contour_mesh().vertex_buffer.size();
	uint32_t num_edges = GdstrokeServer::get_singleton()->get_contour_mesh().edge_to_vertex_buffer.size();
	uint32_t num_faces = GdstrokeServer::get_singleton()->get_contour_mesh().face_to_vertex_buffer.size();

	// HACK: rethink abstractions to avoid this pls
	using      MeshBuffers  = GdstrokeShaderInterface::     MeshInterfaceSet::Buffer;
	using      MeshBindings = GdstrokeShaderInterface::     MeshInterfaceSet::Binding;
	using   ContourBuffers  = GdstrokeShaderInterface::  ContourInterfaceSet::Buffer;
	using   ContourBindings = GdstrokeShaderInterface::  ContourInterfaceSet::Binding;
	using PixelEdgeBuffers  = GdstrokeShaderInterface::PixelEdgeInterfaceSet::Buffer;
	using PixelEdgeBindings = GdstrokeShaderInterface::PixelEdgeInterfaceSet::Binding;
	uint64_t       mesh_buffers_ptr = rd->buffer_get_device_address(this->      mesh_interface_set.resources[int(     MeshBuffers::BUFFER_MAX) + int(     MeshBindings::BINDING_MESH_BUFFERS)]);
	uint64_t    contour_buffers_ptr = rd->buffer_get_device_address(this->   contour_interface_set.resources[int(  ContourBuffers::BUFFER_MAX) + int(  ContourBindings::BINDING_CONTOUR_BUFFERS)]);
	uint64_t pixel_edge_buffers_ptr = rd->buffer_get_device_address(this->pixel_edge_interface_set.resources[int(PixelEdgeBuffers::BUFFER_MAX) + int(PixelEdgeBindings::BINDING_PIXEL_EDGE_BUFFERS)]);

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

		PackedByteArray push = PackedInt64Array({
			// B_edge_is_contour.contour[idx],
			int64_t(sizeof(uint64_t) * MeshBuffers::BUFFER_EDGE_IS_CONTOUR_BUFFER + mesh_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			int64_t(1 * sizeof(int32_t)),
			// B_mesh_desc.num_edges,
			int64_t(sizeof(uint64_t) * MeshBuffers::BUFFER_MESH_DESC_BUFFER + mesh_buffers_ptr),
			int64_t(1 * sizeof(int32_t)),
			// B_contour_desc.num_contour_edges,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_DESC_BUFFER + contour_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			// B_edge_to_contour_edge.idx[idx],
			int64_t(sizeof(uint64_t) * MeshBuffers::BUFFER_EDGE_TO_CONTOUR_EDGE_BUFFER + mesh_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			int64_t(1 * sizeof(int32_t)),
		}).to_byte_array();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_COMMANDER]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L0_UP]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L1_UP]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L0_DOWN]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
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
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_CLIPPING]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_FRAG_COUNTS]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_WG_ALLOCATION]);
		rd->compute_list_set_push_constant(list, PackedInt64Array({
			// B_contour_edge_to_contour_fragment.data[idx].num_fragments,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_EDGE_TO_CONTOUR_FRAGMENT_BUFFER + contour_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			int64_t(2 * sizeof(int32_t)),
			// B_contour_desc.num_contour_edges,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_DESC_BUFFER + contour_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			// B_contour_desc.num_contour_fragments,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_DESC_BUFFER + contour_buffers_ptr),
			int64_t(2 * sizeof(int32_t)),
			// B_contour_edge_to_contour_fragment.data[idx].first_fragment_idx,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_EDGE_TO_CONTOUR_FRAGMENT_BUFFER + contour_buffers_ptr),
			int64_t(1 * sizeof(int32_t)),
			int64_t(2 * sizeof(int32_t)),
		}).to_byte_array(), 80);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_SECOND_COMMANDER]);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_CONTOUR_EDGES);
		rd->compute_list_end();

		if (_raster_method == RasterMethod::RASTER_METHOD_BRESENHAM) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_RASTERIZE_BRESENHAM]);
			this->bind_sets(rd, list);
			this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
			rd->compute_list_end();
		}
		else if (_raster_method == RasterMethod::RASTER_METHOD_DDA) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CR_FG_RASTERIZE_DDA]);
			this->bind_sets(rd, list);
			this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
			rd->compute_list_end();
		}
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Contour Pixel Generation", Color(1.0, 0.3, 1.0));
	{
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

		PackedByteArray push = PackedInt64Array({
			// B_allocation_contour_pixel.data[idx].is_fragment_cluster_leader,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_ALLOCATION_CONTOUR_PIXEL_BUFFER + contour_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			int64_t(2 * sizeof(int32_t)),
			// B_contour_desc.num_contour_fragments,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_DESC_BUFFER + contour_buffers_ptr),
			int64_t(2 * sizeof(int32_t)),
			// B_contour_desc.num_contour_pixels,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_CONTOUR_DESC_BUFFER + contour_buffers_ptr),
			int64_t(4 * sizeof(int32_t)),
			// B_allocation_contour_pixel.data[idx].contour_pixel_idx,
			int64_t(sizeof(uint64_t) * ContourBuffers::BUFFER_ALLOCATION_CONTOUR_PIXEL_BUFFER + contour_buffers_ptr),
			int64_t(1 * sizeof(int32_t)),
			int64_t(2 * sizeof(int32_t)),
		}).to_byte_array();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_COMMANDER]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L0_UP]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L1_UP]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L0_DOWN]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
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

	rd->draw_command_begin_label("Defragmentation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_D_INIT]);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		PackedByteArray push = PackedInt64Array({
			// B_sparse_pixel_edge_is_head.is_head[idx],
			int64_t(sizeof(uint64_t) * PixelEdgeBuffers::BUFFER_SPARSE_PIXEL_EDGE_IS_HEAD_BUFFER + pixel_edge_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			int64_t(1 * sizeof(int32_t)),
			// B_pixel_edge_desc.buf_len_sparse_pixel_edge,
			int64_t(sizeof(uint64_t) * PixelEdgeBuffers::BUFFER_PIXEL_EDGE_DESC_BUFFER + pixel_edge_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			// B_pixel_edge_desc.num_pixel_edge_loops,
			int64_t(sizeof(uint64_t) * PixelEdgeBuffers::BUFFER_PIXEL_EDGE_DESC_BUFFER + pixel_edge_buffers_ptr),
			int64_t(2 * sizeof(int32_t)),
			// B_sparse_pixel_edge_to_pixel_edge_loop.idx[idx],
			int64_t(sizeof(uint64_t) * PixelEdgeBuffers::BUFFER_SPARSE_PIXEL_EDGE_TO_PIXEL_EDGE_LOOP_BUFFER + pixel_edge_buffers_ptr),
			int64_t(0 * sizeof(int32_t)),
			int64_t(1 * sizeof(int32_t)),
		}).to_byte_array();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_COMMANDER]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L0_UP]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L1_UP]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_REUSABLE_ALLOCATION_L0_DOWN]);
		rd->compute_list_set_push_constant(list, push, 80);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_D_HEAD_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_D_COMPACTED_ALLOCATION]);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_D_COMPACTED_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_CC_D_FIRST_COMMANDER]);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Midpoints Filtering", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_MF_INIT]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_MF_SMOOTHING]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Orientations Filtering", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_OF_CURVE_FITTING]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Inside Outside Test", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_IOT_TEST]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_IOT_SMOOTHING]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Segmentation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_SEGMENT_HEAD_ID]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_LOOP_LOCAL_SEGMENTATION]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_PIXEL_EDGE_LOOPS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_ALLOCATION]);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_FIRST_COMMANDER]);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_CLEAR]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_SCATTER_TOP]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_S_SCATTER_BOTTOM]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Stroke Generation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_SG_INIT]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_SG_ALLOCATION]);
		this->bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_SG_FIRST_COMMANDER]);
		this->bind_sets(rd, list);
		this->bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SE_SG_SCATTER]);
		this->bind_sets(rd, list);
		this->command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Stroke Rendering", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, this->_pipelines[Shader::SHADER_SR_UPDATE_API]);
		this->bind_sets(rd, list);
		this->shader_api_interface_set.bind_to_compute_list(rd, list, this->_compiled_shaders[Shader::SHADER_SR_UPDATE_API]);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("debug", Color(0.2, 0.2, 0.2));
	{
	}
	rd->draw_command_end_label();
}

int64_t GdstrokeEffect::get_id() const {
	return _id;
}

void    GdstrokeEffect::set_id(int64_t p_value) {
	ERR_FAIL_COND(_ready);
	_id = p_value;
}

GdstrokeEffect::RasterMethod GdstrokeEffect::get_raster_method() const {
	return _raster_method;
}

void                         GdstrokeEffect::set_raster_method(RasterMethod p_value) {
	_raster_method = p_value;
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

uint32_t GdstrokeEffect::get_config_min_segment_length() const {
	return scene_interface_set.config_data.min_segment_length;
}

void GdstrokeEffect::set_config_min_segment_length(uint32_t p_value) {
	scene_interface_set.config_data.min_segment_length = uint32_t(p_value);
}

RID GdstrokeEffect::get_stroke_shader_uniform_set_rid() {
	return this->shader_api_interface_set.get_uniform_set_rid(this->_compiled_shaders[Shader::SHADER_SR_DEFAULT_SHADER]);
}

int64_t GdstrokeEffect::get_stroke_shader_uniform_set_slot() const {
	return this->shader_api_interface_set.get_slot();
}

void GdstrokeEffect::draw_indirect_stroke_shader(RenderingDevice *p_rd, int64_t p_draw_list) {
	this->command_interface_set.draw_indirect(p_rd, p_draw_list, DrawIndirectCommands::DRAW_INDIRECT_COMMANDS_STROKE_RENDERING);
}
