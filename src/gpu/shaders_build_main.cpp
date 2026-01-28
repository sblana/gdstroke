#include "embed.c"
#include "shaders.hpp"

#include <unordered_map>
#include <string>
#include <format>
#include <thread>

std::unordered_map<int, char const *> shader_stage_bit_to_stage_affix_lower{{
	{ M_SHADER_STAGE_VERTEX_BIT,                 "vert" },
	{ M_SHADER_STAGE_FRAGMENT_BIT,               "frag" },
	{ M_SHADER_STAGE_TESSELATION_CONTROL_BIT,    "tesc" },
	{ M_SHADER_STAGE_TESSELATION_EVALUATION_BIT, "tese" },
	{ M_SHADER_STAGE_COMPUTE_BIT,                "comp" },
}};

std::unordered_map<int, char const *> shader_stage_bit_to_stage_affix_upper{{
	{ M_SHADER_STAGE_VERTEX_BIT,                 "VERT" },
	{ M_SHADER_STAGE_FRAGMENT_BIT,               "FRAG" },
	{ M_SHADER_STAGE_TESSELATION_CONTROL_BIT,    "TESC" },
	{ M_SHADER_STAGE_TESSELATION_EVALUATION_BIT, "TESE" },
	{ M_SHADER_STAGE_COMPUTE_BIT,                "COMP" },
}};

std::unordered_map<int, char const *> shader_stage_bit_to_glc_stage_flags{{
	{ M_SHADER_STAGE_VERTEX_BIT,                 "-DSTAGE_VERT -S vert" },
	{ M_SHADER_STAGE_FRAGMENT_BIT,               "-DSTAGE_FRAG -S frag" },
	{ M_SHADER_STAGE_TESSELATION_CONTROL_BIT,    "-DSTAGE_TESC -S tesc" },
	{ M_SHADER_STAGE_TESSELATION_EVALUATION_BIT, "-DSTAGE_TESE -S tese" },
	{ M_SHADER_STAGE_COMPUTE_BIT,                "-DSTAGE_COMP -S comp" },
}};


void build_shader(ShaderBuildInfo shader_build_info, int godot_version_minor) {
	for (int i = 0; i < M_SHADER_STAGE_MAX; ++i) {
		if ((shader_build_info.shader_stages & (1 << i)) == 0) {
			continue;
		}

		std::string glc_stage_flags = shader_stage_bit_to_glc_stage_flags.at(1 << i);
		std::string stage_affix = shader_stage_bit_to_stage_affix_lower.at(1 << i);

		constexpr std::string_view glc_fmt = "glslang -DGODOT_VERSION_MINOR={} -g -Isrc/gpu/ --target-env vulkan1.3 --spirv-val {} {} src/gpu/{} -o temp/{}.{}.spv";
		std::system(std::format(glc_fmt, godot_version_minor, glc_stage_flags, shader_build_info.glc_flags, shader_build_info.src_file_path, shader_build_info.name, stage_affix).c_str());

		std::string stage_affix_upper = shader_stage_bit_to_stage_affix_upper.at(1 << i);

		embed(
			std::format("temp/{}.{}.spv", shader_build_info.name, stage_affix).c_str(),
			std::format("src/gen/{}.{}.spv.h", shader_build_info.name, stage_affix).c_str(),
			std::format("SHADER_{}_SPV_{}", stage_affix_upper, shader_build_info.name).c_str()
		);
	}
}


void generate_header() {
	FILE *fp = fopen("src/gen/shaders.cpp", "wb");
	assert(fp);
	std::fprintf(fp, "#include \"../gpu/shaders.hpp\"\n");
	std::fprintf(fp, "\n");
	for (auto kvp : shader_to_shader_info_map) {
		for (int i = 0; i < M_SHADER_STAGE_MAX; ++i) {
			if ((kvp.second.shader_stages & (1 << i)) == 0) {
				continue;
			}
			std::fprintf(fp, "#include \"%s\"\n", std::format("../gen/{}.{}.spv.h", kvp.second.name, shader_stage_bit_to_stage_affix_lower.at(1 << i)).c_str());
		}
	}
	std::fprintf(fp, "\n");
	std::fprintf(fp, "\n");
	std::fprintf(fp, "EmbeddedData const *shader_to_embedded_data_map[][%d] = {\n", M_SHADER_STAGE_MAX);
	for (auto kvp : shader_to_shader_info_map) {
		std::fprintf(fp, "\t{ ");
		for (int i = 0; i < M_SHADER_STAGE_MAX; ++i) {
			if ((kvp.second.shader_stages & (1 << i)) == 0) {
				std::fprintf(fp, "nullptr, ");
				continue;
			}
			std::fprintf(fp, "%s, ", std::format("&SHADER_{}_SPV_{}", shader_stage_bit_to_stage_affix_upper.at(1 << i), kvp.second.name).c_str());
		}
		std::fprintf(fp, "},\n");
	}
	std::fprintf(fp, "};\n");
}


int main(int argc, char **argv) {
	using namespace std::chrono_literals;
	{
		std::map<Shader, std::thread> shader_to_thread;
		for (auto kvp : shader_to_shader_info_map) {
			shader_to_thread.insert({ kvp.first, std::thread(build_shader, kvp.second, 5) });
		}
		for (auto kvp : shader_to_shader_info_map) {
			shader_to_thread.at(kvp.first).join();
		}
	}
	generate_header();
	return 0;
}
