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
	resources.clear();
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_SCENE_DATA_UNIFORM] = p_render_data->get_render_scene_data()->get_uniform_buffer();
	resources[Binding::BINDING_CONFIG_UNIFORM] = p_rd->uniform_buffer_create(sizeof(float) * 8);
	return Error::OK;
}

Error GdstrokeShaderInterface::SceneInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(resources.size() == 0, Error::FAILED);
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	ERR_FAIL_COND_V(!p_render_data->get_render_scene_data()->get_uniform_buffer().is_valid(), Error::FAILED);
	resources[Binding::BINDING_SCENE_DATA_UNIFORM] = p_render_data->get_render_scene_data()->get_uniform_buffer();

	PackedByteArray config_data_bytes;
	config_data_bytes.resize(sizeof(ConfigData));
	config_data_bytes.encode_float(0, config_data.depth_bias);
	config_data_bytes.encode_u32(4, config_data.use_soft_depth_test_modification);
	config_data_bytes.encode_u32(8, config_data.min_segment_length);
	config_data_bytes.encode_float(12, config_data.stroke_width);
	config_data_bytes.encode_float(16, config_data.stroke_width_factor_start);
	config_data_bytes.encode_float(20, config_data.stroke_width_factor_end);

	p_rd->buffer_update((RID const&)resources[Binding::BINDING_CONFIG_UNIFORM], 0, sizeof(ConfigData), config_data_bytes);
	return Error::OK;
}

void GdstrokeShaderInterface::SceneInterfaceSet::make_bindings() {
	bindings.clear();
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
	resources.clear();
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER] = p_rd->storage_buffer_create(sizeof(DispatchIndirectCommand) * DispatchIndirectCommands::DISPATCH_INDIRECT_COMMANDS_MAX, {}, RenderingDevice::StorageBufferUsage::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);
	resources[Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER] = p_rd->storage_buffer_create(sizeof(DrawIndirectCommand) * DrawIndirectCommands::DRAW_INDIRECT_COMMANDS_MAX, {}, RenderingDevice::StorageBufferUsage::STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT);
	return Error::OK;
}

Error GdstrokeShaderInterface::CommandInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::CommandInterfaceSet::make_bindings() {
	bindings.clear();
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER]));
	bindings.append(new_uniform(Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_DRAW_INDIRECT_COMMANDS_BUFFER]));
}


