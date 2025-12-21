extends MeshInstance3D

func _ready() -> void:
	GdstrokeServer.register_contour_instance(self)
