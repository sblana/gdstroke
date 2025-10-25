#include "gdstroke_shader_interface.hpp"

#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/render_scene_data.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/framebuffer_cache_rd.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>
#include <godot_cpp/classes/rd_texture_format.hpp>
#include <godot_cpp/classes/rd_texture_view.hpp>
#include <godot_cpp/classes/rd_sampler_state.hpp>
#include <godot_cpp/classes/rd_pipeline_rasterization_state.hpp>
#include <godot_cpp/classes/rd_pipeline_multisample_state.hpp>
#include <godot_cpp/classes/rd_pipeline_depth_stencil_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state.hpp>
#include <godot_cpp/classes/rd_pipeline_color_blend_state_attachment.hpp>
#include <godot_cpp/classes/rd_vertex_attribute.hpp>
#include <godot_cpp/classes/rd_attachment_format.hpp>
#include <godot_cpp/classes/rd_framebuffer_pass.hpp>

#include "rd_util.hpp"
#include "vec_util.hpp"
#include "gdstroke_server.hpp"


using namespace godot;

void GdstrokeShaderInterface::_bind_methods() {}

RID GdstrokeShaderInterface::InterfaceSet::get_uniform_set_rid(RID const &p_shader) const {
	return UniformSetCacheRD::get_cache(p_shader, get_slot(), bindings);
}

RID GdstrokeShaderInterface::InterfaceSet::get_draw_uniform_set_rid(RID const &p_shader) const {
	return UniformSetCacheRD::get_cache(p_shader, get_slot(), get_draw_bindings());
}

void GdstrokeShaderInterface::InterfaceSet::bind_to_compute_list(RenderingDevice *p_rd, int64_t p_compute_list, RID const &p_shader) const {
	p_rd->compute_list_bind_uniform_set(p_compute_list, get_uniform_set_rid(p_shader), get_slot());
}

void GdstrokeShaderInterface::InterfaceSet::bind_to_draw_list(RenderingDevice *p_rd, int64_t p_draw_list, RID const &p_shader) const {
	p_rd->draw_list_bind_uniform_set(p_draw_list, get_draw_uniform_set_rid(p_shader), get_slot());
}


Error GdstrokeShaderInterface::SceneInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	ERR_FAIL_COND_V(!p_render_data->get_render_scene_data()->get_uniform_buffer().is_valid(), Error::FAILED);
	resources = {};
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_SCENE_DATA_UNIFORM] = p_render_data->get_render_scene_data()->get_uniform_buffer();
	resources[Binding::BINDING_CONFIG_UNIFORM] = p_rd->uniform_buffer_create(sizeof(float) * 4);
	return Error::OK;
}

Error GdstrokeShaderInterface::SceneInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(resources.size() == 0, Error::FAILED);
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	ERR_FAIL_COND_V(!p_render_data->get_render_scene_data()->get_uniform_buffer().is_valid(), Error::FAILED);
	resources[Binding::BINDING_SCENE_DATA_UNIFORM] = p_render_data->get_render_scene_data()->get_uniform_buffer();

	PackedByteArray config_data_bytes = {};
	config_data_bytes.resize(sizeof(ConfigData));
	config_data_bytes.encode_float(0, config_data.depth_bias);
	config_data_bytes.encode_u32(4, config_data.use_soft_depth_test_modification);

	p_rd->buffer_update((RID const&)resources[Binding::BINDING_CONFIG_UNIFORM], 0, sizeof(ConfigData), config_data_bytes);
	return Error::OK;
}

void GdstrokeShaderInterface::SceneInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_SCENE_DATA_UNIFORM, RenderingDevice::UniformType::UNIFORM_TYPE_UNIFORM_BUFFER, resources[Binding::BINDING_SCENE_DATA_UNIFORM]));
	bindings.append(new_uniform(Binding::BINDING_CONFIG_UNIFORM,     RenderingDevice::UniformType::UNIFORM_TYPE_UNIFORM_BUFFER, resources[Binding::BINDING_CONFIG_UNIFORM    ]));
}


