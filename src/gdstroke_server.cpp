#include "gdstroke_server.hpp"


using namespace godot;

GdstrokeServer *GdstrokeServer::singleton = nullptr;

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
	return this->contour_mesh;
}


void GdstrokeServer::register_contour_instance(MeshInstance3D *p_node) {
	this->contour_mesh = ContourMesh();

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

					for (int k = 0; k < this->contour_mesh.vertex_buffer.size(); ++k) {
						if (this->contour_mesh.vertex_buffer[k] == m_tri_element_vert_pos) {
							m_tri_element_vert_idx = k;
							are_verts_new[j] = false;
							break;
						}
					}

					if (are_verts_new[j]) {
						m_tri_element_vert_idx = this->contour_mesh.vertex_buffer.size();
						this->contour_mesh.vertex_buffer.append(m_tri_element_vert_pos);
					}

					pp_face_vert_idxs[j] = m_tri_element_vert_idx;
				}

				Vector3 pp_vert_0_to_vert_1 = this->contour_mesh.vertex_buffer[pp_face_vert_idxs[1]] - this->contour_mesh.vertex_buffer[pp_face_vert_idxs[0]];
				Vector3 pp_vert_1_to_vert_2 = this->contour_mesh.vertex_buffer[pp_face_vert_idxs[2]] - this->contour_mesh.vertex_buffer[pp_face_vert_idxs[1]];
				Vector3 pp_face_normal = pp_vert_0_to_vert_1.cross(pp_vert_1_to_vert_2).normalized();

				int pp_face_idx = this->contour_mesh.face_to_vertex_buffer.size();
				this->contour_mesh.face_to_vertex_buffer.append(pp_face_vert_idxs);
				this->contour_mesh.face_normal_buffer.append(pp_face_normal);

				for (int j = 0; j < 3; ++j) {
					int edge_first_vert = (j + 0) % 3;
					int edge_secnd_vert = (j + 1) % 3;

					int edge_idx = -1;
					if (!are_verts_new[edge_first_vert] && !are_verts_new[edge_secnd_vert]) {
						for (int k = 0; k < this->contour_mesh.edge_to_vertex_buffer.size(); ++k) {
							if ((this->contour_mesh.edge_to_vertex_buffer[k][0] == pp_face_vert_idxs[edge_first_vert] ||
								 this->contour_mesh.edge_to_vertex_buffer[k][1] == pp_face_vert_idxs[edge_first_vert]) &&
								(this->contour_mesh.edge_to_vertex_buffer[k][0] == pp_face_vert_idxs[edge_secnd_vert] ||
								 this->contour_mesh.edge_to_vertex_buffer[k][1] == pp_face_vert_idxs[edge_secnd_vert])
							) {
								edge_idx = k;
								break;
							}
						}
					}
					if (-1 == edge_idx) {
						edge_idx = this->contour_mesh.edge_to_vertex_buffer.size();
						this->contour_mesh.edge_to_vertex_buffer.append(Vector2i(pp_face_vert_idxs[edge_first_vert], pp_face_vert_idxs[edge_secnd_vert]));
						this->contour_mesh.edge_to_face_buffer.append(Vector2i(-1, -1));
					}

					if (-1 == this->contour_mesh.edge_to_face_buffer[edge_idx][0]) {
						(const_cast<Vector2i&>(this->contour_mesh.edge_to_face_buffer[edge_idx]))[0] = pp_face_idx;
					}
					else if (-1 == this->contour_mesh.edge_to_face_buffer[edge_idx][1])
						(const_cast<Vector2i&>(this->contour_mesh.edge_to_face_buffer[edge_idx]))[1] = pp_face_idx;
					else
						// More than 2 faces for this edge
						ERR_FAIL();
				}
			}
		}
	}
}
