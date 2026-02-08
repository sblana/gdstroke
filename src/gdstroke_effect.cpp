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

void GdstrokeEffect::_bind_sets(RenderingDevice *p_rd, int64_t p_compute_list) const {
	_common_interface_set.bind_to_compute_list(p_rd, p_compute_list, _compiled_shaders[Shader::SHADER_DUMMY]);
}

void GdstrokeEffect::_bind_sets_commander(RenderingDevice *p_rd, int64_t p_compute_list) const {
	_command_interface_set.bind_to_compute_list(p_rd, p_compute_list, _compiled_shaders[Shader::SHADER_DUMMY_COMMANDER]);
}

void GdstrokeEffect::_bind_sets_debug(RenderingDevice *p_rd, int64_t p_compute_list) const {
	_debug_interface_set.bind_to_compute_list(p_rd, p_compute_list, _compiled_shaders[Shader::SHADER_DUMMY_DEBUG]);
}

void GdstrokeEffect::_compile_shader(RenderingDevice *p_rd, Shader p_shader, String const &p_name) {
	if (shader_to_shader_info_map.at(p_shader).shader_stages & M_SHADER_STAGE_COMPUTE_BIT) {
		_compiled_shaders[p_shader] = create_comp_shader_from_embedded_spirv(p_rd, *shader_to_embedded_data_map[p_shader][M_SHADER_STAGE_COMPUTE], p_name);
	}
	else {
		_compiled_shaders[p_shader] = create_draw_shader_from_embedded_spirv(p_rd, *shader_to_embedded_data_map[p_shader][M_SHADER_STAGE_VERTEX], *shader_to_embedded_data_map[p_shader][M_SHADER_STAGE_FRAGMENT], p_name);
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
	ClassDB::bind_method(D_METHOD("set_config_laplacian_iterations", "p_value"), &GdstrokeEffect::set_config_laplacian_iterations);
	ClassDB::bind_method(D_METHOD("get_config_laplacian_iterations"),            &GdstrokeEffect::get_config_laplacian_iterations);
	ClassDB::bind_method(D_METHOD("set_config_laplacian_factor", "p_value"), &GdstrokeEffect::set_config_laplacian_factor);
	ClassDB::bind_method(D_METHOD("get_config_laplacian_factor"),            &GdstrokeEffect::get_config_laplacian_factor);
	ClassDB::bind_method(D_METHOD("set_config_orientation_threshold", "p_value"), &GdstrokeEffect::set_config_orientation_threshold);
	ClassDB::bind_method(D_METHOD("get_config_orientation_threshold"),            &GdstrokeEffect::get_config_orientation_threshold);
	ClassDB::bind_method(D_METHOD("set_config_min_segment_length", "p_value"), &GdstrokeEffect::set_config_min_segment_length);
	ClassDB::bind_method(D_METHOD("get_config_min_segment_length"),            &GdstrokeEffect::get_config_min_segment_length);
	ClassDB::bind_method(D_METHOD("set_debug_view", "p_value"), &GdstrokeEffect::set_debug_view);
	ClassDB::bind_method(D_METHOD("get_debug_view"),            &GdstrokeEffect::get_debug_view);
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
			Variant::INT, "laplacian_iterations",
			PropertyHint::PROPERTY_HINT_RANGE, "1,20"
		),
		"set_config_laplacian_iterations",
		"get_config_laplacian_iterations"
	);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::FLOAT, "laplacian_factor",
			PropertyHint::PROPERTY_HINT_RANGE, "-1.0,1.0"
		),
		"set_config_laplacian_factor",
		"get_config_laplacian_factor"
	);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::FLOAT, "orientation_threshold",
			PropertyHint::PROPERTY_HINT_RANGE, "-1.0,1.0"
		),
		"set_config_orientation_threshold",
		"get_config_orientation_threshold"
	);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::INT, "min_segment_length",
			PropertyHint::PROPERTY_HINT_RANGE, "0,8192"
		),
		"set_config_min_segment_length",
		"get_config_min_segment_length"
	);
	ADD_PROPERTY(
		PropertyInfo(
			Variant::INT, "debug_view",
			PropertyHint::PROPERTY_HINT_ENUM, "Disabled,Contour Pixel Orientation,Contour Pixel Is Head"
		),
		"set_debug_view",
		"get_debug_view"
	);
}