RID GdstrokeShaderInterface::CommandInterfaceSet::get_dispatch_indirect_commands_buffer() const {
	ERR_FAIL_COND_V(resources.size() == 0, RID());
	return resources[Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER];
}

RID GdstrokeShaderInterface::CommandInterfaceSet::get_draw_indirect_commands_buffer() const {
	ERR_FAIL_COND_V(resources.size() == 0, RID());
	return resources[Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER];
}

void GdstrokeShaderInterface::CommandInterfaceSet::dispatch_indirect(RenderingDevice *p_rd, int64_t p_compute_list, DispatchIndirectCommands cmd) const {
	p_rd->compute_list_dispatch_indirect(p_compute_list, get_dispatch_indirect_commands_buffer(), cmd * sizeof(DispatchIndirectCommand));
}

void GdstrokeShaderInterface::CommandInterfaceSet::draw_indirect(RenderingDevice *p_rd, int64_t p_draw_list, DrawIndirectCommands cmd) const {
	p_rd->draw_list_draw_indirect(p_draw_list, false, get_draw_indirect_commands_buffer(), cmd * sizeof(DrawIndirectCommand));
}

Error GdstrokeShaderInterface::CommandInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources = {};
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER] = p_rd->storage_buffer_create(sizeof(DispatchIndirectCommand) * DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_MAX, {}, RenderingDevice::StorageBufferUsage::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);
	resources[Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER] = p_rd->storage_buffer_create(sizeof(DrawIndirectCommand) * DrawIndirectCommands::DRAW_INDIRECT_COMMANDS_MAX, {}, RenderingDevice::StorageBufferUsage::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);
	return Error::OK;
}

Error GdstrokeShaderInterface::CommandInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::CommandInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER]));
	bindings.append(new_uniform(Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER]));
}


