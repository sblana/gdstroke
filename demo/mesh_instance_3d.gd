extends MeshInstance3D


func _enter_tree() -> void:
	if not GdstrokeServer.has_contour_mesh(mesh):
		GdstrokeServer.register_contour_mesh(mesh)
	GdstrokeServer.register_contour_instance(self)


func _exit_tree() -> void:
	GdstrokeServer.unregister_contour_instance(self)
	GdstrokeServer.unregister_contour_mesh(mesh)

