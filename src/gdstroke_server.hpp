#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include "gdstroke_effect.hpp"
#include <unordered_map>

using namespace godot;

class GdstrokeServer : public Object {
	GDCLASS(GdstrokeServer, Object)

public:
	struct ContourMesh {
		Vector<Vector3> vertex_buffer = {};
		Vector<Vector2i> edge_to_vertex_buffer = {};
		Vector<Vector2i> edge_to_face_buffer = {};
		Vector<bool> edge_is_concave_buffer = {};
		Vector<Vector3i> face_to_vertex_buffer = {};
		Vector<Vector3> face_normal_buffer = {};
	};

protected:
	static void _bind_methods();

public:
	static void init_static();

	static ContourMesh const &get_contour_mesh();
	static MeshInstance3D const *get_contour_instance();

	static void register_contour_instance(MeshInstance3D *p_node);

	static void register_gdstroke_effect(int64_t p_id, Ref<GdstrokeEffect> p_gdstroke_effect);
	static Ref<GdstrokeEffect> get_gdstroke_effect(int64_t p_id);

private:
	static ContourMesh *contour_mesh;
	static MeshInstance3D *contour_instance;

	static std::unordered_map<int64_t, Ref<GdstrokeEffect>> *id_to_gdstroke_effect_map;
};
