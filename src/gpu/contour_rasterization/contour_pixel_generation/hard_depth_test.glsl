#version 460
#extension GL_ARB_shading_language_include : require

#include "common/buffers.glsli"


#ifdef STAGE_VERT
	layout(location = 0) out flat uint AO_contour_fragment_idx;
	layout(location = 1) out flat uint AO_is_pseudo_visible;

	void main() {
		gl_PointSize = 1.0;
		AO_contour_fragment_idx = gl_InstanceIndex;
		AO_is_pseudo_visible = uint(B_contour_fragment_attribs.data[AO_contour_fragment_idx].is_pseudo_visible);

		ivec2 pixel_coord = B_contour_fragment_attribs.data[AO_contour_fragment_idx].pixel_coord;
		vec2 frag_coord = (pixel_coord + 0.5) / U_scene_data.cur.viewport_size * 2.0 - 1.0;
		float depth = B_contour_fragment_attribs.data[AO_contour_fragment_idx].normal_depth.w;
		gl_Position = vec4(frag_coord, depth, 1.0);
	}

#endif // STAGE_VERT


#ifdef STAGE_FRAG
	layout(location = 0) in flat uint AI_contour_fragment_idx;
	layout(location = 1) in flat uint AI_is_pseudo_visible;
	layout(location = 0) out uint     AO_contour_fragment_idx;

	void main() {
		if (AI_is_pseudo_visible > 0) {
			AO_contour_fragment_idx = AI_contour_fragment_idx + 1u;
		}
		else {
			discard;
		}
	}

#endif // STAGE_FRAG
