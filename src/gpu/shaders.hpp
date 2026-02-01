#pragma once

#include <cstdint>
#include <map>

#define DEFINE_EMBEDDED_DATA_TYPE
#include "embed.c"

#define M_SHADER_STAGE_VERTEX                 (0)
#define M_SHADER_STAGE_FRAGMENT               (1)
#define M_SHADER_STAGE_TESSELATION_CONTROL    (2)
#define M_SHADER_STAGE_TESSELATION_EVALUATION (3)
#define M_SHADER_STAGE_COMPUTE                (4)
#define M_SHADER_STAGE_MAX                    (5)
#define M_SHADER_STAGE_VERTEX_BIT                 (1 << M_SHADER_STAGE_VERTEX)
#define M_SHADER_STAGE_FRAGMENT_BIT               (1 << M_SHADER_STAGE_FRAGMENT)
#define M_SHADER_STAGE_TESSELATION_CONTROL_BIT    (1 << M_SHADER_STAGE_TESSELATION_CONTROL)
#define M_SHADER_STAGE_TESSELATION_EVALUATION_BIT (1 << M_SHADER_STAGE_TESSELATION_EVALUATION)
#define M_SHADER_STAGE_COMPUTE_BIT                (1 << M_SHADER_STAGE_COMPUTE)

enum Shader : int32_t {
	SHADER_DUMMY = 0,
	SHADER_DUMMY_COMMANDER,
	SHADER_DUMMY_DEBUG,
	SHADER_REUSABLE_WG_ALLOCATION,
	SHADER_REUSABLE_ALLOCATION_COMMANDER,
	SHADER_REUSABLE_ALLOCATION_L0_UP,
	SHADER_REUSABLE_ALLOCATION_L1_UP,
	SHADER_REUSABLE_ALLOCATION_L0_DOWN,
	SHADER_CR_MP_GLOBAL_GEOMETRY_ALLOCATION,
	SHADER_CR_MP_FIRST_COMMANDER,
	SHADER_CR_MP_GLOBAL_GEOMETRY_SCATTER_EDGES,
	SHADER_CR_MP_GLOBAL_GEOMETRY_SCATTER_FACES,
	SHADER_CR_CED_FACE_ORIENTATION,
	SHADER_CR_CED_DETECTION,
	SHADER_CR_CED_MEM_ACQUIRE,
	SHADER_CR_CED_SCATTER,
	SHADER_CR_FG_FIRST_COMMANDER,
	SHADER_CR_FG_CLIPPING,
	SHADER_CR_FG_FRAG_COUNTS,
	SHADER_CR_FG_MEM_ACQUIRE,
	SHADER_CR_FG_SECOND_COMMANDER,
	SHADER_CR_FG_SCATTER,
	SHADER_CR_FG_RASTERIZE_BRESENHAM,
	SHADER_CR_FG_RASTERIZE_DDA,
	SHADER_CR_CPG_SOFT_DEPTH_TEST,
	SHADER_CR_CPG_PRE_ALLOC,
	SHADER_CR_CPG_MEM_ACQUIRE,
	SHADER_CR_CPG_SCATTER,
	SHADER_CR_CPG_SECOND_COMMANDER,
	SHADER_CR_CPG_HARD_DEPTH_TEST,
	SHADER_CR_CPG_DECODE,
	SHADER_CC_PEG_FIRST_COMMANDER,
	SHADER_CC_PEG_GENERATION,
	SHADER_CC_PEG_FRAGMENTED_ALLOC,
	SHADER_CC_PEG_SCATTER,
	SHADER_CC_PEG_SECOND_COMMANDER,
	SHADER_CC_LB_INIT,
	SHADER_CC_LB_WYLLIE,
	SHADER_CC_LB_SCATTER,
	SHADER_CC_LR_INIT,
	SHADER_CC_LR_WYLLIE,
	SHADER_CC_LR_SCATTER,
	SHADER_CC_D_HEAD_ALLOCATION_COMMANDER,
	SHADER_CC_D_HEAD_ALLOCATION_L0_UP,
	SHADER_CC_D_HEAD_ALLOCATION_L1_UP,
	SHADER_CC_D_HEAD_ALLOCATION_L0_DOWN,
	SHADER_CC_D_HEAD_SCATTER,
	SHADER_CC_D_COMPACTED_ALLOCATION,
	SHADER_CC_D_COMPACTED_SCATTER,
	SHADER_CC_D_FIRST_COMMANDER,
	SHADER_SE_MF_SMOOTHING,
	SHADER_SE_OF_CURVE_FITTING,
	SHADER_SE_IOT_TEST,
	SHADER_SE_IOT_SMOOTHING,
	SHADER_SE_S_SEGMENT_HEAD_ID,
	SHADER_SE_S_LOOP_LOCAL_SEGMENTATION,
	SHADER_SE_S_ALLOCATION,
	SHADER_SE_S_FIRST_COMMANDER,
	SHADER_SE_S_CLEAR,
	SHADER_SE_S_SCATTER_TOP,
	SHADER_SE_S_SCATTER_BOTTOM,
	SHADER_SE_SG_INIT,
	SHADER_SE_SG_ALLOCATION,
	SHADER_SE_SG_FIRST_COMMANDER,
	SHADER_SE_SG_SCATTER,
	SHADER_SR_A_SEGMENT_EDGE_ARC_LENGTH_INIT,
	SHADER_SR_A_SEGMENT_EDGE_ARC_LENGTH,
	SHADER_SR_A_SEGMENT_ARC_LENGTH,
	SHADER_SR_UPDATE_API,
	SHADER_SR_DEFAULT_SHADER,
	SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS,
	SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS,
	SHADER_MAX,
};