Error GdstrokeShaderInterface::MeshInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources.clear();
	resources.resize(int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MAX));

	resources[Buffer::BUFFER_GEOMETRY_DESC_BUFFER     ] = p_rd->storage_buffer_create(sizeof(int32_t) * 4,              {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	// arbitrary limit
	resources[Buffer::BUFFER_MESH_DESC_BUFFER         ] = p_rd->storage_buffer_create(sizeof(int32_t) * 10 * (1 << 16), {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_MESH_INSTANCE_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 18 * (1 << 16), {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_MESH_INSTANCE_MAPS_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * (1 << 16), {},  0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_ALLOCATION_COLUMN_BUFFER] = p_rd->storage_buffer_create(sizeof(uint32_t) * 2 * 8192, {},  0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	// arbitrary limit
	resources[Buffer::BUFFER_GLOBAL_EDGES_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 4 * (1 << 22), {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_GLOBAL_FACES_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * (1 << 22), {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	PackedByteArray buffers_addresses_data;
	buffers_addresses_data.resize(Buffer::BUFFER_MAX * 8);
	for (int i = 0; i < Buffer::BUFFER_MAX; ++i) {
		buffers_addresses_data.encode_u64(i * 8, p_rd->buffer_get_device_address(resources[i]));
	}

	resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MESH_BUFFERS)] = p_rd->storage_buffer_create(Buffer::BUFFER_MAX * 8, buffers_addresses_data, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	return Error::OK;
}

Error GdstrokeShaderInterface::MeshInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	PackedByteArray geometry_desc_buffer_data = PackedByteArray();
	geometry_desc_buffer_data.resize(sizeof(int32_t) * 2);
	geometry_desc_buffer_data.encode_s32(0, GdstrokeServer::get_num_contour_meshes());
	geometry_desc_buffer_data.encode_s32(4, GdstrokeServer::get_num_contour_instances());

	p_rd->buffer_update(resources[Buffer::BUFFER_GEOMETRY_DESC_BUFFER], 0, geometry_desc_buffer_data.size(), geometry_desc_buffer_data);


	PackedByteArray mesh_desc_buffer_data = PackedByteArray();
	mesh_desc_buffer_data.resize(sizeof(int32_t) * 10 * GdstrokeServer::get_num_contour_meshes());
	uint64_t bytes_written = 0;
	for (auto const &kvp : GdstrokeServer::get_contour_meshes()) {
		mesh_desc_buffer_data.encode_s32(bytes_written + 0, kvp.second.num_vertices);
		mesh_desc_buffer_data.encode_s32(bytes_written + 4, kvp.second.num_edges);
		mesh_desc_buffer_data.encode_s32(bytes_written + 8, kvp.second.num_faces);
		mesh_desc_buffer_data.encode_s32(bytes_written + 12, 0);
		mesh_desc_buffer_data.encode_u64(bytes_written + 16, p_rd->buffer_get_device_address(kvp.second.local_vertex_buffer));
		mesh_desc_buffer_data.encode_u64(bytes_written + 24, p_rd->buffer_get_device_address(kvp.second.local_edge_buffer));
		mesh_desc_buffer_data.encode_u64(bytes_written + 32, p_rd->buffer_get_device_address(kvp.second.local_face_buffer));
		bytes_written += 40;
	}

	p_rd->buffer_update(resources[Buffer::BUFFER_MESH_DESC_BUFFER], 0, mesh_desc_buffer_data.size(), mesh_desc_buffer_data);


	PackedByteArray mesh_instance_desc_buffer_data = PackedByteArray();
	mesh_instance_desc_buffer_data.resize(sizeof(int32_t) * 18 * GdstrokeServer::get_num_contour_instances());
	bytes_written = 0;
	for (auto object_id : GdstrokeServer::get_contour_instances()) {
		MeshInstance3D *contour_instance = cast_to<MeshInstance3D>(ObjectDB::get_instance(ObjectID(object_id)));
		Basis contour_instance_basis = contour_instance->get_global_transform().get_basis().transposed();
		Vector3 contour_instance_origin = contour_instance->get_global_transform().get_origin();

		mesh_instance_desc_buffer_data.encode_float(bytes_written +  0, contour_instance_basis[0][0]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written +  4, contour_instance_basis[0][1]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written +  8, contour_instance_basis[0][2]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 12, 0);

		mesh_instance_desc_buffer_data.encode_float(bytes_written + 16, contour_instance_basis[1][0]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 20, contour_instance_basis[1][1]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 24, contour_instance_basis[1][2]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 28, 0);

		mesh_instance_desc_buffer_data.encode_float(bytes_written + 32, contour_instance_basis[2][0]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 36, contour_instance_basis[2][1]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 40, contour_instance_basis[2][2]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 44, 0);

		mesh_instance_desc_buffer_data.encode_float(bytes_written + 48, contour_instance_origin[0]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 52, contour_instance_origin[1]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 56, contour_instance_origin[2]);
		mesh_instance_desc_buffer_data.encode_float(bytes_written + 60, 1.0);

		mesh_instance_desc_buffer_data.encode_s32(bytes_written + 64, GdstrokeServer::get_contour_meshes_mesh_idx().at(contour_instance->get_mesh()->get_rid().get_id()));
		mesh_instance_desc_buffer_data.encode_s32(bytes_written + 68, 0);
		bytes_written += 72;
	}

	p_rd->buffer_update(resources[Buffer::BUFFER_MESH_INSTANCE_DESC_BUFFER], 0, mesh_instance_desc_buffer_data.size(), mesh_instance_desc_buffer_data);

	return Error::OK;
}

void GdstrokeShaderInterface::MeshInterfaceSet::make_bindings() {
	bindings.clear();
	ERR_FAIL_COND(resources.size() != int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MAX));
	bindings.append(new_uniform(Binding::BINDING_MESH_BUFFERS, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MESH_BUFFERS)]));
}


void GdstrokeShaderInterface::ContourInterfaceSet::receive_hard_depth_test_attachments(TypedArray<RID> p_attachments) {
	resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_FOREMOST_FRAGMENT_BITMAP)] = p_attachments[0];
}

