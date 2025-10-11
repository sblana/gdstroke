#pragma once

#include <godot_cpp/classes/rd_shader_spirv.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>

#define DEFINE_EMBEDDED_DATA_TYPE
#include "gpu/embed.c"

using namespace godot;

static inline Ref<RDUniform> new_uniform(int32_t p_binding_slot, RenderingDevice::UniformType p_uniform_type, RID const &p_first_id, RID const &p_secnd_id = RID()) {
	Ref<RDUniform> uniform = Ref(memnew(RDUniform));
	uniform->set_binding(p_binding_slot);
	uniform->set_uniform_type(p_uniform_type);
	ERR_FAIL_COND_V(!p_first_id.is_valid(), Ref<RDUniform>());
	uniform->add_id(p_first_id);
	if (p_secnd_id.is_valid())
		uniform->add_id(p_secnd_id);
	return uniform;
}


inline RID create_comp_shader_from_embedded_spirv(RenderingDevice *p_rd, EmbeddedData const &p_embedded_spirv, String const &p_name) {
	PackedByteArray spirv_data = PackedByteArray();
	for (uint32_t i = 0; i < p_embedded_spirv.length; ++i) {
		spirv_data.append(p_embedded_spirv.data[i]);
	}
	Ref<RDShaderSPIRV> spirv = Ref(memnew(RDShaderSPIRV));
	spirv->set_stage_bytecode(RenderingDevice::ShaderStage::SHADER_STAGE_COMPUTE, spirv_data);
	return p_rd->shader_create_from_spirv(spirv, p_name);
}

constexpr  int64_t idiv_floor( int64_t n,  int64_t d) { return n / d -  int64_t((n % d) != 0 && (n^d) < 0); }
constexpr  int64_t idiv_ceil ( int64_t n,  int64_t d) { return n / d +  int64_t((n % d) != 0 && (n^d) > 0); }
constexpr uint64_t udiv_floor(uint64_t n, uint64_t d) { return n / d; } // just for the sake of consistency
constexpr uint64_t udiv_ceil (uint64_t n, uint64_t d) { return n / d + uint64_t((n % d) != 0); }


struct DispatchIndirectCommand {
	uint32_t workgroups_x;
	uint32_t workgroups_y;
	uint32_t workgroups_z;
	uint32_t pad;
};