GdstrokeEffect::GdstrokeEffect() {
	set_effect_callback_type(EffectCallbackType::EFFECT_CALLBACK_TYPE_POST_OPAQUE);
}

void GdstrokeEffect::_render_callback(int32_t p_effect_callback_type, RenderData *p_render_data) {
	RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();

	if (GdstrokeServer::get_num_contour_instances() == 0)
		return;

	if (!_ready) {
		_command_interface_set.create_resources(rd, p_render_data);
		_common_interface_set.create_resources(rd, p_render_data);
		_shader_api_interface_set.create_resources(rd, p_render_data);
		_debug_interface_set.create_resources(rd, p_render_data);

		for (int i = 0; i < Shader::SHADER_MAX; ++i) {
			COMPILE_SHADER(rd, Shader(i));
		}
		for (int i = 0; i < Shader::SHADER_MAX; ++i) {
			if (shader_to_shader_info_map.at(Shader(i)).shader_stages & M_SHADER_STAGE_COMPUTE_BIT)
				_pipelines[Shader(i)] = rd->compute_pipeline_create(_compiled_shaders[Shader(i)]);
		}

		_pipelines[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST] = _hard_depth_test_resources.create_render_pipeline(rd, _compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		_pipelines[Shader::SHADER_SR_DEFAULT_SHADER] = _stroke_rendering_resources.create_render_pipeline(rd, p_render_data, _compiled_shaders[Shader::SHADER_SR_DEFAULT_SHADER]);

		GdstrokeServer::register_gdstroke_effect(_id, this);
		_ready = true;
	}

	_hard_depth_test_resources.clear_color_attachments(rd, p_render_data);

	_command_interface_set.update_resources(rd, p_render_data);
	_command_interface_set.make_bindings();
	ERR_FAIL_COND(!_command_interface_set.get_uniform_set_rid(_compiled_shaders[Shader::SHADER_DUMMY_COMMANDER]).is_valid());

	_common_interface_set.receive_hard_depth_test_attachments(_hard_depth_test_resources.get_attachments(rd, p_render_data));
	_common_interface_set.update_resources(rd, p_render_data);
	_common_interface_set.make_bindings();
	ERR_FAIL_COND(!_common_interface_set.get_uniform_set_rid(_compiled_shaders[Shader::SHADER_DUMMY]).is_valid());

	_shader_api_interface_set.update_resources(rd, p_render_data);
	_shader_api_interface_set.make_bindings();
	ERR_FAIL_COND(!_shader_api_interface_set.get_uniform_set_rid(_compiled_shaders[Shader::SHADER_SR_UPDATE_API]).is_valid());

	_debug_interface_set.update_resources(rd, p_render_data);
	_debug_interface_set.make_bindings();
	ERR_FAIL_COND(!_debug_interface_set.get_uniform_set_rid(_compiled_shaders[Shader::SHADER_DUMMY_DEBUG]).is_valid());


	int64_t list;
	rd->draw_command_begin_label("dummy", Color(0.3, 0.3, 0.3));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_DUMMY]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_DUMMY]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Mesh Processing", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_MP_FIRST_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_MP_GLOBAL_GEOMETRY_ALLOCATION]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_MP_SECOND_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_MP_GLOBAL_GEOMETRY_SCATTER_EDGES]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_MP_GLOBAL_GEOMETRY_SCATTER_FACES]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_FACES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Contour Edge Detection", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_FACE_ORIENTATION]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_FACES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_DETECTION]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_ALLOCATION_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_ALLOCATION_L0_UP]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_ALLOCATION_L1_UP]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_ALLOCATION_L0_DOWN]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_MEM_ACQUIRE]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CED_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Fragment Generation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_FIRST_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_CLIPPING]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_FRAG_COUNTS]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_ALLOCATION]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_MEM_ACQUIRE]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_SECOND_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_CONTOUR_EDGES);
		rd->compute_list_end();

		if (_raster_method == RasterMethod::RASTER_METHOD_BRESENHAM) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_RASTERIZE_BRESENHAM]);
			_bind_sets(rd, list);
			_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
			rd->compute_list_end();
		}
		else if (_raster_method == RasterMethod::RASTER_METHOD_DDA) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_FG_RASTERIZE_DDA]);
			_bind_sets(rd, list);
			_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
			rd->compute_list_end();
		}
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Contour Pixel Generation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_SOFT_DEPTH_TEST]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_PRE_ALLOC]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_ALLOCATION_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_ALLOCATION_L0_UP]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_ALLOCATION_L1_UP]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_ALLOCATION_L0_DOWN]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_MEM_ACQUIRE]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_SECOND_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->draw_list_begin(_hard_depth_test_resources.get_framebuffer(rd, p_render_data), RenderingDevice::DrawFlags::DRAW_CLEAR_ALL, {Color(0, 0, 0, 0)}, 0.0);
		rd->draw_list_bind_render_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		_common_interface_set.bind_to_draw_list(rd, list, _compiled_shaders[Shader::SHADER_CR_CPG_HARD_DEPTH_TEST]);
		_command_interface_set.draw_indirect(rd, list, DrawIndirectCommands::DRAW_INDIRECT_COMMANDS_HARD_DEPTH_TEST);
		rd->draw_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CR_CPG_DECODE]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_PIXELS);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Pixel Edge Generation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_PEG_FIRST_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_PEG_GENERATION]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_PEG_FRAGMENTED_ALLOC]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_PEG_SECOND_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_PEG_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Loop Breaking", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_LB_INIT]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
		rd->compute_list_end();

		int const num_steps = 20;
		for (int step = 0; step < num_steps; ++step) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_LB_WYLLIE]);
			rd->compute_list_set_push_constant(list, PackedInt32Array({ (step + 0) % 2, (step + 1) % 2, 0, 0 }).to_byte_array(), 16u);
			_bind_sets(rd, list);
			_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
			rd->compute_list_end();
		}

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_LB_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("List Ranking", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_LR_INIT]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
		rd->compute_list_end();

		int const num_steps = 20;
		for (int step = 0; step < num_steps; ++step) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_LR_WYLLIE]);
			rd->compute_list_set_push_constant(list, PackedInt32Array({ (step + 0) % 2, (step + 1) % 2, 0, 0 }).to_byte_array(), 16u);
			_bind_sets(rd, list);
			_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
			rd->compute_list_end();
		}

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_LR_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Defragmentation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_HEAD_ALLOCATION_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_HEAD_ALLOCATION_L0_UP]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_HEAD_ALLOCATION_L1_UP]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_HEAD_ALLOCATION_L0_DOWN]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_FIRST_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_HEAD_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_COMPACTED_ALLOCATION]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_CC_D_COMPACTED_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Midpoints Filtering", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_MF_SMOOTHING]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Orientations Filtering", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_OF_CURVE_FITTING]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Inside Outside Test", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_IOT_TEST]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_IOT_SMOOTHING]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Segmentation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_SEGMENT_HEAD_ID]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_LOOP_LOCAL_SEGMENTATION]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_PIXEL_EDGE_LOOPS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_ALLOCATION]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_FIRST_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_CLEAR]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_SCATTER_TOP]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_S_SCATTER_BOTTOM]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Stroke Generation", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_SG_INIT]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENTS);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_SG_ALLOCATION]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_SG_FIRST_COMMANDER]);
		_bind_sets(rd, list);
		_bind_sets_commander(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SE_SG_SCATTER]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("Stroke Rendering", Color(1.0, 0.3, 1.0));
	{
		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SR_A_SEGMENT_EDGE_ARC_LENGTH_INIT]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SR_A_SEGMENT_EDGE_ARC_LENGTH]);
		_bind_sets(rd, list);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SR_A_SEGMENT_ARC_LENGTH]);
		_bind_sets(rd, list);
		_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES);
		rd->compute_list_end();

		list = rd->compute_list_begin();
		rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_SR_UPDATE_API]);
		_bind_sets(rd, list);
		_shader_api_interface_set.bind_to_compute_list(rd, list, _compiled_shaders[Shader::SHADER_SR_UPDATE_API]);
		rd->compute_list_dispatch(list, 1, 1, 1);
		rd->compute_list_end();
	}
	rd->draw_command_end_label();

	rd->draw_command_begin_label("debug", Color(0.2, 0.2, 0.2));
	{
		if (_debug_view == DebugView::DEBUG_VIEW_CONTOUR_PIXEL_ORIENTATION || _debug_view == DebugView::DEBUG_VIEW_CONTOUR_PIXEL_IS_HEAD) {
			list = rd->compute_list_begin();
			rd->compute_list_bind_compute_pipeline(list, _pipelines[Shader::SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS]);
			rd->compute_list_set_push_constant(list, PackedInt32Array({int32_t(_debug_view), 0, 0, 0}).to_byte_array(), 16);
			_bind_sets(rd, list);
			_bind_sets_debug(rd, list);
			_command_interface_set.dispatch_indirect(rd, list, DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_PIXELS);
			rd->compute_list_end();
		}
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
	return _common_interface_set.config_data.depth_bias;
}

