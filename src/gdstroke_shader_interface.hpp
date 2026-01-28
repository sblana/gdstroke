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
			uint32_t min_segment_length = 32;
			float stroke_width = 1.0;
			float stroke_width_factor_start = 1.0;
			float stroke_width_factor_end = 0.5;
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
			DISPATCH_INDIRECT_COMMANDS_REUSABLE_ALLOCATION_L0 = 0,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_GLOBAL_FACES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_EDGES,
			DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_CONTOUR_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_FRAGMENTS,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_CONTOUR_PIXELS,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SPARSE_PIXEL_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_FRAGMENTED_PIXEL_EDGES,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_COMPACTED_PIXEL_EDGES,
			DISPATCH_INDIRECT_COMMANDS_WORKGROUP_TO_PIXEL_EDGE_LOOPS,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENTS,
			DISPATCH_INDIRECT_COMMANDS_INVOCATION_TO_SEGMENT_EDGES,
			DISPATCH_INDIRECT_COMMANDS_MAX,
		};

		enum DrawIndirectCommands : uint32_t {
			DRAW_INDIRECT_COMMANDS_HARD_DEPTH_TEST = 0,
			DRAW_INDIRECT_COMMANDS_STROKE_RENDERING,
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
		enum Buffer : uint32_t {
			BUFFER_GEOMETRY_DESC_BUFFER = 0,
			BUFFER_MESH_DESC_BUFFER,
			BUFFER_MESH_INSTANCE_DESC_BUFFER,
			BUFFER_MESH_INSTANCE_MAPS_BUFFER,
			BUFFER_ALLOCATION_COLUMN_BUFFER,
			BUFFER_GLOBAL_EDGES_BUFFER,
			BUFFER_GLOBAL_FACES_BUFFER,
			BUFFER_MAX,
		};

		enum Binding : uint32_t {
			BINDING_MESH_BUFFERS = 0,
			BINDING_MAX,
		};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 2; }
	};

	struct ContourInterfaceSet : InterfaceSet {
		enum Buffer : uint32_t {
			BUFFER_CONTOUR_DESC_BUFFER = 0,
			BUFFER_MEMORY_BLOCK_BUFFER,
			BUFFER_CONTOUR_EDGE_MAPS_BUFFER,
			BUFFER_CONTOUR_EDGE_CLIP_T_BUFFER,
			BUFFER_CONTOUR_FRAGMENT_ATTRIBS_BUFFER,
			BUFFER_CONTOUR_FRAGMENT_TO_CONTOUR_EDGE_BUFFER,
			BUFFER_CONTOUR_FRAGMENT_PSEUDO_VISIBLE_BUFFER,
			BUFFER_ALLOCATION_CONTOUR_PIXEL_BUFFER,
			BUFFER_FOREMOST_CONTOUR_FRAGMENT_TO_CONTOUR_PIXEL_BUFFER,
			BUFFER_CONTOUR_PIXEL_ATTRIBS_BUFFER,
			BUFFER_MAX,
		};

		enum Binding : uint32_t {
			BINDING_CONTOUR_BUFFERS = 0,
			BINDING_SCREEN_DEPTH_TEXTURE,
			BINDING_FOREMOST_FRAGMENT_BITMAP,
			BINDING_MAX,
		};

		static constexpr uint32_t max_num_contour_fragments = 1u << 18u;
		static constexpr uint32_t max_num_contour_pixels = 1u << 18u;
		static constexpr uint32_t balloc_buffer_size = 16 * 1024 * 1024;

		RID nearest_sampler;

		void receive_hard_depth_test_attachments(TypedArray<RID> p_attachments);

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 3; }
		virtual TypedArray<Ref<RDUniform>> get_draw_bindings() const override;
	};

	struct PixelEdgeInterfaceSet : InterfaceSet {
		enum Buffer : uint32_t {
			BUFFER_PIXEL_EDGE_DESC_BUFFER = 0,
			BUFFER_SPARSE_PIXEL_EDGE_NEIGHBOURS_BUFFER,
			BUFFER_SPARSE_PIXEL_EDGE_IS_VALID_BUFFER,
			BUFFER_SPARSE_PIXEL_EDGE_TO_FRAGMENTED_PIXEL_EDGE_BUFFER,
			BUFFER_FRAGMENTED_PIXEL_EDGE_TO_SPARSE_PIXEL_EDGE_BUFFER,
			BUFFER_FRAGMENTED_PIXEL_EDGE_WYLLIE_BUFFER,
			BUFFER_FRAGMENTED_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER,
			BUFFER_FRAGMENTED_PIXEL_EDGE_IS_HEAD_BUFFER,
			BUFFER_FRAGMENTED_PIXEL_EDGE_LOCAL_IDX_BUFFER,
			BUFFER_FRAGMENTED_PIXEL_EDGE_TO_PIXEL_EDGE_LOOP_BUFFER,
			BUFFER_PIXEL_EDGE_LOOP_DESC_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_NEIGHBOURS_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_TO_CONTOUR_PIXEL_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_ORIENTATION_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_ASSOCIATED_HEAD_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_TO_PIXEL_EDGE_LOOP_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_FILTERED_MIDPOINT_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_FILTERED_ORIENTATION_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_IS_INSIDE_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_IS_SEGMENT_HEAD_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_LOOP_LOCAL_SEGMENT_KEY_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_IS_DISCARDED_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_SEGMENT_DESC_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_SEGMENT_KEY_BUFFER,
			BUFFER_COMPACTED_PIXEL_EDGE_TO_SEGMENT_EDGE_BUFFER,
			BUFFER_PIXEL_EDGE_LOOP_SEGMENTS_DESC_BUFFER,
			BUFFER_ALLOCATION_SEGMENT_BUFFER,
			BUFFER_SEGMENT_DESC_BUFFER,
			BUFFER_SEGMENT_EDGE_TO_COMPACTED_PIXEL_EDGE_BUFFER,
			BUFFER_SEGMENT_EDGE_IS_HEAD_BUFFER,
			BUFFER_SEGMENT_EDGE_ARC_LENGTH_BUFFER,
			BUFFER_SEGMENT_ARC_LENGTH_BUFFER,
			BUFFER_SEGMENT_RANGE_BUFFER,
			BUFFER_STROKE_DESC_BUFFER,
			BUFFER_SEGMENT_STROKE_VERTEX_RANGE_BUFFER,
			BUFFER_STROKE_VERTEX_TO_SEGMENT_EDGE_BUFFER,
			BUFFER_STROKE_VERTEX_KIND_BUFFER,
			BUFFER_MAX,
		};

		enum Binding : uint32_t {
			BINDING_PIXEL_EDGE_BUFFERS = 0,
			BINDING_MAX,
		};

		static constexpr uint32_t max_num_sparse_pixel_edges = ContourInterfaceSet::max_num_contour_pixels * 4;
		static constexpr uint32_t max_num_fragmented_pixel_edges = max_num_sparse_pixel_edges / 2;
		static constexpr uint32_t max_num_compacted_pixel_edges = max_num_fragmented_pixel_edges;
		static constexpr uint32_t max_num_pixel_edge_loops = max_num_compacted_pixel_edges / 4;
		static constexpr uint32_t max_num_segments = max_num_compacted_pixel_edges / 8;
		static constexpr uint32_t max_num_segment_edges = max_num_segments * 4;
		static constexpr uint32_t max_num_stroke_vertices = max_num_segment_edges * 4;

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 4; }
	};

	struct ShaderAPIInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_BUFFER_PTR_TABLE_BUFFER = 0,
			BINDING_MAX,
		};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 7; }
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

		void clear_color_attachments(RenderingDevice *p_rd, RenderData *p_render_data);
	};

	struct StrokeRenderingResources {
		int64_t framebuffer_format = RenderingDevice::INVALID_FORMAT_ID;

		TypedArray<RID> get_attachments(RenderingDevice *p_rd, RenderData *p_render_data);
		int64_t get_framebuffer_format(RenderingDevice *p_rd, RenderData *p_render_data);
		RID get_framebuffer(RenderingDevice *p_rd, RenderData *p_render_data);
		RID create_render_pipeline(RenderingDevice *p_rd, RenderData *p_render_data, RID const &p_shader);
	};

private:

protected:
	static void _bind_methods();

public:
	GdstrokeShaderInterface() = default;
	~GdstrokeShaderInterface() = default;
};