Error GdstrokeShaderInterface::MeshInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources = {};
	resources.resize(Binding::BINDING_MAX);

	GdstrokeServer::ContourMesh const &contour_mesh = GdstrokeServer::get_singleton()->get_contour_mesh();
	Transform3D contour_instance_transform = GdstrokeServer::get_singleton()->get_contour_instance()->get_global_transform().inverse();
	PackedByteArray mesh_desc_buffer_data = PackedByteArray();
	mesh_desc_buffer_data.resize(4 * sizeof(int32_t));
	mesh_desc_buffer_data.encode_s32(0, contour_mesh.vertex_buffer.size());
	mesh_desc_buffer_data.encode_s32(4, contour_mesh.edge_to_vertex_buffer.size());
	mesh_desc_buffer_data.encode_s32(8, contour_mesh.face_to_vertex_buffer.size());
	mesh_desc_buffer_data.encode_s32(12, 0);
	mesh_desc_buffer_data.append_array(PackedVector4Array({
		ctor_vec3_f(contour_instance_transform.get_basis()[0]),
		ctor_vec3_f(contour_instance_transform.get_basis()[1]),
		ctor_vec3_f(contour_instance_transform.get_basis()[2]),
		ctor_vec3_f(contour_instance_transform.get_origin(), 1.0),
	}).to_byte_array());

	// todo: maybe some functions to make serialization less ass
	PackedByteArray vertex_buffer_data = PackedByteArray();
	for (int i = 0; i < contour_mesh.vertex_buffer.size(); ++i) {
		vertex_buffer_data.append_array(PackedVector4Array({
			ctor_vec3_f(contour_mesh.vertex_buffer.get(i)),
		}).to_byte_array());
	}
	PackedByteArray edge_to_vertex_buffer_data = PackedByteArray();
	for (int i = 0; i < contour_mesh.edge_to_vertex_buffer.size(); ++i) {
		edge_to_vertex_buffer_data.append_array(PackedInt32Array({
			contour_mesh.edge_to_vertex_buffer.get(i)[0],
			contour_mesh.edge_to_vertex_buffer.get(i)[1],
		}).to_byte_array());
	}
	PackedByteArray edge_to_face_buffer_data = PackedByteArray();
	for (int i = 0; i < contour_mesh.edge_to_face_buffer.size(); ++i) {
		edge_to_face_buffer_data.append_array(PackedInt32Array({
			contour_mesh.edge_to_face_buffer.get(i)[0],
			contour_mesh.edge_to_face_buffer.get(i)[1],
		}).to_byte_array());
	}
	PackedByteArray edge_is_concave_buffer_data = PackedByteArray();
	for (int i = 0; i < contour_mesh.edge_is_concave_buffer.size(); ++i) {
		edge_is_concave_buffer_data.append_array(PackedInt32Array({
			contour_mesh.edge_is_concave_buffer.get(i),
		}).to_byte_array());
	}
	PackedByteArray face_to_vertex_buffer_data = PackedByteArray();
	for (int i = 0; i < contour_mesh.face_to_vertex_buffer.size(); ++i) {
		face_to_vertex_buffer_data.append_array(PackedInt32Array({
			contour_mesh.face_to_vertex_buffer.get(i)[0],
			contour_mesh.face_to_vertex_buffer.get(i)[1],
			contour_mesh.face_to_vertex_buffer.get(i)[2],
			0,
		}).to_byte_array());
	}
	PackedByteArray face_normal_buffer_data = PackedByteArray();
	for (int i = 0; i < contour_mesh.face_normal_buffer.size(); ++i) {
		face_normal_buffer_data.append_array(PackedVector4Array({
			ctor_vec3_f(contour_mesh.face_normal_buffer.get(i)),
		}).to_byte_array());
	}

	resources[Binding::BINDING_MESH_DESC_BUFFER           ] = p_rd->storage_buffer_create(mesh_desc_buffer_data.size(),            mesh_desc_buffer_data);
	resources[Binding::BINDING_VERTEX_BUFFER              ] = p_rd->storage_buffer_create(vertex_buffer_data.size(),               vertex_buffer_data);
	resources[Binding::BINDING_EDGE_TO_VERTEX_BUFFER      ] = p_rd->storage_buffer_create(edge_to_vertex_buffer_data.size(),       edge_to_vertex_buffer_data);
	resources[Binding::BINDING_EDGE_TO_FACE_BUFFER        ] = p_rd->storage_buffer_create(edge_to_face_buffer_data.size(),         edge_to_face_buffer_data);
	resources[Binding::BINDING_EDGE_IS_CONCAVE_BUFFER     ] = p_rd->storage_buffer_create(edge_is_concave_buffer_data.size(),      edge_is_concave_buffer_data);
	resources[Binding::BINDING_EDGE_IS_CONTOUR_BUFFER     ] = p_rd->storage_buffer_create(contour_mesh.edge_to_vertex_buffer.size() * sizeof(int32_t));
	resources[Binding::BINDING_EDGE_TO_CONTOUR_EDGE_BUFFER] = p_rd->storage_buffer_create(contour_mesh.edge_to_vertex_buffer.size() * sizeof(int32_t));
	resources[Binding::BINDING_FACE_TO_VERTEX_BUFFER      ] = p_rd->storage_buffer_create(face_to_vertex_buffer_data.size(),       face_to_vertex_buffer_data);
	resources[Binding::BINDING_FACE_NORMAL_BUFFER         ] = p_rd->storage_buffer_create(face_normal_buffer_data.size(),          face_normal_buffer_data);
	resources[Binding::BINDING_FACE_BACKFACING_BUFFER     ] = p_rd->storage_buffer_create(contour_mesh.face_to_vertex_buffer.size() * sizeof(int32_t));

	return Error::OK;
}