Error GdstrokeShaderInterface::ContourInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();

	resources.clear();
	resources.resize(int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MAX));
	resources[Buffer::BUFFER_CONTOUR_DESC_BUFFER       ] = p_rd->storage_buffer_create(sizeof(int32_t) * 6, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_MEMORY_BLOCK_BUFFER       ] = p_rd->storage_buffer_create(balloc_buffer_size, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_CONTOUR_EDGE_MAPS_BUFFER  ] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_CONTOUR_EDGE_CLIP_T_BUFFER] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_CONTOUR_FRAGMENT_ATTRIBS_BUFFER                  ] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_CONTOUR_FRAGMENT_TO_CONTOUR_EDGE_BUFFER          ] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_CONTOUR_FRAGMENT_PSEUDO_VISIBLE_BUFFER           ] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_ALLOCATION_CONTOUR_PIXEL_BUFFER                  ] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_FOREMOST_CONTOUR_FRAGMENT_TO_CONTOUR_PIXEL_BUFFER] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_CONTOUR_PIXEL_ATTRIBS_BUFFER ] = p_rd->storage_buffer_create(4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_SCREEN_DEPTH_TEXTURE)] = render_scene_buffers->get_depth_texture();

	PackedByteArray buffers_addresses_data;
	buffers_addresses_data.resize(Buffer::BUFFER_MAX * 8);
	for (int i = 0; i < Buffer::BUFFER_MAX; ++i) {
		buffers_addresses_data.encode_u64(i * 8, p_rd->buffer_get_device_address(resources[i]));
	}

	resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_CONTOUR_BUFFERS)] = p_rd->storage_buffer_create(Buffer::BUFFER_MAX * 8, buffers_addresses_data, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	Ref<RDSamplerState> nearest_sampler_state = Ref(memnew(RDSamplerState));
	nearest_sampler = p_rd->sampler_create(nearest_sampler_state);
	return Error::OK;
}

Error GdstrokeShaderInterface::ContourInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(resources.size() == 0, Error::FAILED);
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();

	resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_SCREEN_DEPTH_TEXTURE)] = render_scene_buffers->get_depth_texture();

	return Error::OK;
}

void GdstrokeShaderInterface::ContourInterfaceSet::make_bindings() {
	bindings.clear();
	ERR_FAIL_COND(resources.size() != int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MAX));

	ERR_FAIL_COND(!nearest_sampler.is_valid());
	bindings.append(new_uniform(Binding::BINDING_CONTOUR_BUFFERS,          RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER,                        resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_CONTOUR_BUFFERS)]));
	bindings.append(new_uniform(Binding::BINDING_SCREEN_DEPTH_TEXTURE,     RenderingDevice::UniformType::UNIFORM_TYPE_SAMPLER_WITH_TEXTURE, nearest_sampler, resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_SCREEN_DEPTH_TEXTURE)]));
	bindings.append(new_uniform(Binding::BINDING_FOREMOST_FRAGMENT_BITMAP, RenderingDevice::UniformType::UNIFORM_TYPE_IMAGE,                                 resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_FOREMOST_FRAGMENT_BITMAP)]));
}

TypedArray<Ref<RDUniform>> GdstrokeShaderInterface::ContourInterfaceSet::get_draw_bindings() const {
	TypedArray<Ref<RDUniform>> draw_bindings = bindings.duplicate();
	draw_bindings.remove_at(draw_bindings.find(bindings[Binding::BINDING_SCREEN_DEPTH_TEXTURE]));
	draw_bindings.remove_at(draw_bindings.find(bindings[Binding::BINDING_FOREMOST_FRAGMENT_BITMAP]));
	return draw_bindings;
}


