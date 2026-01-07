#include "gdstroke_server.hpp"

#include <map>
#include <bit>
#include <bitset>

#include "vec_util.hpp"


using namespace godot;

std::unordered_map<int64_t, GdstrokeServer::ContourMesh> GdstrokeServer::_contour_meshes = {};
std::unordered_set<uint64_t> GdstrokeServer::_contour_instances = {};
std::unordered_map<int64_t, Ref<GdstrokeEffect>> GdstrokeServer::_id_to_gdstroke_effect_map = {};

GdstrokeServer::ContourMesh GdstrokeServer::_process_mesh(Ref<Mesh> p_mesh) {
	TempContourMesh temp_contour_mesh = {};

	temp_contour_mesh.num_vertices = 0;
	temp_contour_mesh.num_edges = 0;
	temp_contour_mesh.num_faces = 0;

	for (int surface_idx = 0; surface_idx < p_mesh->get_surface_count(); ++surface_idx) {
		Array surface_arrays = p_mesh->surface_get_arrays(surface_idx);
		PackedVector3Array surface_vertex_pos_buffer = surface_arrays[Mesh::ArrayType::ARRAY_VERTEX];
		PackedVector3Array surface_vertex_normal_buffer = surface_arrays[Mesh::ArrayType::ARRAY_NORMAL];
		PackedInt32Array surface_index_buffer = surface_arrays[Mesh::ArrayType::ARRAY_INDEX];

		// how the fuck do you check if the surface_index_buffer array is null
		bool has_index_array = true;

		ERR_FAIL_COND_V(!has_index_array, {});

		if (has_index_array) {
			temp_contour_mesh.vertex_buffer.resize(temp_contour_mesh.num_vertices + surface_index_buffer.size());
			temp_contour_mesh.face_to_vertex_buffer.resize(temp_contour_mesh.num_faces + surface_index_buffer.size() / 3);
			temp_contour_mesh.face_normal_buffer.resize(temp_contour_mesh.num_faces + surface_index_buffer.size() / 3);
			temp_contour_mesh.edge_to_vertex_buffer.resize(temp_contour_mesh.num_edges + surface_index_buffer.size() / 3 * 2);
			temp_contour_mesh.edge_to_face_buffer.resize(temp_contour_mesh.num_edges + surface_index_buffer.size() / 3 * 2);

			std::unordered_map<std::bitset<96>, int32_t> vert_pos_to_vert_idx;
			std::unordered_map<uint64_t, int32_t> vert_idxs_to_edge_idx;

			ERR_FAIL_COND_V(0 != (surface_index_buffer.size() % 3), {});
			for (int i = 0; i < (surface_index_buffer.size() / 3); ++i) {
				Vector3i pp_face_vert_idxs = Vector3i(-1,-1,-1);

				Vector3i are_verts_new = Vector3i(true, true, true);
				for (int j = 0; j < 3; ++j) {
					int m_tri_start = i * 3;
					int m_tri_element_idx = m_tri_start + j;
					int m_tri_element_vert_idx = surface_index_buffer[m_tri_element_idx];
					Vector3 m_tri_element_vert_pos = surface_vertex_pos_buffer[m_tri_element_vert_idx];

					uint32_t axis_key[3] = {
						std::bit_cast<uint32_t>((float)m_tri_element_vert_pos[0]),
						std::bit_cast<uint32_t>((float)m_tri_element_vert_pos[1]),
						std::bit_cast<uint32_t>((float)m_tri_element_vert_pos[2]),
					};

					std::bitset<96> pos_key = std::bitset<96>(uint64_t(axis_key[0]) << 32 | axis_key[1]) << 32 | std::bitset<96>(axis_key[2]);

					if (vert_pos_to_vert_idx.find(pos_key) != vert_pos_to_vert_idx.end()) {
						int32_t k = vert_pos_to_vert_idx.at(pos_key);
						if (temp_contour_mesh.vertex_buffer[k] == m_tri_element_vert_pos) {
							m_tri_element_vert_idx = k;
							are_verts_new[j] = false;
						}
						else
							ERR_FAIL_V({});
					}
					if (are_verts_new[j]) {
						m_tri_element_vert_idx = temp_contour_mesh.num_vertices;
						temp_contour_mesh.vertex_buffer.set(temp_contour_mesh.num_vertices, m_tri_element_vert_pos);
						vert_pos_to_vert_idx.insert({pos_key, temp_contour_mesh.num_vertices});
						++temp_contour_mesh.num_vertices;
					}

					pp_face_vert_idxs[j] = m_tri_element_vert_idx;
				}

				Vector3 pp_vert_0_to_vert_1 = temp_contour_mesh.vertex_buffer[pp_face_vert_idxs[1]] - temp_contour_mesh.vertex_buffer[pp_face_vert_idxs[0]];
				Vector3 pp_vert_1_to_vert_2 = temp_contour_mesh.vertex_buffer[pp_face_vert_idxs[2]] - temp_contour_mesh.vertex_buffer[pp_face_vert_idxs[1]];
				Vector3 pp_face_normal = pp_vert_0_to_vert_1.cross(pp_vert_1_to_vert_2);
				pp_face_normal.normalize();

				int pp_face_idx = temp_contour_mesh.num_faces;
				temp_contour_mesh.face_to_vertex_buffer.set(temp_contour_mesh.num_faces, pp_face_vert_idxs);
				temp_contour_mesh.face_normal_buffer.set(temp_contour_mesh.num_faces, pp_face_normal);
				++temp_contour_mesh.num_faces;

				for (int j = 0; j < 3; ++j) {
					int edge_first_vert = pp_face_vert_idxs[(j + 0) % 3];
					int edge_secnd_vert = pp_face_vert_idxs[(j + 1) % 3];

					uint64_t key = (
						uint64_t(MIN(edge_first_vert, edge_secnd_vert)) << 32 |
						uint64_t(MAX(edge_first_vert, edge_secnd_vert))
					);
					int edge_idx;
					if (vert_idxs_to_edge_idx.end() != vert_idxs_to_edge_idx.find(key)) {
						edge_idx = vert_idxs_to_edge_idx.at(key);
						if (-1 == temp_contour_mesh.edge_to_face_buffer[edge_idx][1]) {
							temp_contour_mesh.edge_to_face_buffer.set(edge_idx, Vector2i(temp_contour_mesh.edge_to_face_buffer[edge_idx][0], pp_face_idx));
						}
						else
							// More than 2 faces for this edge
							ERR_FAIL_V({});
					}
					else {
						edge_idx = temp_contour_mesh.num_edges;
						temp_contour_mesh.edge_to_vertex_buffer.set(temp_contour_mesh.num_edges, Vector2i(edge_first_vert, edge_secnd_vert));
						temp_contour_mesh.edge_to_face_buffer.set(temp_contour_mesh.num_edges, Vector2i(pp_face_idx, -1));
						++temp_contour_mesh.num_edges;
						vert_idxs_to_edge_idx.insert({
							key,
							edge_idx
						});
					}
				}
			}
		}
	}
	temp_contour_mesh.vertex_buffer.resize(temp_contour_mesh.num_vertices);
	temp_contour_mesh.face_to_vertex_buffer.resize(temp_contour_mesh.num_faces);
	temp_contour_mesh.face_normal_buffer.resize(temp_contour_mesh.num_faces);
	temp_contour_mesh.edge_to_vertex_buffer.resize(temp_contour_mesh.num_edges);
	temp_contour_mesh.edge_to_face_buffer.resize(temp_contour_mesh.num_edges);

	temp_contour_mesh.edge_is_concave_buffer.resize(temp_contour_mesh.num_edges);
	for (int edge_idx = 0; edge_idx < temp_contour_mesh.num_edges; ++edge_idx) {
		Vector2i faces_other_vert_idx = Vector2i(-1, -1);

		for (int face = 0; face < 2; ++face) {
			for (int face_vert = 0; face_vert < 3; ++face_vert) {
				int32_t face_vert_idx = temp_contour_mesh.face_to_vertex_buffer[temp_contour_mesh.edge_to_face_buffer[edge_idx][face]][face_vert];
				if (face_vert_idx != temp_contour_mesh.edge_to_vertex_buffer[edge_idx][0] &&
					face_vert_idx != temp_contour_mesh.edge_to_vertex_buffer[edge_idx][1]
				) {
					faces_other_vert_idx[face] = face_vert_idx;
					break;
				}
			}
			ERR_FAIL_COND_V(faces_other_vert_idx[face] == -1, {});
		}

		// see doi.org/10.1561/0600000075 Appendix C.2: FRONTSIDE
		Vector3 face_0_normal = temp_contour_mesh.face_normal_buffer[temp_contour_mesh.edge_to_face_buffer[edge_idx][0]];
		Vector3 face_1_other_vert = temp_contour_mesh.vertex_buffer[faces_other_vert_idx[1]];
		Vector3 any_edge_vertex = temp_contour_mesh.vertex_buffer[temp_contour_mesh.edge_to_vertex_buffer[edge_idx][0]]; // doesn't matter if we use vert 0 or 1

		bool is_face_1_vert_on_front_side = 0.0 > ((face_1_other_vert - any_edge_vertex).dot(face_0_normal));

		temp_contour_mesh.edge_is_concave_buffer.set(edge_idx, is_face_1_vert_on_front_side);
	}

	// TODO: rebuild data with only concave edges and related

	ContourMesh contour_mesh = {};

	contour_mesh.num_vertices = temp_contour_mesh.num_vertices;
	contour_mesh.num_edges = temp_contour_mesh.num_edges;
	contour_mesh.num_faces = temp_contour_mesh.num_faces;

	for (int i = 0; i < contour_mesh.num_vertices; ++i) {
		contour_mesh.local_vertex_buffer.append_array(PackedVector4Array({
			ctor_vec3_f(temp_contour_mesh.vertex_buffer.get(i)),
		}).to_byte_array());
	}

	for (int i = 0; i < contour_mesh.num_edges; ++i) {
		contour_mesh.local_edge_buffer.append_array(PackedInt32Array({
			temp_contour_mesh.edge_to_vertex_buffer.get(i).x,
			temp_contour_mesh.edge_to_vertex_buffer.get(i).y,
			temp_contour_mesh.edge_to_face_buffer.get(i).x,
			temp_contour_mesh.edge_to_face_buffer.get(i).y,
			temp_contour_mesh.edge_is_concave_buffer.get(i),
			0,
		}).to_byte_array());
	}

	for (int i = 0; i < contour_mesh.num_faces; ++i) {
		contour_mesh.local_face_buffer.append_array(PackedInt32Array({
			temp_contour_mesh.face_to_vertex_buffer.get(i).x,
			temp_contour_mesh.face_to_vertex_buffer.get(i).y,
			temp_contour_mesh.face_to_vertex_buffer.get(i).z,
			0,
		}).to_byte_array());
		contour_mesh.local_face_buffer.append_array(PackedVector4Array({
			ctor_vec3_f(temp_contour_mesh.face_normal_buffer.get(i)),
		}).to_byte_array());
	}

	return contour_mesh;
}

