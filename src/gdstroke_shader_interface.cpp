#include "gdstroke_shader_interface.hpp"

#include <godot_cpp/classes/render_scene_data.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>

#include "rd_util.hpp"
#include "vec_util.hpp"
#include "gdstroke_server.hpp"


using namespace godot;

void GdstrokeShaderInterface::_bind_methods() {}

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
	ERR_FAIL_COND_V(!((RID)resources[Binding::BINDING_SCENE_DATA_UNIFORM]).is_valid(), Error::FAILED);
	resources[Binding::BINDING_SCENE_DATA_UNIFORM] = p_render_data->get_render_scene_data()->get_uniform_buffer();
	return Error::OK;
}

void GdstrokeShaderInterface::SceneInterfaceSet::make_bindings() {
	bindings = {};
	bindings.append(new_uniform(Binding::BINDING_SCENE_DATA_UNIFORM, RenderingDevice::UniformType::UNIFORM_TYPE_UNIFORM_BUFFER, resources[Binding::BINDING_SCENE_DATA_UNIFORM]));
	bindings.append(new_uniform(Binding::BINDING_CONFIG_UNIFORM,     RenderingDevice::UniformType::UNIFORM_TYPE_UNIFORM_BUFFER, resources[Binding::BINDING_CONFIG_UNIFORM    ]));
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

Error GdstrokeShaderInterface::ContourInterfaceSet::create_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	resources = {};
	resources.resize(Binding::BINDING_MAX);
	resources[Binding::BINDING_CONTOUR_DESC_BUFFER]         = p_rd->storage_buffer_create(sizeof(int32_t) * 6);
	resources[Binding::BINDING_CONTOUR_EDGE_TO_EDGE_BUFFER] = p_rd->storage_buffer_create(sizeof(int32_t) * GdstrokeServer::get_singleton()->get_contour_mesh().edge_to_vertex_buffer.size());
	return Error::OK;
}

Error GdstrokeShaderInterface::ContourInterfaceSet::update_resources(RenderingDevice *p_rd, RenderData *p_render_data) {
	return Error::OK;
}

void GdstrokeShaderInterface::ContourInterfaceSet::make_bindings() {
	bindings = {};
	ERR_FAIL_COND(resources.size() != Binding::BINDING_MAX);
	bindings.append(new_uniform(Binding::BINDING_CONTOUR_DESC_BUFFER,         RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_CONTOUR_DESC_BUFFER        ]));
	bindings.append(new_uniform(Binding::BINDING_CONTOUR_EDGE_TO_EDGE_BUFFER, RenderingDevice::UniformType::UNIFORM_TYPE_STORAGE_BUFFER, resources[Binding::BINDING_CONTOUR_EDGE_TO_EDGE_BUFFER]));
}
