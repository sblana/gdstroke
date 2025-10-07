#pragma once

#include <godot_cpp/classes/render_scene_buffers_rd.hpp>
#include <godot_cpp/classes/render_data.hpp>
#include <godot_cpp/classes/uniform_set_cache_rd.hpp>
#include <godot_cpp/classes/rendering_device.hpp>

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

		enum Resource : uint32_t {
			RESOURCE_SCENE_DATA_UNIFORM = 0,
			RESOURCE_CONFIG_UNIFORM,
			RESOURCE_MAX,
		};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 0; }
	};

	struct MeshInterfaceSet : InterfaceSet {
		enum Binding : uint32_t {
			BINDING_MESH_DESC_BUFFER = 0,
			BINDING_VERTEX_BUFFER,
			BINDING_EDGE_TO_VERTEX_BUFFER,
			BINDING_EDGE_TO_FACE_BUFFER,
			BINDING_EDGE_IS_CONCAVE_BUFFER,
			BINDING_FACE_TO_VERTEX_BUFFER,
			BINDING_FACE_NORMAL_BUFFER,
			BINDING_FACE_BACKFACING_BUFFER,
			BINDING_MAX,
		};

		enum Resource : uint32_t {
			RESOURCE_MESH_DESC_BUFFER = 0,
			RESOURCE_VERTEX_BUFFER,
			RESOURCE_EDGE_TO_VERTEX_BUFFER,
			RESOURCE_EDGE_TO_FACE_BUFFER,
			RESOURCE_EDGE_IS_CONCAVE_BUFFER,
			RESOURCE_FACE_TO_VERTEX_BUFFER,
			RESOURCE_FACE_NORMAL_BUFFER,
			RESOURCE_FACE_BACKFACING_BUFFER,
			RESOURCE_MAX,
		};

		virtual Error create_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual Error update_resources(RenderingDevice *p_rd, RenderData *p_render_data) override;
		virtual void make_bindings() override;
		inline virtual uint32_t get_slot() const override { return 2; }
	};

private:

protected:
	static void _bind_methods();

public:
	GdstrokeShaderInterface() = default;
	~GdstrokeShaderInterface() = default;
};