void  GdstrokeEffect::set_config_depth_bias(float p_value) {
	_common_interface_set.config_data.depth_bias = p_value;
}

bool GdstrokeEffect::get_config_use_soft_depth_test_modification() const {
	return _common_interface_set.config_data.use_soft_depth_test_modification;
}

void GdstrokeEffect::set_config_use_soft_depth_test_modification(bool p_value) {
	_common_interface_set.config_data.use_soft_depth_test_modification = uint32_t(p_value);
}

float GdstrokeEffect::get_config_laplacian_factor() const {
	return _common_interface_set.config_data.laplacian_factor;
}

void  GdstrokeEffect::set_config_laplacian_factor(float p_value) {
	_common_interface_set.config_data.laplacian_factor = p_value;
}

uint32_t GdstrokeEffect::get_config_laplacian_iterations() const {
	return _common_interface_set.config_data.laplacian_iterations;
}

void GdstrokeEffect::set_config_laplacian_iterations(uint32_t p_value) {
	_common_interface_set.config_data.laplacian_iterations = uint32_t(p_value);
}

float GdstrokeEffect::get_config_orientation_threshold() const {
	return _common_interface_set.config_data.orientation_threshold;
}

void  GdstrokeEffect::set_config_orientation_threshold(float p_value) {
	_common_interface_set.config_data.orientation_threshold = p_value;
}

uint32_t GdstrokeEffect::get_config_min_segment_length() const {
	return _common_interface_set.config_data.min_segment_length;
}

void GdstrokeEffect::set_config_min_segment_length(uint32_t p_value) {
	_common_interface_set.config_data.min_segment_length = uint32_t(p_value);
}

GdstrokeEffect::DebugView GdstrokeEffect::get_debug_view() const {
	return _debug_view;
}

void                      GdstrokeEffect::set_debug_view(DebugView p_value) {
	_debug_view = p_value;
}

RID GdstrokeEffect::get_stroke_shader_uniform_set_rid() {
	return _shader_api_interface_set.get_uniform_set_rid(_compiled_shaders[Shader::SHADER_SR_DEFAULT_SHADER]);
}

int64_t GdstrokeEffect::get_stroke_shader_uniform_set_slot() const {
	return _shader_api_interface_set.get_slot();
}

void GdstrokeEffect::draw_indirect_stroke_shader(RenderingDevice *p_rd, int64_t p_draw_list) {
	_command_interface_set.draw_indirect(p_rd, p_draw_list, DrawIndirectCommands::DRAW_INDIRECT_COMMANDS_STROKE_RENDERING);
}
