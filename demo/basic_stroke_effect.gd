class_name BasicStrokeEffect
extends CompositorEffect


@export var gdstroke_effect_id: int

var shader_path := "res://basic_stroke_shader.glsl"
var shader: RID
var pipeline: RID

var framebuffer_format: int = -1


func _init() -> void:
	init_resources()
	load_shaders()


func load_shaders() -> void:
	var rd := RenderingServer.get_rendering_device()
	var shader_spirv: RDShaderSPIRV = (load(shader_path) as RDShaderFile).get_spirv()
	shader = rd.shader_create_from_spirv(shader_spirv)


func make_pipelines(color_texture: RID, depth_texture: RID) -> void:
	var rd := RenderingServer.get_rendering_device()

	#region
	var framebuffer_attachmentformats : Array[RDAttachmentFormat] = [ RDAttachmentFormat.new(), RDAttachmentFormat.new() ];
	framebuffer_attachmentformats[0].format = rd.texture_get_format(color_texture).format
	framebuffer_attachmentformats[0].samples = rd.texture_get_format(color_texture).samples
	framebuffer_attachmentformats[0].usage_flags = rd.texture_get_format(color_texture).usage_bits
	framebuffer_attachmentformats[1].format = rd.texture_get_format(depth_texture).format
	framebuffer_attachmentformats[1].samples = rd.texture_get_format(depth_texture).samples
	framebuffer_attachmentformats[1].usage_flags = rd.texture_get_format(depth_texture).usage_bits
	framebuffer_format = rd.framebuffer_format_create(framebuffer_attachmentformats);

	var pipeline_rasterization_state := RDPipelineRasterizationState.new()
	var pipeline_multisample_state := RDPipelineMultisampleState.new()
	var pipeline_depthstencil_state := RDPipelineDepthStencilState.new()
	var pipeline_colorblend_state := RDPipelineColorBlendState.new()
	var pipeline_colorblend_state_attachment := RDPipelineColorBlendStateAttachment.new()
	pipeline_colorblend_state.attachments.append(pipeline_colorblend_state_attachment)

	var pipeline_colorblend_state_transparent := RDPipelineColorBlendState.new()
	var pipeline_colorblend_state_attachment_transparent := RDPipelineColorBlendStateAttachment.new()
	pipeline_colorblend_state_attachment_transparent.alpha_blend_op = RenderingDevice.BLEND_OP_ADD
	pipeline_colorblend_state_attachment_transparent.color_blend_op = RenderingDevice.BLEND_OP_ADD
	pipeline_colorblend_state_attachment_transparent.src_color_blend_factor = RenderingDevice.BLEND_FACTOR_SRC_ALPHA
	pipeline_colorblend_state_attachment_transparent.dst_color_blend_factor = RenderingDevice.BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
	pipeline_colorblend_state_attachment_transparent.src_alpha_blend_factor = RenderingDevice.BLEND_FACTOR_ONE
	pipeline_colorblend_state_attachment_transparent.dst_alpha_blend_factor = RenderingDevice.BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
	pipeline_colorblend_state_attachment_transparent.enable_blend = true
	pipeline_colorblend_state_transparent.attachments.append(pipeline_colorblend_state_attachment_transparent)
	#endregion

	pipeline = rd.render_pipeline_create(shader, framebuffer_format, RenderingDevice.INVALID_FORMAT_ID, RenderingDevice.RENDER_PRIMITIVE_TRIANGLES, pipeline_rasterization_state, pipeline_multisample_state, pipeline_depthstencil_state, pipeline_colorblend_state_transparent)


func init_resources() -> void:
	pass


func _render_callback(_callback_type: int, render_data: RenderData) -> void:
	var rd := RenderingServer.get_rendering_device()
	var scene_buffers: RenderSceneBuffersRD = render_data.get_render_scene_buffers()
	var gdstroke_effect: GdstrokeEffect = GdstrokeServer.get_gdstroke_effect(gdstroke_effect_id)

	if not pipeline.is_valid():
		make_pipelines(scene_buffers.get_color_texture(), scene_buffers.get_depth_texture())
	var framebuffer := FramebufferCacheRD.get_cache_multipass([scene_buffers.get_color_texture(), scene_buffers.get_depth_texture()], [], 1)
	assert(framebuffer_format == rd.framebuffer_get_format(framebuffer))

	var list := rd.draw_list_begin(framebuffer, RenderingDevice.DRAW_DEFAULT_ALL)
	rd.draw_list_bind_render_pipeline(list, pipeline)
	rd.draw_list_bind_uniform_set(list, gdstroke_effect.get_stroke_shader_uniform_set_rid(), gdstroke_effect.get_stroke_shader_uniform_set_slot())
	gdstroke_effect.draw_indirect_stroke_shader(rd, list)
	rd.draw_list_end()
