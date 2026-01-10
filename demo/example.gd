extends Node


func _ready() -> void:
	Performance.add_custom_monitor("Gdstroke/NumContourMeshes", GdstrokeServer.get_num_contour_meshes)
	Performance.add_custom_monitor("Gdstroke/NumContourInstances", GdstrokeServer.get_num_contour_instances)
