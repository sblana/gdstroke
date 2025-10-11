#pragma once

#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/classes/rd_uniform.hpp>

using namespace godot;

class GdstrokeShaderInterface : public RefCounted {
	GDCLASS(GdstrokeShaderInterface, RefCounted)

public:
	struct InterfaceSet {
		TypedArray<Ref<RDUniform>> bindings;
		TypedArray<RID> resources;

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) = 0;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) = 0;
		virtual void make_bindings() = 0;
		virtual uint32_t get_slot() const = 0;

		inline RID get_uniform_set_rid(RID const &p_shader) const {
			return UniformSetCacheRD::get_cache(p_shader, get_slot(), bindings);
		}

		inline void bind_to_compute_list(RenderingDevice *p_rd, int64_t p_compute_list, RID const &p_shader) const {
			p_rd->compute_list_bind_uniform_set(p_compute_list, get_uniform_set_rid(p_shader), get_slot());
		}
	};

	struct SceneInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_SCENE_DATA_UNIFORM = 0,
			BINDING_CONFIG_UNIFORM,
			BINDING_MAX,
		};

		struct ConfigData {
			float depth_bias = 0.0;
			uint32_t use_soft_depth_test_modification = false;
		};
		
		ConfigData config_data = {};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 0; }
	};

	struct CommandInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_DISPATCH_INDIRECT_COMMANDS_BUFFER = 0,
			BINDING_MAX,
		};

		enum DispatchIndirectCommands : uint32_t {
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES = 0,
			DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_CONTOUR_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS,
			DISPATCH_INDIRECT_COMMANDS_MAX,
		};

		RID get_dispatch_indirect_commands_buffer() const;
		void dispatch_indirect(RenderingDevice *p_rd, int64_t p_compute_list, DispatchIndirectCommands cmd) const;

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 1; }
	};

	struct MeshInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_MESH_DESC_BUFFER = 0,
			BINDING_VERTEX_BUFFER,
			BINDING_EDGE_TO_VERTEX_BUFFER,
			BINDING_EDGE_TO_FACE_BUFFER,
			BINDING_EDGE_IS_CONCAVE_BUFFER,
			BINDING_EDGE_IS_CONTOUR_BUFFER,
			BINDING_EDGE_TO_CONTOUR_EDGE_BUFFER,
			BINDING_FACE_TO_VERTEX_BUFFER,
			BINDING_FACE_NORMAL_BUFFER,
			BINDING_FACE_BACKFACING_BUFFER,
			BINDING_MAX,
		};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 2; }
	};

	struct ContourInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_CONTOUR_DESC_BUFFER = 0,
			BINDING_CONTOUR_EDGE_TO_EDGE_BUFFER,
			BINDING_CONTOUR_EDGE_TO_CONTOUR_FRAGMENT_BUFFER,
			BINDING_CONTOUR_FRAGMENT_PIXEL_COORD_BUFFER,
			BINDING_CONTOUR_FRAGMENT_ORIENTATION_BUFFER,
			BINDING_CONTOUR_FRAGMENT_NORMAL_DEPTH_BUFFER,
			BINDING_CONTOUR_FRAGMENT_PSEUDO_VISIBLE_BUFFER,
			BINDING_SCREEN_DEPTH_TEXTURE,
			BINDING_MAX,
		};

		static constexpr uint32_t max_num_contour_fragments = 1u << 20u;

		RID nearest_sampler;

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 3; }
	};

	struct DebugInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_CONTOUR_SCREEN_COLOR_IMAGE = 0,
			BINDING_MAX,
		};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 15; }
	};

private:

protected:
	static void _bind_methods();

public:
	GdstrokeShaderInterface() = default;
	~GdstrokeShaderInterface() = default;
};
