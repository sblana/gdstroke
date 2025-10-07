#include "gdstroke_server.hpp"


using namespace godot;

GdstrokeServer *GdstrokeServer::singleton = nullptr;
GdstrokeServer::ContourMesh GdstrokeServer::contour_mesh = {};
MeshInstance3D *GdstrokeServer::contour_instance = nullptr;

void GdstrokeServer::_bind_methods() {
	ClassDB::bind_static_method("GdstrokeServer", D_METHOD("get_singleton"), &GdstrokeServer::get_singleton);
	ClassDB::bind_method(D_METHOD("register_contour_instance", "p_node"), &GdstrokeServer::register_contour_instance);
}

void GdstrokeServer::create_singleton() {
	singleton = memnew(GdstrokeServer);
}

GdstrokeServer *GdstrokeServer::get_singleton() {
	return singleton;
}

GdstrokeServer::ContourMesh const &GdstrokeServer::get_contour_mesh() const {
	return contour_mesh;
}

MeshInstance3D const *GdstrokeServer::get_contour_instance() const {
	return contour_instance;
}


void GdstrokeServer::register_contour_instance(MeshInstance3D *p_node) {
	contour_mesh = ContourMesh();
	contour_instance = p_node;

	ERR_FAIL_COND(!p_node->get_mesh().is_valid());

	for (int surface_idx = 0; surface_idx < p_node->get_mesh()->get_surface_count(); ++surface_idx) {
		Array surface_arrays = p_node->get_mesh()->surface_get_arrays(surface_idx);
		PackedVector3Array surface_vertex_pos_buffer = surface_arrays[Mesh::ArrayType::ARRAY_VERTEX];
		PackedVector3Array surface_vertex_normal_buffer = surface_arrays[Mesh::ArrayType::ARRAY_NORMAL];
		PackedInt32Array surface_index_buffer = surface_arrays[Mesh::ArrayType::ARRAY_INDEX];

		// how the fuck do you check if the surface_index_buffer array is null
		bool has_index_array = true;

		ERR_FAIL_COND(!has_index_array);

		if (has_index_array) {
			ERR_FAIL_COND(0 != (surface_index_buffer.size() % 3));
			for (int i = 0; i < (surface_index_buffer.size() / 3); ++i) {
				Vector3i pp_face_vert_idxs = Vector3i(-1,-1,-1);

				Vector3i are_verts_new = Vector3i(true, true, true);
				for (int j = 0; j < 3; ++j) {
					int m_tri_start = i * 3;
					int m_tri_element_idx = m_tri_start + j;
					int m_tri_element_vert_idx = surface_index_buffer[m_tri_element_idx];
					Vector3 m_tri_element_vert_pos = surface_vertex_pos_buffer[m_tri_element_vert_idx];

					for (int k = 0; k < contour_mesh.vertex_buffer.size(); ++k) {
						if (contour_mesh.vertex_buffer[k] == m_tri_element_vert_pos) {
							m_tri_element_vert_idx = k;
							are_verts_new[j] = false;
							break;
						}
					}

					if (are_verts_new[j]) {
						m_tri_element_vert_idx = contour_mesh.vertex_buffer.size();
						contour_mesh.vertex_buffer.append(m_tri_element_vert_pos);
					}

					pp_face_vert_idxs[j] = m_tri_element_vert_idx;
				}

				Vector3 pp_vert_0_to_vert_1 = contour_mesh.vertex_buffer[pp_face_vert_idxs[1]] - contour_mesh.vertex_buffer[pp_face_vert_idxs[0]];
				Vector3 pp_vert_1_to_vert_2 = contour_mesh.vertex_buffer[pp_face_vert_idxs[2]] - contour_mesh.vertex_buffer[pp_face_vert_idxs[1]];
				Vector3 pp_face_normal = pp_vert_0_to_vert_1.cross(pp_vert_1_to_vert_2).normalized();

				int pp_face_idx = contour_mesh.face_to_vertex_buffer.size();
				contour_mesh.face_to_vertex_buffer.append(pp_face_vert_idxs);
				contour_mesh.face_normal_buffer.append(pp_face_normal);

				for (int j = 0; j < 3; ++j) {
					int edge_first_vert = (j + 0) % 3;
					int edge_secnd_vert = (j + 1) % 3;

					int edge_idx = -1;
					if (!are_verts_new[edge_first_vert] && !are_verts_new[edge_secnd_vert]) {
						for (int k = 0; k < contour_mesh.edge_to_vertex_buffer.size(); ++k) {
							if ((contour_mesh.edge_to_vertex_buffer[k][0] == pp_face_vert_idxs[edge_first_vert] ||
								 contour_mesh.edge_to_vertex_buffer[k][1] == pp_face_vert_idxs[edge_first_vert]) &&
								(contour_mesh.edge_to_vertex_buffer[k][0] == pp_face_vert_idxs[edge_secnd_vert] ||
								 contour_mesh.edge_to_vertex_buffer[k][1] == pp_face_vert_idxs[edge_secnd_vert])
							) {
								edge_idx = k;
								break;
							}
						}
					}
					if (-1 == edge_idx) {
						edge_idx = contour_mesh.edge_to_vertex_buffer.size();
						contour_mesh.edge_to_vertex_buffer.append(Vector2i(pp_face_vert_idxs[edge_first_vert], pp_face_vert_idxs[edge_secnd_vert]));
						contour_mesh.edge_to_face_buffer.append(Vector2i(-1, -1));
					}

					if (-1 == contour_mesh.edge_to_face_buffer[edge_idx][0]) {
						(const_cast<Vector2i&>(contour_mesh.edge_to_face_buffer[edge_idx]))[0] = pp_face_idx;
					}
					else if (-1 == contour_mesh.edge_to_face_buffer[edge_idx][1])
						(const_cast<Vector2i&>(contour_mesh.edge_to_face_buffer[edge_idx]))[1] = pp_face_idx;
					else
						// More than 2 faces for this edge
						ERR_FAIL();
				}
			}
		}
	}

	contour_mesh.edge_is_concave_buffer.resize(contour_mesh.edge_to_vertex_buffer.size());
	for (int edge_idx = 0; edge_idx < contour_mesh.edge_to_vertex_buffer.size(); ++edge_idx) {
		Vector2i faces_other_vert_idx = Vector2i(-1, -1);

		for (int face = 0; face < 2; ++face) {
			for (int face_vert = 0; face_vert < 3; ++face_vert) {
				int32_t face_vert_idx = contour_mesh.face_to_vertex_buffer[contour_mesh.edge_to_face_buffer[edge_idx][face]][face_vert];
				if (face_vert_idx != contour_mesh.edge_to_vertex_buffer[edge_idx][0] &&
					face_vert_idx != contour_mesh.edge_to_vertex_buffer[edge_idx][1]
				) {
					faces_other_vert_idx[face] = face_vert_idx;
					break;
				}
			}
			ERR_FAIL_COND(faces_other_vert_idx[face] == -1);
		}

		// see doi.org/10.1561/0600000075 Appendix C.2: FRONTSIDE
		Vector3 face_0_normal = contour_mesh.face_normal_buffer[contour_mesh.edge_to_face_buffer[edge_idx][0]];
		Vector3 face_1_other_vert = contour_mesh.vertex_buffer[faces_other_vert_idx[1]];
		Vector3 any_edge_vertex = contour_mesh.vertex_buffer[contour_mesh.edge_to_vertex_buffer[edge_idx][0]]; // doesn't matter if we use vert 0 or 1

		bool is_face_1_vert_on_front_side = 0.0 > ((face_1_other_vert - any_edge_vertex).dot(face_0_normal));

		contour_mesh.edge_is_concave_buffer.set(edge_idx, is_face_1_vert_on_front_side);
	}

	// TODO: rebuild data with only concave edges and related
}
