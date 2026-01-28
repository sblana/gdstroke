#[vertex]
#version 450
#extension GL_ARB_shading_language_include : enable
#include "../src/gpu/api/gdstroke_stroke.glsli"

layout(location = 0) out vec2 AO_stroke_coord;
layout(location = 1) out float AO_arc_length;
layout(location = 2) out flat float AO_stroke_arc_length;
layout(location = 3) out flat uint AO_stroke_id;

void main() {
	const uint stroke_vertex_idx = gl_VertexIndex;
	const uint segment_edge_idx = gdstroke_get_segment_edge_from_stroke_vertex(stroke_vertex_idx);

	const vec2 tangent = gdstroke_get_stroke_vertex_tangent(stroke_vertex_idx);
	const vec2 normal = gdstroke_get_stroke_vertex_normal(stroke_vertex_idx);

	const vec2 stroke_coord = gdstroke_get_stroke_vertex_coord(stroke_vertex_idx);

	const float width = 7.0 * mix(1.0, 0.4, stroke_coord.x);

	const vec2 aspect = vec2(1.0, gdstroke_get_viewport_size().x / gdstroke_get_viewport_size().y);

	const vec2 segment_edge_screen_pos = gdstroke_get_segment_edge_screen_position(segment_edge_idx);
	const vec2 frag_coord = segment_edge_screen_pos * 2.0 - 1.0 + width * 0.001 * aspect * normal * (stroke_coord.y * 2.0 - 1.0);

	gl_Position = vec4(frag_coord, 0.5, 1.0);

	const uint stroke_id = gdstroke_get_stroke_id_from_segment_edge(segment_edge_idx);

	AO_arc_length = gdstroke_get_segment_edge_arc_length(segment_edge_idx);
	AO_stroke_arc_length = gdstroke_get_stroke_arc_length(stroke_id);
	AO_stroke_id = stroke_id;

	AO_stroke_coord = stroke_coord;
}

#[fragment]
#version 450
#extension GL_ARB_shading_language_include : enable
#include "../src/gpu/api/gdstroke_stroke.glsli"

#define M_PI (3.1415926)

layout(location = 0) in vec2 AI_stroke_coord;
layout(location = 1) in float AI_arc_length;
layout(location = 2) in flat float AI_stroke_arc_length;
layout(location = 3) in flat uint AI_stroke_id;
layout(location = 0) out vec4 AO_color;

void main() {
	float edge_fade = sin(AI_stroke_coord.y * M_PI)
		*      clamp((AI_stroke_arc_length - AI_arc_length) / pow(0.2 * AI_stroke_arc_length, 1.2), 0.0, 1.0)
		* sqrt(clamp((                       AI_arc_length) / 20.0, 0.0, 1.0))
		*      clamp((                       AI_arc_length) / 3.0, 0.0, 1.0)
	;
	vec3 color = vec3(0.7, 0.1, 0.3);
	AO_color = vec4(color, 0.3 * abs(edge_fade));
}
