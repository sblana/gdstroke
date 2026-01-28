#ifndef IMPL_SUBGROUP_GLSL
#define IMPL_SUBGROUP_GLSL

#extension GL_ARB_shading_language_include : enable
#include "common/ext/subgroup.glsli"

#include "common/ext/explicit_arithmetic_types.glsli"
#extension GL_EXT_shader_subgroup_extended_types_int64 : enable


#define M_DEFINITION_GENTYPE(					\
	MP_function_def_macro,						\
	MP_scalar_type_,							\
	MP_vec_type_								\
) \
	MP_function_def_macro(MP_scalar_type_)		\
	MP_function_def_macro(MP_vec_type_##2)		\
	MP_function_def_macro(MP_vec_type_##3)		\
	MP_function_def_macro(MP_vec_type_##4)


bool subgroupElectLast() {
	return gl_SubgroupInvocationID == subgroupBallotFindMSB(subgroupBallot(true));
}


#define M_DEFINITION_subgroupBroadcastLast(	\
	MP_type_								\
) \
	MP_type_ subgroupBroadcastLast(in const MP_type_ i_value) {							\
		return subgroupBroadcast(i_value, subgroupBallotFindMSB(subgroupBallot(true)));	\
	}																					\

M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, int, ivec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, uint, uvec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, bool, bvec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, float, vec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, double, dvec)

M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, int64_t, i64vec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupBroadcastLast, uint64_t, u64vec)


SubgroupMask subgroupSegmentedScan_LEInvocationHeadMask(in const bool i_head) {
	const SubgroupMask le_invocation_head_mask = gl_SubgroupLeMask & subgroupBallot(i_head);
	return le_invocation_head_mask;
}


#define _M_IMPLEMENTATION_subgroupSegmentedInclusiveScan(	\
	MP_type_												\
) \
{																										\
	const SubgroupMask le_invocation_head_mask = subgroupSegmentedScan_LEInvocationHeadMask(i_head);	\
	const bool invocation_or_head_flag = any(bvec4(le_invocation_head_mask));							\
	const uint distance_to_segment_head = (invocation_or_head_flag)										\
		? (gl_SubgroupInvocationID - subgroupBallotFindMSB(le_invocation_head_mask))					\
		: (gl_SubgroupInvocationID);																	\
	MP_type_ prev_value;																				\
	MP_type_ scan_value = i_value;																		\
																										\
	for (uint i = 1; i < gl_SubgroupSize; i *= 2) {														\
		prev_value = subgroupShuffleUp(scan_value, i);													\
		scan_value = (i <= distance_to_segment_head) ? (prev_value + scan_value) : (scan_value);		\
	}																									\
	o_invocation_or_head_flag = invocation_or_head_flag;												\
	return scan_value;																					\
}

#define _M_IMPLEMENTATION_subgroupSegmentedExclusiveScan(	\
	MP_type_												\
) \
{																																\
	const MP_type_ invocation_inclusive_scan_value = subgroupSegmentedInclusiveAdd(i_value, i_head, o_invocation_or_head_flag);	\
	o_subgroup_reduction = subgroupBroadcastLast(invocation_inclusive_scan_value);												\
																																\
	const MP_type_ prev_value = subgroupShuffleUp(invocation_inclusive_scan_value, 1);											\
	return (gl_SubgroupInvocationID > 0 && !i_head) ? (prev_value) : (MP_type_(0));												\
}

#define M_DEFINITION_subgroupSegmentedScan(	\
	MP_type_								\
) \
	MP_type_ subgroupSegmentedInclusiveAdd(in const MP_type_ i_value, in const bool i_head, out bool o_invocation_or_head_flag) {										\
		_M_IMPLEMENTATION_subgroupSegmentedInclusiveScan(MP_type_)																										\
	}																																									\
	MP_type_ subgroupSegmentedExclusiveAdd(in const MP_type_ i_value, in const bool i_head, out bool o_invocation_or_head_flag, out MP_type_ o_subgroup_reduction) {	\
		_M_IMPLEMENTATION_subgroupSegmentedExclusiveScan(MP_type_)																										\
	}

M_DEFINITION_GENTYPE(M_DEFINITION_subgroupSegmentedScan, int, ivec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupSegmentedScan, uint, uvec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupSegmentedScan, float, vec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupSegmentedScan, double, dvec)

M_DEFINITION_GENTYPE(M_DEFINITION_subgroupSegmentedScan, int64_t, i64vec)
M_DEFINITION_GENTYPE(M_DEFINITION_subgroupSegmentedScan, uint64_t, u64vec)


#endif // !IMPL_SUBGROUP_GLSL