Error GdstrokeShaderInterface::MeshInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::MeshInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_MESH_DESC_BUFFER,            RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_MESH_DESC_BUFFER           ]));
	bindings.append(new_uniform(Binding::BINDING_VERTEX_BUFFER,               RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_VERTEX_BUFFER              ]));
	bindings.append(new_uniform(Binding::BINDING_EDGE_TO_VERTEX_BUFFER,       RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_EDGE_TO_VERTEX_BUFFER      ]));
	bindings.append(new_uniform(Binding::BINDING_EDGE_TO_FACE_BUFFER,         RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_EDGE_TO_FACE_BUFFER        ]));
	bindings.append(new_uniform(Binding::BINDING_EDGE_IS_CONCAVE_BUFFER,      RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_EDGE_IS_CONCAVE_BUFFER     ]));
	bindings.append(new_uniform(Binding::BINDING_EDGE_IS_CONTOUR_BUFFER,      RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_EDGE_IS_CONTOUR_BUFFER     ]));
	bindings.append(new_uniform(Binding::BINDING_EDGE_TO_CONTOUR_EDGE_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_EDGE_TO_CONTOUR_EDGE_BUFFER]));
	bindings.append(new_uniform(Binding::BINDING_FACE_TO_VERTEX_BUFFER,       RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_FACE_TO_VERTEX_BUFFER      ]));
	bindings.append(new_uniform(Binding::BINDING_FACE_NORMAL_BUFFER,          RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_FACE_NORMAL_BUFFER         ]));
	bindings.append(new_uniform(Binding::BINDING_FACE_BACKFACING_BUFFER,      RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_FACE_BACKFACING_BUFFER     ]));
}


void GdstrokeShaderInterface::ContourInterfaceSet::receive_hard_depth_test_attachments(TypedArray<RID> p_attachments) {
	resources[Binding::BINDING_FOREMOST_FRAGMENT_BITMAP] = p_attachments[0];
}

Error GdstrokeShaderInterface::ContourInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();
	uint32_t num_edges = GdstrokeServer::get_singleton()->get_contour_mesh().edge_to_vertex_buffer.size();

	resources = {};
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_CONTOUR_DESC_BUFFER                    ] = p_rd->storage_buffer_create(sizeof(int32_t) * 6);
	resources[Binding::BINDING_CONTOUR_EDGE_TO_EDGE_BUFFER            ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * num_edges);
	resources[Binding::BINDING_CONTOUR_EDGE_TO_CONTOUR_FRAGMENT_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * num_edges);

	resources[Binding::BINDING_CONTOUR_FRAGMENT_PIXEL_COORD_BUFFER   ] = p_rd->storage_buffer_create(sizeof(int32_t)  * 2 * max_num_contour_fragments);
	resources[Binding::BINDING_CONTOUR_FRAGMENT_ORIENTATION_BUFFER   ] = p_rd->storage_buffer_create(sizeof(float)    * 2 * max_num_contour_fragments);
	resources[Binding::BINDING_CONTOUR_FRAGMENT_NORMAL_DEPTH_BUFFER  ] = p_rd->storage_buffer_create(sizeof(float)    * 4 * max_num_contour_fragments);
	resources[Binding::BINDING_CONTOUR_FRAGMENT_PSEUDO_VISIBLE_BUFFER] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_contour_fragments);

	resources[Binding::BINDING_SCREEN_DEPTH_TEXTURE] = render_scene_buffers->get_depth_texture();

	resources[Binding::BINDING_ALLOCATION_CONTOUR_PIXEL_BUFFER] = p_rd->storage_buffer_create(sizeof(uint32_t) * 2 * max_num_contour_fragments);

	resources[Binding::BINDING_FOREMOST_CONTOUR_FRAGMENT_TO_CONTOUR_PIXEL_BUFFER] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_contour_fragments);

	resources[Binding::BINDING_CONTOUR_PIXEL_PIXEL_COORD_BUFFER ] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * max_num_contour_pixels);
	resources[Binding::BINDING_CONTOUR_PIXEL_ORIENTATION_BUFFER ] = p_rd->storage_buffer_create(sizeof(float)   * 2 * max_num_contour_pixels);
	resources[Binding::BINDING_CONTOUR_PIXEL_NORMAL_DEPTH_BUFFER] = p_rd->storage_buffer_create(sizeof(float)   * 4 * max_num_contour_pixels);

	Ref<RDSamplerState> nearest_sampler_state = Ref(memnew(RDSamplerState));
	nearest_sampler = p_rd->sampler_create(nearest_sampler_state);
	return Error::OK;
}

