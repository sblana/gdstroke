extends MeshInstance3D

func _ready() -> void:
	if not GdstrokeServer.has_contour_mesh(mesh):
		GdstrokeServer.register_contour_mesh(mesh)
	GdstrokeServer.register_contour_instance(self)
