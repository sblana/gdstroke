#version 450
#extension GL_ARB_shading_language_include : enable

#include "common/buffers.glsli"


#ifdef STAGE_VERT
	layout(location = 0) out smooth vec2 AO_frag_coord;
	layout(location = 1) out smooth vec2 AO_tangent;
	layout(location = 2) out smooth vec2 AO_normal;
	layout(location = 3) out smooth vec2 AO_stroke_coord;
	layout(location = 4) out flat uint AO_segment_key;

	void main() {
		const int stroke_vertex_idx = gl_VertexIndex;
		const int segment_edge_idx = B_stroke_vertex_to_segment_edge.idx[stroke_vertex_idx];
		const int compacted_pixel_edge_idx = B_segment_edge_to_compacted_pixel_edge.idx[segment_edge_idx];
		const int segment_key = B_compacted_pixel_edge_segment_key.segment_key[compacted_pixel_edge_idx];
		const SegmentRangeData segment_range = B_segment_range.data[segment_key];
		const int segment_head_edge_idx = segment_range.first_idx;

		const vec2 midpoint = B_compacted_pixel_edge_filtered_midpoint.midpoint[compacted_pixel_edge_idx];

		const vec2 tangent = normalize(B_compacted_pixel_edge_filtered_orientation.orientation[compacted_pixel_edge_idx]);
		const vec2 normal = cross(vec3(tangent, 0.0), vec3(0.0, 0.0, 1.0)).xy;

		const vec2 aspect = vec2(U_scene_data.cur.viewport_size.y / U_scene_data.cur.viewport_size.x, 1.0);
		const vec2 stroke_coord = vec2(
			float(segment_edge_idx - segment_range.first_idx) / segment_range.num_segment_edges,
			float(B_stroke_vertex_kind.kind[stroke_vertex_idx] & E_StrokeVertexKind_RIGHT_MASK)
		);

		const float width = U_config.stroke_width * mix(U_config.stroke_width_factor_start, U_config.stroke_width_factor_end, stroke_coord.x);
		const vec2 frag_coord = midpoint / U_scene_data.cur.viewport_size * 2.0 - 1.0 + width * 0.001 * aspect * normal * (stroke_coord.y * 2.0 - 1.0);
		const int contour_pixel_idx = B_compacted_pixel_edge_to_contour_pixel.contour_pixel_idx[compacted_pixel_edge_idx];


		AO_frag_coord = frag_coord;
		AO_tangent = tangent;
		AO_normal = normal;
		AO_stroke_coord = stroke_coord;
		AO_segment_key = segment_key;


		gl_Position = vec4(frag_coord, 0.5, 1.0);
	}

#endif // STAGE_VERT


#ifdef STAGE_FRAG
	layout(location = 0) in smooth vec2 AI_frag_coord;
	layout(location = 1) in smooth vec2 AI_tangent;
	layout(location = 2) in smooth vec2 AI_normal;
	layout(location = 3) in smooth vec2 AI_stroke_coord;
	layout(location = 4) in flat uint AI_segment_key;
	layout(location = 0) out vec4 AO_color;

	void main() {
		AO_color = vec4(1.0);
	}

#endif // STAGE_FRAG