Error GdstrokeShaderInterface::ContourInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(resources.size() == 0, Error::FAILED);
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();

	resources[Binding::BINDING_SCREEN_DEPTH_TEXTURE] = render_scene_buffers->get_depth_texture();

	return Error::OK;
}

void GdstrokeShaderInterface::ContourInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);

	for (int i = 0; i < Binding::BINDING_MAX; ++i) {
		if (Binding(i) == Binding::BINDING_SCREEN_DEPTH_TEXTURE) {
			ERR_FAIL_COND(!nearest_sampler.is_valid());
			bindings.append(new_uniform(Binding::BINDING_SCREEN_DEPTH_TEXTURE, RenderingDevice::UniformType::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, nearest_sampler, resources[Binding::BINDING_SCREEN_DEPTH_TEXTURE]));
		}
		else if (Binding(i) == Binding::BINDING_FOREMOST_FRAGMENT_BITMAP) {
			bindings.append(new_uniform(Binding::BINDING_FOREMOST_FRAGMENT_BITMAP, RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE, resources[Binding::BINDING_FOREMOST_FRAGMENT_BITMAP]));
		}
		else {
			bindings.append(new_uniform(Binding(i), RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding(i)]));
		}
	}
}

TypedArray<Ref<RDUniform>> GdstrokeShaderInterface::ContourInterfaceSet::get_draw_bindings() const {
	TypedArray<Ref<RDUniform>> draw_bindings = bindings.duplicate();
	draw_bindings.remove_at(draw_bindings.find(bindings[Binding::BINDING_SCREEN_DEPTH_TEXTURE]));
	draw_bindings.remove_at(draw_bindings.find(bindings[Binding::BINDING_FOREMOST_FRAGMENT_BITMAP]));
	return draw_bindings;
}


Error GdstrokeShaderInterface::PixelEdgeInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources = {};
	resources.resize(Binding::BINDING_MAX);

	resources[Binding::BINDING_PIXEL_EDGE_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 4);

	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_NEIGHBOURS_BUFFER        ] = p_rd->storage_buffer_create(sizeof(int32_t)  * 2 * max_num_sparse_pixel_edges);
	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_MORTON_CODE_BUFFER       ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_sparse_pixel_edges);
	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_LOOP_BREAKING_BUFFER     ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 8 * max_num_sparse_pixel_edges);
	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_LIST_RANKING_BUFFER      ] = p_rd->storage_buffer_create(sizeof(int32_t)  * 4 * max_num_sparse_pixel_edges);
	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER   ] = p_rd->storage_buffer_create(sizeof(int32_t)  * 1 * max_num_sparse_pixel_edges);
	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_LOCAL_IDX_BUFFER         ] = p_rd->storage_buffer_create(sizeof(int32_t)  * 1 * max_num_sparse_pixel_edges);
	resources[Binding::BINDING_SPARSE_PIXEL_EDGE_TO_PIXEL_EDGE_LOOP_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t)  * 1 * max_num_sparse_pixel_edges);

	resources[Binding::BINDING_PIXEL_EDGE_LOOP_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 4 * max_num_pixel_edge_loops);

	resources[Binding::BINDING_COMPACTED_PIXEL_EDGE_NEIGHBOURS_BUFFER      ] = p_rd->storage_buffer_create(sizeof( int32_t) * 2 * max_num_compacted_pixel_edges);
	resources[Binding::BINDING_COMPACTED_PIXEL_EDGE_TO_CONTOUR_PIXEL_BUFFER] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges);
	resources[Binding::BINDING_COMPACTED_PIXEL_EDGE_ORIENTATION_BUFFER     ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_compacted_pixel_edges);
	resources[Binding::BINDING_COMPACTED_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_compacted_pixel_edges);

	return Error::OK;
}