struct ShaderBuildInfo {
	char const *name;
	char const *src_file_path;
	int shader_stages;
	char const *glc_flags="";
};

extern EmbeddedData const *shader_to_embedded_data_map[Shader::SHADER_MAX][M_SHADER_STAGE_MAX];

extern std::map<Shader, ShaderBuildInfo> shader_to_shader_info_map;

#ifndef SHADERS_DECLARATIONS_ONLY

std::map<Shader, ShaderBuildInfo> shader_to_shader_info_map = {{
	{ SHADER_DUMMY,           ShaderBuildInfo{"dummy",           "./test/dummy.comp" ,          M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DUMMY_COMMANDER, ShaderBuildInfo{"dummy_commander", "./test/dummy_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DUMMY_DEBUG,     ShaderBuildInfo{"dummy_debug",     "./test/dummy_debug.comp",     M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_REUSABLE_WG_ALLOCATION, ShaderBuildInfo{"reusable__wg_allocation", "./reusable/wg_allocation.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_REUSABLE_ALLOCATION_COMMANDER, ShaderBuildInfo{"reusable__allocation_commander", "./reusable/allocation.comp", M_SHADER_STAGE_COMPUTE_BIT, "-DCOMMANDER"} },
	{ SHADER_REUSABLE_ALLOCATION_L0_UP,     ShaderBuildInfo{"reusable__allocation_l0_up",     "./reusable/allocation.comp", M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_0 -DUP_SWEEP"} },
	{ SHADER_REUSABLE_ALLOCATION_L1_UP,     ShaderBuildInfo{"reusable__allocation_l1_up",     "./reusable/allocation.comp", M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_1 -DUP_SWEEP"} },
	{ SHADER_REUSABLE_ALLOCATION_L0_DOWN,   ShaderBuildInfo{"reusable__allocation_l0_down",   "./reusable/allocation.comp", M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_0 -DDOWN_SWEEP"} },

	{ SHADER_CR_MP_GLOBAL_GEOMETRY_ALLOCATION,    ShaderBuildInfo{"cr__mp__global_geometry_allocation",    "./contour_rasterization/mesh_processing/global_geometry_allocation.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_MP_FIRST_COMMANDER,               ShaderBuildInfo{"cr__mp__first_commander",               "./contour_rasterization/mesh_processing/first_commander.comp",            M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_MP_GLOBAL_GEOMETRY_SCATTER_EDGES, ShaderBuildInfo{"cr__mp__global_geometry_scatter_edges", "./contour_rasterization/mesh_processing/global_geometry_scatter.comp",    M_SHADER_STAGE_COMPUTE_BIT, "-DPASS_GLOBAL_EDGES"} },
	{ SHADER_CR_MP_GLOBAL_GEOMETRY_SCATTER_FACES, ShaderBuildInfo{"cr__mp__global_geometry_scatter_faces", "./contour_rasterization/mesh_processing/global_geometry_scatter.comp",    M_SHADER_STAGE_COMPUTE_BIT, "-DPASS_GLOBAL_FACES"} },

	{ SHADER_CR_CED_FACE_ORIENTATION,   ShaderBuildInfo{"cr__ced__face_orientation",   "./contour_rasterization/contour_edge_detection/face_orientation.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CED_DETECTION,          ShaderBuildInfo{"cr__ced__detection",          "./contour_rasterization/contour_edge_detection/detection.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CED_MEM_ACQUIRE,        ShaderBuildInfo{"cr__ced__mem_acquire",        "./contour_rasterization/contour_edge_detection/mem_acquire.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CED_SCATTER,            ShaderBuildInfo{"cr__ced__scatter",            "./contour_rasterization/contour_edge_detection/scatter.comp",          M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CR_FG_FIRST_COMMANDER,     ShaderBuildInfo{"cr__fg__first_commander",     "./contour_rasterization/fragment_generation/first_commander.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_CLIPPING,            ShaderBuildInfo{"cr__fg__clipping",            "./contour_rasterization/fragment_generation/clipping.comp",         M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_FRAG_COUNTS,         ShaderBuildInfo{"cr__fg__frag_counts",         "./contour_rasterization/fragment_generation/frag_counts.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_MEM_ACQUIRE,         ShaderBuildInfo{"cr__fg__mem_acquire",         "./contour_rasterization/fragment_generation/mem_acquire.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_SECOND_COMMANDER,    ShaderBuildInfo{"cr__fg__second_commander",    "./contour_rasterization/fragment_generation/second_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_SCATTER,             ShaderBuildInfo{"cr__fg__scatter",             "./contour_rasterization/fragment_generation/scatter.comp",          M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_RASTERIZE_BRESENHAM, ShaderBuildInfo{"cr__fg__rasterize_bresenham", "./contour_rasterization/fragment_generation/rasterize.comp",        M_SHADER_STAGE_COMPUTE_BIT, "-DRASTER_METHOD_BRESENHAM"} },
	{ SHADER_CR_FG_RASTERIZE_DDA,       ShaderBuildInfo{"cr__fg__rasterize_dda",       "./contour_rasterization/fragment_generation/rasterize.comp",        M_SHADER_STAGE_COMPUTE_BIT, "-DRASTER_METHOD_DDA"} },

	{ SHADER_CR_CPG_SOFT_DEPTH_TEST,  ShaderBuildInfo{"cr_cpg_soft_depth_test",  "./contour_rasterization/contour_pixel_generation/soft_depth_test.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_PRE_ALLOC,        ShaderBuildInfo{"cr_cpg_pre_alloc",        "./contour_rasterization/contour_pixel_generation/pre_alloc.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_MEM_ACQUIRE,      ShaderBuildInfo{"cr_cpg_mem_acquire",      "./contour_rasterization/contour_pixel_generation/mem_acquire.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_SCATTER,          ShaderBuildInfo{"cr_cpg_scatter",          "./contour_rasterization/contour_pixel_generation/scatter.comp",          M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_SECOND_COMMANDER, ShaderBuildInfo{"cr_cpg_second_commander", "./contour_rasterization/contour_pixel_generation/second_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_HARD_DEPTH_TEST,  ShaderBuildInfo{"cr_cpg_hard_depth_test",  "./contour_rasterization/contour_pixel_generation/hard_depth_test.glsl",  M_SHADER_STAGE_VERTEX_BIT | M_SHADER_STAGE_FRAGMENT_BIT} },
	{ SHADER_CR_CPG_DECODE,           ShaderBuildInfo{"cr_cpg_decode",           "./contour_rasterization/contour_pixel_generation/decode.comp",           M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_PEG_FIRST_COMMANDER,  ShaderBuildInfo{"cc_peg_first_commander",  "./contour_chaining/pixel_edge_generation/first_commander.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_PEG_GENERATION,       ShaderBuildInfo{"cc_peg_generation",       "./contour_chaining/pixel_edge_generation/generation.comp",       M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_PEG_FRAGMENTED_ALLOC, ShaderBuildInfo{"cc_peg_fragmented_alloc", "./contour_chaining/pixel_edge_generation/fragmented_alloc.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_PEG_SCATTER,          ShaderBuildInfo{"cc_peg_scatter",          "./contour_chaining/pixel_edge_generation/scatter.comp",          M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_PEG_SECOND_COMMANDER, ShaderBuildInfo{"cc_peg_second_commander", "./contour_chaining/pixel_edge_generation/second_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_LB_INIT,    ShaderBuildInfo{"cc_lb_init",    "./contour_chaining/loop_breaking/init.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LB_WYLLIE,  ShaderBuildInfo{"cc_lb_wyllie",  "./contour_chaining/loop_breaking/wyllie.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LB_SCATTER, ShaderBuildInfo{"cc_lb_scatter", "./contour_chaining/loop_breaking/scatter.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_LR_INIT,    ShaderBuildInfo{"cc_lr_init",    "./contour_chaining/list_ranking/init.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LR_WYLLIE,  ShaderBuildInfo{"cc_lr_wyllie",  "./contour_chaining/list_ranking/wyllie.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LR_SCATTER, ShaderBuildInfo{"cc_lr_scatter", "./contour_chaining/list_ranking/scatter.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_D_HEAD_ALLOCATION_COMMANDER, ShaderBuildInfo{"cc_d_head_allocation_commander", "./contour_chaining/defragmentation/head_allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT, "-DCOMMANDER"} },
	{ SHADER_CC_D_HEAD_ALLOCATION_L0_UP,     ShaderBuildInfo{"cc_d_head_allocation_l0_up",     "./contour_chaining/defragmentation/head_allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_0 -DUP_SWEEP"} },
	{ SHADER_CC_D_HEAD_ALLOCATION_L1_UP,     ShaderBuildInfo{"cc_d_head_allocation_l1_up",     "./contour_chaining/defragmentation/head_allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_1 -DUP_SWEEP"} },
	{ SHADER_CC_D_HEAD_ALLOCATION_L0_DOWN,   ShaderBuildInfo{"cc_d_head_allocation_l0_down",   "./contour_chaining/defragmentation/head_allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_0 -DDOWN_SWEEP"} },
	{ SHADER_CC_D_HEAD_SCATTER,              ShaderBuildInfo{"cc_d_head_scatter",              "./contour_chaining/defragmentation/head_scatter.comp",         M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_COMPACTED_ALLOCATION,      ShaderBuildInfo{"cc_d_compacted_allocation",      "./contour_chaining/defragmentation/compacted_allocation.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_COMPACTED_SCATTER,         ShaderBuildInfo{"cc_d_compacted_scatter",         "./contour_chaining/defragmentation/compacted_scatter.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_FIRST_COMMANDER,           ShaderBuildInfo{"cc_d_first_commander",           "./contour_chaining/defragmentation/first_commander.comp",      M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SE_MF_SMOOTHING, ShaderBuildInfo{"se_mf_smoothing", "./stroke_extraction/midpoints_filtering/smoothing.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SE_OF_CURVE_FITTING, ShaderBuildInfo{"se_of_curve_fitting", "./stroke_extraction/orientations_filtering/curve_fitting.comp",      M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SE_IOT_TEST,      ShaderBuildInfo{"se_iot_test",      "./stroke_extraction/inside_outside_test/test.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_IOT_SMOOTHING, ShaderBuildInfo{"se_iot_smoothing", "./stroke_extraction/inside_outside_test/smoothing.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SE_S_SEGMENT_HEAD_ID,         ShaderBuildInfo{"se_s_segment_head_id",         "./stroke_extraction/segmentation/segment_head_id.comp",         M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_S_LOOP_LOCAL_SEGMENTATION, ShaderBuildInfo{"se_s_loop_local_segmentation", "./stroke_extraction/segmentation/loop_local_segmentation.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_S_ALLOCATION,              ShaderBuildInfo{"se_s_allocation",              "./stroke_extraction/segmentation/allocation.comp",              M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_S_FIRST_COMMANDER,         ShaderBuildInfo{"se_s_first_commander",         "./stroke_extraction/segmentation/first_commander.comp",         M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_S_CLEAR,                   ShaderBuildInfo{"se_s_clear",                   "./stroke_extraction/segmentation/clear.comp",                   M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_S_SCATTER_TOP,             ShaderBuildInfo{"se_s_scatter_top",             "./stroke_extraction/segmentation/scatter_top.comp",             M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_S_SCATTER_BOTTOM,          ShaderBuildInfo{"se_s_scatter_bottom",          "./stroke_extraction/segmentation/scatter_bottom.comp",          M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SE_SG_INIT,            ShaderBuildInfo{"se_sg_init",            "./stroke_extraction/stroke_generation/init.comp",            M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_SG_ALLOCATION,      ShaderBuildInfo{"se_sg_allocation",      "./stroke_extraction/stroke_generation/allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_SG_FIRST_COMMANDER, ShaderBuildInfo{"se_sg_first_commander", "./stroke_extraction/stroke_generation/first_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SE_SG_SCATTER,         ShaderBuildInfo{"se_sg_scatter",         "./stroke_extraction/stroke_generation/scatter.comp",         M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SR_A_SEGMENT_EDGE_ARC_LENGTH_INIT, ShaderBuildInfo{"sr__a__segment_edge_arc_length_init", "./stroke_rendering/attribs/segment_edge_arc_length_init.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SR_A_SEGMENT_EDGE_ARC_LENGTH,      ShaderBuildInfo{"sr__a__segment_edge_arc_length",      "./stroke_rendering/attribs/segment_edge_arc_length.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SR_A_SEGMENT_ARC_LENGTH,           ShaderBuildInfo{"sr__a__segment_arc_length",           "./stroke_rendering/attribs/segment_arc_length.comp",           M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SR_UPDATE_API,     ShaderBuildInfo{"sr__update_api",     "./stroke_rendering/update_api.comp",     M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SR_DEFAULT_SHADER, ShaderBuildInfo{"sr__default_shader", "./stroke_rendering/default_shader.glsl", M_SHADER_STAGE_VERTEX_BIT | M_SHADER_STAGE_FRAGMENT_BIT} },

	{ SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS,     ShaderBuildInfo{"debug__display_contour_fragments",     "./debug/display_contour_fragments.comp",     M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS,        ShaderBuildInfo{"debug__display_contour_pixels",        "./debug/display_contour_pixels.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
}};

#endif // !SHADERS_BUILD_DECLARATIONS_ONLY
