#pragma once

#include <godot_cpp/classes/render_data.hpp>
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
		inline virtual TypedArray<Ref<RDUniform>> get_draw_bindings() const { return bindings; }

		RID get_uniform_set_rid(RID const &p_shader) const;
		RID get_draw_uniform_set_rid(RID const &p_shader) const;

		void bind_to_compute_list(RenderingDevice *p_rd, int64_t p_compute_list, RID const &p_shader) const;
		void bind_to_draw_list(RenderingDevice *p_rd, int64_t p_draw_list, RID const &p_shader) const;
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
			BINDING_DRAW_INDIRECT_COMMANDS_BUFFER,
			BINDING_MAX,
		};

		enum DispatchIndirectCommands : uint32_t {
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES = 0,
			DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_CONTOUR_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_PIXELS,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_8_SPARSE_PIXEL_EDGES,
			DISPATCH_INDIRECT_COMMANDS_MAX,
		};

		enum DrawIndirectCommands : uint32_t {
			DRAW_INDIRECT_COMMANDS_HARD_DEPTH_TEST = 0,
			DRAW_INDIRECT_COMMANDS_MAX,
		};

		RID get_dispatch_indirect_commands_buffer() const;
		RID get_draw_indirect_commands_buffer() const;
		void dispatch_indirect(RenderingDevice *p_rd, int64_t p_compute_list, DispatchIndirectCommands cmd) const;
		void draw_indirect(RenderingDevice *p_rd, int64_t p_draw_list, DrawIndirectCommands cmd) const;

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
			BINDING_FOREMOST_FRAGMENT_BITMAP,
			BINDING_ALLOCATION_CONTOUR_PIXEL_BUFFER,
			BINDING_FOREMOST_CONTOUR_FRAGMENT_TO_CONTOUR_PIXEL_BUFFER,
			BINDING_CONTOUR_PIXEL_PIXEL_COORD_BUFFER,
			BINDING_CONTOUR_PIXEL_ORIENTATION_BUFFER,
			BINDING_CONTOUR_PIXEL_NORMAL_DEPTH_BUFFER,
			BINDING_MAX,
		};

		static constexpr uint32_t max_num_contour_fragments = 1u << 20u;
		static constexpr uint32_t max_num_contour_pixels = 1u << 20u;

		RID nearest_sampler;

		void receive_hard_depth_test_attachments(TypedArray<RID> p_attachments);

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 3; }
		virtual TypedArray<Ref<RDUniform>> get_draw_bindings() const;
	};

	struct PixelEdgeInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_PIXEL_EDGE_DESC_BUFFER = 0,
			BINDING_SPARSE_PIXEL_EDGE_NEIGHBOURS_BUFFER,
			BINDING_SPARSE_PIXEL_EDGE_MORTON_CODE_BUFFER,
			BINDING_SPARSE_PIXEL_EDGE_LOOP_BREAKING_BUFFER,
			BINDING_SPARSE_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER,
			BINDING_MAX,
		};

		static constexpr uint32_t max_num_sparse_pixel_edges = ContourInterfaceSet::max_num_contour_pixels * 4;

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 4; }
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

	struct HardDepthTestResources {
		int64_t framebuffer_format = RenderingDevice::INVALID_FORMAT_ID;
		RID color_attachment;
		RID depth_attachment;
		Vector2i prev_internal_size = Vector2i(0, 0);

		TypedArray<uint32_t> get_attachment_usage_flags() const;

		TypedArray<RID> get_attachments(RenderingDevice *p_rd, RenderData *p_render_data);
		int64_t get_framebuffer_format(RenderingDevice *p_rd);
		RID get_framebuffer(RenderingDevice *p_rd, RenderData *p_render_data);
		RID create_render_pipeline(RenderingDevice *p_rd, RID const &p_shader);

		void clear_attachments(RenderingDevice *p_rd, RenderData *p_render_data);
	};

private:

protected:
	static void _bind_methods();

public:
	GdstrokeShaderInterface() = default;
	~GdstrokeShaderInterface() = default;
};
