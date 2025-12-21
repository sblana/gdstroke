#[vertex]
#version 450
#extension GL_ARB_shading_language_include : enable
#include "../src/gpu/api/gdstroke_stroke.glsli"

layout(location = 0) out vec2 AO_stroke_coord;

void main() {
	const uint stroke_vertex_idx = gl_VertexIndex;
	const uint segment_edge_idx = gdstroke_get_segment_edge_from_stroke_vertex(stroke_vertex_idx);

	const vec2 tangent = gdstroke_get_stroke_vertex_tangent(stroke_vertex_idx);
	const vec2 normal = gdstroke_get_stroke_vertex_normal(stroke_vertex_idx);

	const vec2 stroke_coord = gdstroke_get_stroke_coord(stroke_vertex_idx);

	const float width = 5.0 * mix(1.0, 0.25, stroke_coord.x);

	const vec2 aspect = vec2(1.0, gdstroke_get_viewport_size().x / gdstroke_get_viewport_size().y);

	const vec2 segment_edge_screen_pos = gdstroke_get_segment_edge_screen_position(segment_edge_idx);
	const vec2 frag_coord = segment_edge_screen_pos * 2.0 - 1.0 + width * 0.001 * aspect * normal * (stroke_coord.y * 2.0 - 1.0);

	gl_Position = vec4(frag_coord, 0.5, 1.0);

	AO_stroke_coord = stroke_coord;
}

#[fragment]
#version 450
#extension GL_ARB_shading_language_include : enable
#include "../src/gpu/api/gdstroke_stroke.glsli"

#define M_PI (3.1415926)

layout(location = 0) in vec2 AI_stroke_coord;
layout(location = 0) out vec4 AO_color;

void main() {
	float edge_fade = sin(AI_stroke_coord.y * M_PI);
	AO_color = vec4(vec3(0.7, 0.1, 0.3), 0.2 * abs(edge_fade));
}