Error GdstrokeShaderInterface::PixelEdgeInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources.clear();
	resources.resize(int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MAX));

	resources[Buffer::BUFFER_PIXEL_EDGE_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 4, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_SPARSE_PIXEL_EDGE_NEIGHBOURS_BUFFER              ] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * max_num_sparse_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_SPARSE_PIXEL_EDGE_IS_VALID_BUFFER                ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_sparse_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_SPARSE_PIXEL_EDGE_TO_FRAGMENTED_PIXEL_EDGE_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_sparse_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_FRAGMENTED_PIXEL_EDGE_TO_SPARSE_PIXEL_EDGE_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_fragmented_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_FRAGMENTED_PIXEL_EDGE_WYLLIE_BUFFER              ] = p_rd->storage_buffer_create(sizeof(int32_t) * 8 * max_num_fragmented_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_FRAGMENTED_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER     ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_fragmented_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_FRAGMENTED_PIXEL_EDGE_IS_HEAD_BUFFER             ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_fragmented_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_FRAGMENTED_PIXEL_EDGE_LOCAL_IDX_BUFFER           ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_fragmented_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_FRAGMENTED_PIXEL_EDGE_TO_PIXEL_EDGE_LOOP_BUFFER  ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_fragmented_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_PIXEL_EDGE_LOOP_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 4 * max_num_pixel_edge_loops, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_NEIGHBOURS_BUFFER            ] = p_rd->storage_buffer_create(sizeof( int32_t) * 2 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_TO_CONTOUR_PIXEL_BUFFER      ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_ORIENTATION_BUFFER           ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER       ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_TO_PIXEL_EDGE_LOOP_BUFFER    ] = p_rd->storage_buffer_create(sizeof(uint32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_FILTERED_MIDPOINT_BUFFER     ] = p_rd->storage_buffer_create(sizeof(   float) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_FILTERED_ORIENTATION_BUFFER  ] = p_rd->storage_buffer_create(sizeof(   float) * 2 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_IS_INSIDE_BUFFER             ] = p_rd->storage_buffer_create(sizeof( int32_t) * 2 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_IS_SEGMENT_HEAD_BUFFER       ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_LOOP_LOCAL_SEGMENT_KEY_BUFFER] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_IS_DISCARDED_BUFFER          ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_SEGMENT_DESC_BUFFER          ] = p_rd->storage_buffer_create(sizeof( int32_t) * 4 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_SEGMENT_KEY_BUFFER           ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_COMPACTED_PIXEL_EDGE_TO_SEGMENT_EDGE_BUFFER       ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_compacted_pixel_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_PIXEL_EDGE_LOOP_SEGMENTS_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof( int32_t) * 4 * max_num_pixel_edge_loops, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_ALLOCATION_SEGMENT_BUFFER           ] = p_rd->storage_buffer_create(sizeof( int32_t) * 2 * max_num_pixel_edge_loops, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_SEGMENT_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof( int32_t) * 2, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_SEGMENT_EDGE_TO_COMPACTED_PIXEL_EDGE_BUFFER] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_segment_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_SEGMENT_EDGE_IS_HEAD_BUFFER                ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_segment_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_SEGMENT_EDGE_ARC_LENGTH_BUFFER             ] = p_rd->storage_buffer_create(sizeof( int32_t) * 1 * max_num_segment_edges, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_SEGMENT_ARC_LENGTH_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_segments, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_SEGMENT_RANGE_BUFFER     ] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * max_num_segments, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_STROKE_DESC_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 1, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_SEGMENT_STROKE_VERTEX_RANGE_BUFFER  ] = p_rd->storage_buffer_create(sizeof(int32_t) * 2 * max_num_segments, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	resources[Buffer::BUFFER_STROKE_VERTEX_TO_SEGMENT_EDGE_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_stroke_vertices, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);
	resources[Buffer::BUFFER_STROKE_VERTEX_KIND_BUFFER           ] = p_rd->storage_buffer_create(sizeof(int32_t) * 1 * max_num_stroke_vertices, {}, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);


	PackedByteArray buffers_addresses_data;
	buffers_addresses_data.resize(Buffer::BUFFER_MAX * 8);
	for (int i = 0; i < Buffer::BUFFER_MAX; ++i) {
		buffers_addresses_data.encode_u64(i * 8, p_rd->buffer_get_device_address(resources[i]));
	}

	resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_PIXEL_EDGE_BUFFERS)] = p_rd->storage_buffer_create(Buffer::BUFFER_MAX * 8, buffers_addresses_data, 0, RenderingDevice::BufferCreationBits::BUFFER_CREATION_DEVICE_ADDRESS_BIT);

	return Error::OK;
}

Error GdstrokeShaderInterface::PixelEdgeInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::PixelEdgeInterfaceSet::make_bindings() {
	bindings.clear();
	ERR_FAIL_COND(resources.size() != int(Buffer::BUFFER_MAX) + int(Binding::BINDING_MAX));
	bindings.append(new_uniform(Binding::BINDING_PIXEL_EDGE_BUFFERS, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[int(Buffer::BUFFER_MAX) + int(Binding::BINDING_PIXEL_EDGE_BUFFERS)]));
}


Error GdstrokeShaderInterface::ShaderAPIInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources.clear();
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_BUFFER_PTR_TABLE_BUFFER] = p_rd->storage_buffer_create(sizeof(uint64_t) * 128);
	return Error::OK;
}

Error GdstrokeShaderInterface::ShaderAPIInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::ShaderAPIInterfaceSet::make_bindings() {
	bindings.clear();
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_BUFFER_PTR_TABLE_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_BUFFER_PTR_TABLE_BUFFER]));
}


Error GdstrokeShaderInterface::DebugInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(p_render_data == nullptr, Error::FAILED);
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();
	resources.clear();
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
	bindings.clear();
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