Error GdstrokeShaderInterface::PixelEdgeInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::PixelEdgeInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);

	for (int i = 0; i < Binding::BINDING_MAX; ++i) {
		bindings.append(new_uniform(Binding(i), RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding(i)]));
	}
}


Error GdstrokeShaderInterface::DebugInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();
	resources = {};
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_CONTOUR_SCREEN_COLOR_IMAGE] = render_scene_buffers->get_color_texture();
	return Error::OK;
}

Error GdstrokeShaderInterface::DebugInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(resources.size() == 0, Error::FAILED);
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();
	resources[Binding::BINDING_CONTOUR_SCREEN_COLOR_IMAGE] = render_scene_buffers->get_color_texture();
	return Error::OK;
}

void GdstrokeShaderInterface::DebugInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_CONTOUR_SCREEN_COLOR_IMAGE, RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE, resources[Binding::BINDING_CONTOUR_SCREEN_COLOR_IMAGE]));
}


TypedArray<uint32_t> GdstrokeShaderInterface::HardDepthTestResources::get_attachment_usage_flags() const {
	return {
		( // color
			RenderingDevice::TextureUsageBits::TEXTURE_USAGE_STORAGE_BIT |
			RenderingDevice::TextureUsageBits::TEXTURE_USAGE_STORAGE_ATOMIC_BIT |
			RenderingDevice::TextureUsageBits::TEXTURE_USAGE_COLOR_ATTACHMENT_BIT |
			RenderingDevice::TextureUsageBits::TEXTURE_USAGE_CAN_COPY_TO_BIT
		),
		( // depth
			RenderingDevice::TextureUsageBits::TEXTURE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
			RenderingDevice::TextureUsageBits::TEXTURE_USAGE_CAN_COPY_TO_BIT
		),
	};
}

TypedArray<RID> GdstrokeShaderInterface::HardDepthTestResources::get_attachments(RenderingDevice *p_rd, RenderData *p_render_data) {
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();
	Vector2i new_size = render_scene_buffers->get_internal_size();

	if (prev_internal_size != new_size || !p_rd->texture_is_valid(color_attachment) || !p_rd->texture_is_valid(depth_attachment)) {
		if (p_rd->texture_is_valid(color_attachment)) {
			p_rd->free_rid(color_attachment);
		}
		if (p_rd->texture_is_valid(depth_attachment)) {
			p_rd->free_rid(depth_attachment);
		}

		Ref<RDTextureFormat> color_format = Ref(memnew(RDTextureFormat));
		color_format->set_format(RenderingDevice::DataFormat::DATA_FORMAT_R32_UINT);
		color_format->set_width(new_size.x);
		color_format->set_height(new_size.y);
		color_format->set_usage_bits((uint32_t)get_attachment_usage_flags()[0]);
		Ref<RDTextureView> color_view = Ref(memnew(RDTextureView));
		color_attachment = p_rd->texture_create(color_format, color_view);

		Ref<RDTextureFormat> depth_format = Ref(memnew(RDTextureFormat));
		depth_format->set_format(RenderingDevice::DataFormat::DATA_FORMAT_D32_SFLOAT);
		depth_format->set_width(new_size.x);
		depth_format->set_height(new_size.y);
		depth_format->set_usage_bits((uint32_t)get_attachment_usage_flags()[1]);
		Ref<RDTextureView> depth_view = Ref(memnew(RDTextureView));
		depth_attachment = p_rd->texture_create(depth_format, depth_view);
	}
	prev_internal_size = new_size;
	return { color_attachment, depth_attachment, };
}