void GdstrokeServer::_bind_methods() {
	ClassDB::bind_static_method("GdstrokeServer", D_METHOD("register_contour_instance", "p_node"), &GdstrokeServer::register_contour_instance);
	ClassDB::bind_static_method("GdstrokeServer", D_METHOD("get_gdstroke_effect", "p_id"), &GdstrokeServer::get_gdstroke_effect);
}

void GdstrokeServer::init_static() {
	_contour_meshes = {};
	_contour_instances = {};
	_id_to_gdstroke_effect_map = {};
}

std::unordered_map<int64_t, GdstrokeServer::ContourMesh> const &GdstrokeServer::get_contour_meshes() {
	return GdstrokeServer::_contour_meshes;
}

std::unordered_set<uint64_t> const &GdstrokeServer::get_contour_instances() {
	return GdstrokeServer::_contour_instances;
}

int32_t GdstrokeServer::get_num_contour_meshes() {
	return GdstrokeServer::_contour_meshes.size();
}

int32_t GdstrokeServer::get_num_contour_instances() {
	return GdstrokeServer::_contour_instances.size();
}

bool GdstrokeServer::has_contour_mesh(Ref<Mesh> p_mesh) {
	ERR_FAIL_NULL_V(p_mesh, false);

	if (!_contour_meshes.contains(p_mesh->get_rid().get_id())) {
		return false;
	}
	return true;
}

