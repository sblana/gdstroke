#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class GdstrokeServer : public RefCounted {
	GDCLASS(GdstrokeServer, RefCounted)

	struct ContourMesh {
		Vector<Vector3> vertex_buffer = {};
		Vector<Vector2i> edge_to_vertex_buffer = {};
		Vector<Vector2i> edge_to_face_buffer = {};
		Vector<Vector3i> face_to_vertex_buffer = {};
		Vector<Vector3> face_normal_buffer = {};
	};

protected:
	static void _bind_methods();

public:
	static void create_singleton();
	static GdstrokeServer *get_singleton();

	ContourMesh const &get_contour_mesh() const;

	void register_contour_instance(MeshInstance3D *p_node);

private:
	static GdstrokeServer *singleton;

	ContourMesh contour_mesh;
};
