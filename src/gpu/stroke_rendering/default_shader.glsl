#version 450
#extension GL_ARB_shading_language_include : enable

#include "api/gdstroke_stroke.glsli"


#ifdef STAGE_VERT
void main() {
	const uint stroke_vertex_idx = gl_VertexIndex;
	const uint segment_edge_idx = gdstroke_get_segment_edge_from_stroke_vertex(stroke_vertex_idx);

	const vec2 tangent = gdstroke_get_stroke_vertex_tangent(stroke_vertex_idx);
	const vec2 normal = gdstroke_get_stroke_vertex_normal(stroke_vertex_idx);

	const vec2 stroke_coord = gdstroke_get_stroke_vertex_coord(stroke_vertex_idx);

	const float width = 5.0 * mix(1.0, 0.1, stroke_coord.x);

	const vec2 aspect = vec2(1.0, gdstroke_get_viewport_size().x / gdstroke_get_viewport_size().y);

	const vec2 segment_edge_screen_pos = gdstroke_get_segment_edge_screen_position(segment_edge_idx);
	const vec2 frag_coord = segment_edge_screen_pos * 2.0 - 1.0 + width * 0.001 * aspect * normal * (stroke_coord.y * 2.0 - 1.0);

	gl_Position = vec4(frag_coord, 0.5, 1.0);
}
#endif // STAGE_VERT


#ifdef STAGE_FRAG
layout(location = 0) out vec4 AO_color;

void main() {
	AO_color = vec4(1.0);
}

#endif // STAGE_FRAG
