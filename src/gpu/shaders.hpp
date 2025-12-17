#pragma once

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <map>
#include <unordered_map>
#include <string>
#include <format>
#include <algorithm>
#include <thread>

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
	SHADER_CR_CED_FACE_ORIENTATION,
	SHADER_CR_CED_DETECTION,
	SHADER_CR_CED_ALLOCATION_L0_UP,
	SHADER_CR_CED_ALLOCATION_L1_UP,
	SHADER_CR_CED_ALLOCATION_L0_DOWN,
	SHADER_CR_CED_SCATTER,
	SHADER_CR_FG_FIRST_COMMANDER,
	SHADER_CR_FG_CLIPPING,
	SHADER_CR_FG_FRAG_COUNTS,
	SHADER_CR_FG_ALLOCATION,
	SHADER_CR_FG_SCATTER,
	SHADER_CR_CPG_FIRST_COMMANDER,
	SHADER_CR_CPG_SOFT_DEPTH_TEST,
	SHADER_CR_CPG_PRE_ALLOC,
	SHADER_CR_CPG_ALLOCATION,
	SHADER_CR_CPG_SCATTER,
	SHADER_CR_CPG_SECOND_COMMANDER,
	SHADER_CR_CPG_HARD_DEPTH_TEST,
	SHADER_CR_CPG_DECODE,
	SHADER_CC_PEG_FIRST_COMMANDER,
	SHADER_CC_PEG_GENERATION,
	SHADER_CC_LB_INIT,
	SHADER_CC_LB_WYLLIE,
	SHADER_CC_LB_SCATTER,
	SHADER_CC_LR_INIT,
	SHADER_CC_LR_WYLLIE,
	SHADER_CC_LR_SCATTER,
	SHADER_CC_D_INIT,
	SHADER_CC_D_HEAD_ALLOCATION,
	SHADER_CC_D_HEAD_SCATTER,
	SHADER_CC_D_COMPACTED_ALLOCATION,
	SHADER_CC_D_COMPACTED_SCATTER,
	SHADER_CC_D_FIRST_COMMANDER,
	SHADER_SE_MF_INIT,
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
	SHADER_SR_UPDATE_API,
	SHADER_SR_DEFAULT_SHADER,
	SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS,
	SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS,
	SHADER_DEBUG_DISPLAY_SPARSE_PIXEL_EDGES,
	SHADER_DEBUG_DISPLAY_COMPACTED_PIXEL_EDGES,
	SHADER_DEBUG_DISPLAY_SEGMENT_EDGES,
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

	{ SHADER_CR_CED_FACE_ORIENTATION,   ShaderBuildInfo{"cr__ced__face_orientation",   "./contour_rasterization/contour_edge_detection/face_orientation.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CED_DETECTION,          ShaderBuildInfo{"cr__ced__detection",          "./contour_rasterization/contour_edge_detection/detection.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CED_ALLOCATION_L0_UP,   ShaderBuildInfo{"cr__ced__allocation_l0_up",   "./contour_rasterization/contour_edge_detection/allocation.comp",       M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_0 -DUP_SWEEP"} },
	{ SHADER_CR_CED_ALLOCATION_L1_UP,   ShaderBuildInfo{"cr__ced__allocation_l1_up",   "./contour_rasterization/contour_edge_detection/allocation.comp",       M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_1 -DUP_SWEEP"} },
	{ SHADER_CR_CED_ALLOCATION_L0_DOWN, ShaderBuildInfo{"cr__ced__allocation_l0_down", "./contour_rasterization/contour_edge_detection/allocation.comp",       M_SHADER_STAGE_COMPUTE_BIT, "-DLEVEL_0 -DDOWN_SWEEP"} },
	{ SHADER_CR_CED_SCATTER,            ShaderBuildInfo{"cr__ced__scatter",            "./contour_rasterization/contour_edge_detection/scatter.comp",          M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CR_FG_FIRST_COMMANDER, ShaderBuildInfo{"cr__fg__first_commander", "./contour_rasterization/fragment_generation/first_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_CLIPPING,        ShaderBuildInfo{"cr__fg__clipping",        "./contour_rasterization/fragment_generation/clipping.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_FRAG_COUNTS,     ShaderBuildInfo{"cr__fg__frag_counts",     "./contour_rasterization/fragment_generation/frag_counts.comp",     M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_ALLOCATION,      ShaderBuildInfo{"cr__fg__allocation",      "./contour_rasterization/fragment_generation/allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_FG_SCATTER,         ShaderBuildInfo{"cr__fg__scatter",         "./contour_rasterization/fragment_generation/scatter.comp",         M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CR_CPG_FIRST_COMMANDER,  ShaderBuildInfo{"cr_cpg_first_commander",  "./contour_rasterization/contour_pixel_generation/first_commander.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_SOFT_DEPTH_TEST,  ShaderBuildInfo{"cr_cpg_soft_depth_test",  "./contour_rasterization/contour_pixel_generation/soft_depth_test.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_PRE_ALLOC,        ShaderBuildInfo{"cr_cpg_pre_alloc",        "./contour_rasterization/contour_pixel_generation/pre_alloc.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_ALLOCATION,       ShaderBuildInfo{"cr_cpg_allocation",       "./contour_rasterization/contour_pixel_generation/allocation.comp",       M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_SCATTER,          ShaderBuildInfo{"cr_cpg_scatter",          "./contour_rasterization/contour_pixel_generation/scatter.comp",          M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_SECOND_COMMANDER, ShaderBuildInfo{"cr_cpg_second_commander", "./contour_rasterization/contour_pixel_generation/second_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CR_CPG_HARD_DEPTH_TEST,  ShaderBuildInfo{"cr_cpg_hard_depth_test",  "./contour_rasterization/contour_pixel_generation/hard_depth_test.glsl",  M_SHADER_STAGE_VERTEX_BIT | M_SHADER_STAGE_FRAGMENT_BIT} },
	{ SHADER_CR_CPG_DECODE,           ShaderBuildInfo{"cr_cpg_decode",           "./contour_rasterization/contour_pixel_generation/decode.comp",           M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_PEG_FIRST_COMMANDER, ShaderBuildInfo{"cc_peg_first_commander", "./contour_chaining/pixel_edge_generation/first_commander.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_PEG_GENERATION,      ShaderBuildInfo{"cc_peg_generation",      "./contour_chaining/pixel_edge_generation/generation.comp",      M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_LB_INIT,    ShaderBuildInfo{"cc_lb_init",    "./contour_chaining/loop_breaking/init.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LB_WYLLIE,  ShaderBuildInfo{"cc_lb_wyllie",  "./contour_chaining/loop_breaking/wyllie.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LB_SCATTER, ShaderBuildInfo{"cc_lb_scatter", "./contour_chaining/loop_breaking/scatter.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_LR_INIT,    ShaderBuildInfo{"cc_lr_init",    "./contour_chaining/list_ranking/init.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LR_WYLLIE,  ShaderBuildInfo{"cc_lr_wyllie",  "./contour_chaining/list_ranking/wyllie.comp",  M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_LR_SCATTER, ShaderBuildInfo{"cc_lr_scatter", "./contour_chaining/list_ranking/scatter.comp", M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_CC_D_INIT,                 ShaderBuildInfo{"cc_d_init",                 "./contour_chaining/defragmentation/init.comp",                 M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_HEAD_ALLOCATION,      ShaderBuildInfo{"cc_d_head_allocation",      "./contour_chaining/defragmentation/head_allocation.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_HEAD_SCATTER,         ShaderBuildInfo{"cc_d_head_scatter",         "./contour_chaining/defragmentation/head_scatter.comp",         M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_COMPACTED_ALLOCATION, ShaderBuildInfo{"cc_d_compacted_allocation", "./contour_chaining/defragmentation/compacted_allocation.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_COMPACTED_SCATTER,    ShaderBuildInfo{"cc_d_compacted_scatter",    "./contour_chaining/defragmentation/compacted_scatter.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_CC_D_FIRST_COMMANDER,      ShaderBuildInfo{"cc_d_first_commander",      "./contour_chaining/defragmentation/first_commander.comp",      M_SHADER_STAGE_COMPUTE_BIT} },

	{ SHADER_SE_MF_INIT,      ShaderBuildInfo{"se_mf_init",      "./stroke_extraction/midpoints_filtering/init.comp",      M_SHADER_STAGE_COMPUTE_BIT} },
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

	{ SHADER_SR_UPDATE_API,     ShaderBuildInfo{"sr__update_api",     "./stroke_rendering/update_api.comp",     M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_SR_DEFAULT_SHADER, ShaderBuildInfo{"sr__default_shader", "./stroke_rendering/default_shader.glsl", M_SHADER_STAGE_VERTEX_BIT | M_SHADER_STAGE_FRAGMENT_BIT} },

	{ SHADER_DEBUG_DISPLAY_CONTOUR_FRAGMENTS,     ShaderBuildInfo{"debug__display_contour_fragments",     "./debug/display_contour_fragments.comp",     M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DEBUG_DISPLAY_CONTOUR_PIXELS,        ShaderBuildInfo{"debug__display_contour_pixels",        "./debug/display_contour_pixels.comp",        M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DEBUG_DISPLAY_SPARSE_PIXEL_EDGES,    ShaderBuildInfo{"debug__display_sparse_pixel_edges",    "./debug/display_sparse_pixel_edges.comp",    M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DEBUG_DISPLAY_COMPACTED_PIXEL_EDGES, ShaderBuildInfo{"debug__display_compacted_pixel_edges", "./debug/display_compacted_pixel_edges.comp", M_SHADER_STAGE_COMPUTE_BIT} },
	{ SHADER_DEBUG_DISPLAY_SEGMENT_EDGES,         ShaderBuildInfo{"debug__display_segment_edges",         "./debug/display_segment_edges.comp",         M_SHADER_STAGE_COMPUTE_BIT} },
}};

#endif // !SHADERS_BUILD_DECLARATIONS_ONLY