int64_t GdstrokeShaderInterface::HardDepthTestResources::get_framebuffer_format(RenderingDevice *p_rd) {
	if (framebuffer_format == RenderingDevice::INVALID_FORMAT_ID) {
		Ref<RDAttachmentFormat> attachment_format_color = Ref(memnew(RDAttachmentFormat));
		attachment_format_color->set_format(RenderingDevice::DataFormat::DATA_FORMAT_R32_UINT);
		attachment_format_color->set_usage_flags(get_attachment_usage_flags()[0]);

		Ref<RDAttachmentFormat> attachment_format_depth = Ref(memnew(RDAttachmentFormat));
		attachment_format_depth->set_format(RenderingDevice::DataFormat::DATA_FORMAT_D32_SFLOAT);
		attachment_format_depth->set_usage_flags(get_attachment_usage_flags()[1]);

		Ref<RDFramebufferPass> framebuffer_pass = Ref(memnew(RDFramebufferPass));
		framebuffer_pass->set_color_attachments({0});
		framebuffer_pass->set_depth_attachment(1);

		framebuffer_format = p_rd->framebuffer_format_create_multipass({ attachment_format_color, attachment_format_depth }, { framebuffer_pass }, 1);
	}
	return framebuffer_format;
}

RID GdstrokeShaderInterface::HardDepthTestResources::get_framebuffer(RenderingDevice *p_rd, RenderData *p_render_data) {
	Ref<RDFramebufferPass> framebuffer_pass = Ref(memnew(RDFramebufferPass));
	framebuffer_pass->set_color_attachments({0});
	framebuffer_pass->set_depth_attachment(1);
	return FramebufferCacheRD::get_cache_multipass(get_attachments(p_rd, p_render_data), { framebuffer_pass }, 1);
}

RID GdstrokeShaderInterface::HardDepthTestResources::create_render_pipeline(RenderingDevice *p_rd, RID const &p_shader) {
	Ref<RDPipelineRasterizationState> pipeline_rasterization_state = Ref(memnew(RDPipelineRasterizationState));
	Ref<RDPipelineMultisampleState> pipeline_multisample_state = Ref(memnew(RDPipelineMultisampleState));
	Ref<RDPipelineDepthStencilState> pipeline_depth_stencil_state = Ref(memnew(RDPipelineDepthStencilState));
	pipeline_depth_stencil_state->set_enable_depth_test(true);
	pipeline_depth_stencil_state->set_enable_depth_write(true);
	pipeline_depth_stencil_state->set_depth_compare_operator(RenderingDevice::CompareOperator::COMPARE_OP_GREATER);

	Ref<RDPipelineColorBlendState> pipeline_color_blend_state = Ref(memnew(RDPipelineColorBlendState));
	Ref<RDPipelineColorBlendStateAttachment> pipeline_color_blend_state_attachment = Ref(memnew(RDPipelineColorBlendStateAttachment));
	pipeline_color_blend_state_attachment->set_src_color_blend_factor(RenderingDevice::BlendFactor::BLEND_FACTOR_ONE);
	pipeline_color_blend_state_attachment->set_dst_color_blend_factor(RenderingDevice::BlendFactor::BLEND_FACTOR_ZERO);
	pipeline_color_blend_state->set_attachments({ pipeline_color_blend_state_attachment });

	return p_rd->render_pipeline_create(p_shader, get_framebuffer_format(p_rd), RenderingDevice::INVALID_FORMAT_ID, RenderingDevice::RenderPrimitive::RENDER_PRIMITIVE_POINTS, pipeline_rasterization_state, pipeline_multisample_state, pipeline_depth_stencil_state, pipeline_color_blend_state);
}

void GdstrokeShaderInterface::HardDepthTestResources::clear_attachments(RenderingDevice *p_rd, RenderData *p_render_data) {
	TypedArray<RID> attachments = get_attachments(p_rd, p_render_data);
	p_rd->texture_clear(attachments[0], Color(0, 0, 0, 0), 0, 1, 0, 1);
	p_rd->texture_clear(attachments[1], Color(0, 0, 0, 0), 0, 1, 0, 1);
}
