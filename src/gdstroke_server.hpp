#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "gdstroke_effect.hpp"

using namespace godot;

class GdstrokeServer : public Object {
	GDCLASS(GdstrokeServer, Object)

public:
	struct ContourMesh {
		int num_vertices;
		int num_edges;
		int num_faces;

		PackedByteArray local_vertex_buffer;
		PackedByteArray local_edge_buffer;
		PackedByteArray local_face_buffer;
	};

private:
	struct TempContourMesh {
		int num_vertices;
		int num_edges;
		int num_faces;

		Vector<Vector3> vertex_buffer;
		Vector<Vector2i> edge_to_vertex_buffer;
		Vector<Vector2i> edge_to_face_buffer;
		Vector<bool> edge_is_concave_buffer;
		Vector<Vector3i> face_to_vertex_buffer;
		Vector<Vector3> face_normal_buffer;
	};

	// key is RID::get_id() referring to Mesh
	static std::unordered_map<int64_t, ContourMesh> _contour_meshes;
	// key is uint64_t(ObjectID) referring to MeshInstance3D
	static std::unordered_set<uint64_t> _contour_instances;
	static std::unordered_map<int64_t, Ref<GdstrokeEffect>> _id_to_gdstroke_effect_map;

	static ContourMesh _process_mesh(Ref<Mesh> p_mesh);

protected:
	static void _bind_methods();

public:
	static void init_static();

	static std::unordered_map<int64_t, ContourMesh> const &get_contour_meshes();
	static std::unordered_set<uint64_t> const &get_contour_instances();

#pragma region // exposed

	static int32_t get_num_contour_meshes();
	static int32_t get_num_contour_instances();

	static bool has_contour_mesh(Ref<Mesh> p_mesh);
	static bool has_contour_instance(MeshInstance3D const *p_node);

	static void register_contour_mesh(Ref<Mesh> p_mesh);

	static void register_contour_instance(MeshInstance3D *p_node);

	static void register_gdstroke_effect(int64_t p_id, Ref<GdstrokeEffect> p_gdstroke_effect);

	static Ref<GdstrokeEffect> get_gdstroke_effect(int64_t p_id);

#pragma endregion // exposed
};