bool GdstrokeServer::has_contour_instance(MeshInstance3D const *p_node) {
	ERR_FAIL_NULL_V(p_node, false);

	if (!_contour_instances.contains(p_node->get_instance_id())) {
		return false;
	}
	return true;
}

void GdstrokeServer::register_contour_mesh(Ref<Mesh> p_mesh) {
	ERR_FAIL_COND(has_contour_mesh(p_mesh));

	_contour_meshes.insert({ p_mesh->get_rid().get_id(), _process_mesh(p_mesh) });
}

void GdstrokeServer::register_contour_instance(MeshInstance3D *p_node) {
	ERR_FAIL_COND(has_contour_instance(p_node));
	ERR_FAIL_NULL(p_node->get_mesh());
	ERR_FAIL_COND(has_contour_mesh(p_node->get_mesh()));

	_contour_instances.insert(p_node->get_instance_id());
}

void GdstrokeServer::register_gdstroke_effect(int64_t p_id, Ref<GdstrokeEffect> p_gdstroke_effect) {
	ERR_FAIL_NULL_EDMSG(p_gdstroke_effect, "p_gdstroke_effect is null.");
	ERR_FAIL_COND_EDMSG(_id_to_gdstroke_effect_map.contains(p_id), "p_id is already registered.");

	_id_to_gdstroke_effect_map.insert({ p_id, p_gdstroke_effect });
}

Ref<GdstrokeEffect> GdstrokeServer::get_gdstroke_effect(int64_t p_id) {
	if (_id_to_gdstroke_effect_map.contains(p_id)) {
		return _id_to_gdstroke_effect_map.at(p_id);
	}
	return nullptr;
}