void GdstrokeShaderInterface::HardDepthTestResources::clear_color_attachments(RenderingDevice *p_rd, RenderData *p_render_data) {
	TypedArray<RID> attachments = get_attachments(p_rd, p_render_data);
	p_rd->texture_clear(attachments[0], Color(0, 0, 0, 0), 0, 1, 0, 1);
}


TypedArray<RID> GdstrokeShaderInterface::StrokeRenderingResources::get_attachments(RenderingDevice *p_rd, RenderData *p_render_data) {
	ERR_FAIL_COND_V(p_render_data == nullptr, {});
	Ref<RenderSceneBuffersRD> render_scene_buffers = (Ref<RenderSceneBuffersRD>)p_render_data->get_render_scene_buffers();

	return { render_scene_buffers->get_color_texture(), render_scene_buffers->get_depth_texture() };
}

int64_t GdstrokeShaderInterface::StrokeRenderingResources::get_framebuffer_format(RenderingDevice *p_rd, RenderData *p_render_data) {
	if (framebuffer_format == RenderingDevice::INVALID_FORMAT_ID) {
		Ref<RDAttachmentFormat> attachment_format_color = Ref(memnew(RDAttachmentFormat));
		attachment_format_color->set_format(p_rd->texture_get_format(get_attachments(p_rd, p_render_data)[0])->get_format());
		attachment_format_color->set_samples(p_rd->texture_get_format(get_attachments(p_rd, p_render_data)[0])->get_samples());
		attachment_format_color->set_usage_flags(p_rd->texture_get_format(get_attachments(p_rd, p_render_data)[0])->get_usage_bits());

		Ref<RDAttachmentFormat> attachment_format_depth = Ref(memnew(RDAttachmentFormat));
		attachment_format_depth->set_format(p_rd->texture_get_format(get_attachments(p_rd, p_render_data)[1])->get_format());
		attachment_format_depth->set_samples(p_rd->texture_get_format(get_attachments(p_rd, p_render_data)[1])->get_samples());
		attachment_format_depth->set_usage_flags(p_rd->texture_get_format(get_attachments(p_rd, p_render_data)[1])->get_usage_bits());

		framebuffer_format = p_rd->framebuffer_format_create({ attachment_format_color, attachment_format_depth }, 1);
	}
	return framebuffer_format;
}

RID GdstrokeShaderInterface::StrokeRenderingResources::get_framebuffer(RenderingDevice *p_rd, RenderData *p_render_data) {
	RID framebuffer = FramebufferCacheRD::get_cache_multipass(get_attachments(p_rd, p_render_data), {}, 1);
	ERR_FAIL_COND_V(get_framebuffer_format(p_rd, p_render_data) != p_rd->framebuffer_get_format(framebuffer), {});
	return framebuffer;
}

RID GdstrokeShaderInterface::StrokeRenderingResources::create_render_pipeline(RenderingDevice *p_rd, RenderData *p_render_data, RID const &p_shader) {
	Ref<RDPipelineRasterizationState> pipeline_rasterization_state = Ref(memnew(RDPipelineRasterizationState));
	Ref<RDPipelineMultisampleState> pipeline_multisample_state = Ref(memnew(RDPipelineMultisampleState));
	Ref<RDPipelineDepthStencilState> pipeline_depth_stencil_state = Ref(memnew(RDPipelineDepthStencilState));

	Ref<RDPipelineColorBlendState> pipeline_color_blend_state = Ref(memnew(RDPipelineColorBlendState));
	Ref<RDPipelineColorBlendStateAttachment> pipeline_color_blend_state_attachment = Ref(memnew(RDPipelineColorBlendStateAttachment));
	pipeline_color_blend_state_attachment->set_src_color_blend_factor(RenderingDevice::BlendFactor::BLEND_FACTOR_ONE);
	pipeline_color_blend_state_attachment->set_dst_color_blend_factor(RenderingDevice::BlendFactor::BLEND_FACTOR_ZERO);
	pipeline_color_blend_state->set_attachments({ pipeline_color_blend_state_attachment });

	return p_rd->render_pipeline_create(p_shader, get_framebuffer_format(p_rd, p_render_data), RenderingDevice::INVALID_FORMAT_ID, RenderingDevice::RenderPrimitive::RENDER_PRIMITIVE_TRIANGLES, pipeline_rasterization_state, pipeline_multisample_state, pipeline_depth_stencil_state, pipeline_color_blend_state);
}
