extends MeshInstance3D

func _ready() -> void:
	GdstrokeServer.get_singleton().register_contour_instance(self)
