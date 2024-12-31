#include "SDL3_wrapper.h"

#include <iostream>

#include "frag_shader.h"
#include "vertex_shader.h"

#define CHECK_CREATE(var, thing)                                         \
    {                                                                    \
        if (!(var)) {                                                    \
            SDL_Log("Failed to create %s: %s\n", thing, SDL_GetError()); \
            exit(2);                                                     \
        }                                                                \
    }

using namespace Render;

void SDL3Wrapper::init_window(std::string title, int width, int height,
                              bool is_fullscreen) {
    m_context = std::make_unique<Context>();
    CHECK_CREATE(m_context, "context");

    m_window.reset(SDL_CreateWindow(title.c_str(), width, height, 0));
    CHECK_CREATE(m_window, "window");

    SDL_GPUShaderFormat support_flags = SDL_GPU_SHADERFORMAT_SPIRV;
    if (is_fullscreen) support_flags |= SDL_WINDOW_FULLSCREEN;
    m_gpu.reset(SDL_CreateGPUDevice(support_flags, false, NULL));
    CHECK_CREATE(m_gpu, "GPU device");

    SDL_ClaimWindowForGPUDevice(m_gpu.get(), m_window.get());
}

void SDL3Wrapper::main_loop(std::shared_ptr<std::vector<SkinnedMesh>> meshes) {
    m_meshes = meshes;

    init_render();

    SDL_Event e;
    bool quit = false;
    while (quit == false) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) quit = true;
            render();
        }
    }
}

// break into smaller functions, maybe
void SDL3Wrapper::init_render() {
    const auto &vertices = m_meshes->at(0).vertices;
    const auto &texture = m_meshes->at(0).texture;
    Uint32 size = sizeof(vertices[0]) * vertices.size();
    Uint32 text_size = texture->h * texture->w * 4;

    // Creating vertex buffer
    auto vert_info = SDL_GPUBufferCreateInfo{
        .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
        .size = size,
    };
    m_vertex_buffer = SDL_CreateGPUBuffer(m_gpu.get(), &vert_info);

    CHECK_CREATE(m_vertex_buffer, "Static vertex buffer");

    // Creating texture resources
    auto tex_create_info = SDL_GPUTextureCreateInfo{
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width = (Uint32)texture->w,
        .height = (Uint32)texture->h,
        .layer_count_or_depth = 1,
        .num_levels = 1,
    };
    m_texture = SDL_CreateGPUTexture(m_gpu.get(), &tex_create_info);
    CHECK_CREATE(m_texture, "Texture gpu");

    // Creating transfer buffer
    auto transfer_info = SDL_GPUTransferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = size,
    };
    auto transfer_buffer =
        SDL_CreateGPUTransferBuffer(m_gpu.get(), &transfer_info);
    CHECK_CREATE(transfer_buffer, "Vertex transfer buffer");

    /* We just need to upload the static data once. */
    void *map = SDL_MapGPUTransferBuffer(m_gpu.get(), transfer_buffer, false);
    SDL_memcpy(map, vertices.data(), size);
    SDL_UnmapGPUTransferBuffer(m_gpu.get(), transfer_buffer);

    // Set up texture data
    auto transfer_tex_info = SDL_GPUTransferBufferCreateInfo{
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = text_size,
    };
    auto texture_transfer_buffer =
        SDL_CreateGPUTransferBuffer(m_gpu.get(), &transfer_tex_info);

    // Transfer texture data
    void *texture_transfer_ptr =
        SDL_MapGPUTransferBuffer(m_gpu.get(), texture_transfer_buffer, false);
    SDL_memcpy(texture_transfer_ptr, texture->pixels, text_size);
    SDL_UnmapGPUTransferBuffer(m_gpu.get(), texture_transfer_buffer);

    // Upload the transfer data to the GPU resources
    auto cmd_buffer = SDL_AcquireGPUCommandBuffer(m_gpu.get());
    auto copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);

    auto transfer_location = SDL_GPUTransferBufferLocation{
        .transfer_buffer = transfer_buffer,
        .offset = 0,
    };
    auto buffer_region = SDL_GPUBufferRegion{
        .buffer = m_vertex_buffer,
        .offset = 0,
        .size = size,
    };
    SDL_UploadToGPUBuffer(copy_pass, &transfer_location, &buffer_region, false);

    auto texture_transfer_info = SDL_GPUTextureTransferInfo{
        .transfer_buffer = texture_transfer_buffer,
        .offset = 0,
    };
    auto texture_buffer_region = SDL_GPUTextureRegion{
        .texture = m_texture,
        .w = (Uint32)texture->w,
        .h = (Uint32)texture->h,
        .d = 1,
    };
    SDL_UploadToGPUTexture(copy_pass, &texture_transfer_info,
                           &texture_buffer_region, false);

    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd_buffer);

    SDL_ReleaseGPUTransferBuffer(m_gpu.get(), transfer_buffer);
    SDL_ReleaseGPUTransferBuffer(m_gpu.get(), texture_transfer_buffer);

    // Loading shaders
    auto vertex_shader = load_shader(vert_spv, vert_spv_len, true);
    auto frag_shader = load_shader(frag_spv, frag_spv_len, false);

    // Pipeline setup
    SDL_GPUGraphicsPipelineCreateInfo pipeline_info;
    SDL_GPUColorTargetDescription color_target_desc;
    SDL_zero(pipeline_info);
    SDL_zero(color_target_desc);

    color_target_desc.format =
        SDL_GetGPUSwapchainTextureFormat(m_gpu.get(), m_window.get());

    pipeline_info.target_info.num_color_targets = 1;
    pipeline_info.target_info.color_target_descriptions = &color_target_desc;
    pipeline_info.target_info.depth_stencil_format =
        SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    pipeline_info.target_info.has_depth_stencil_target = true;

    pipeline_info.depth_stencil_state.enable_depth_test = true;
    pipeline_info.depth_stencil_state.enable_depth_write = true;
    pipeline_info.depth_stencil_state.compare_op =
        SDL_GPU_COMPAREOP_LESS_OR_EQUAL;

    pipeline_info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;

    pipeline_info.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    pipeline_info.vertex_shader = vertex_shader;
    pipeline_info.fragment_shader = frag_shader;

    SDL_GPUVertexBufferDescription vertex_buffer_desc;
    vertex_buffer_desc.slot = 0;
    vertex_buffer_desc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertex_buffer_desc.instance_step_rate = 0;
    vertex_buffer_desc.pitch = sizeof(VertexData);

    // Setup vertex attributes, "in" variables on shader
    SDL_GPUVertexAttribute vertex_attributes[2]{
        {
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0,
        },
        {
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
            .offset = sizeof(float) * 3,
        }};

    pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    pipeline_info.vertex_input_state.vertex_buffer_descriptions =
        &vertex_buffer_desc;
    pipeline_info.vertex_input_state.num_vertex_attributes = 2;
    pipeline_info.vertex_input_state.vertex_attributes =
        (SDL_GPUVertexAttribute *)&vertex_attributes;

    pipeline_info.props = 0;
    m_pipeline = SDL_CreateGPUGraphicsPipeline(m_gpu.get(), &pipeline_info);
    CHECK_CREATE(m_pipeline, "Render Pipeline");

    /* These are reference-counted; once the pipeline is created, you don't need
     * to keep these. */
    SDL_ReleaseGPUShader(m_gpu.get(), vertex_shader);
    SDL_ReleaseGPUShader(m_gpu.get(), frag_shader);

    // Set up depth texture
    Uint32 depth_w, depth_h;
    SDL_GetWindowSizeInPixels(m_window.get(), (int *)&depth_w, (int *)&depth_h);
    setDepthTexture(depth_w, depth_h);

    // Set up sampler
    auto sampler_info = SDL_GPUSamplerCreateInfo{
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    m_texture_sampler = SDL_CreateGPUSampler(m_gpu.get(), &sampler_info);
}

SDL_GPUShader *SDL3Wrapper::load_shader(const unsigned char (&compiled)[],
                                        const unsigned int compiled_len,
                                        bool is_vertex) {
    SDL_GPUShaderCreateInfo createinfo;
    createinfo.num_samplers = is_vertex ? 0 : 1;
    createinfo.num_storage_buffers = 0;
    createinfo.num_storage_textures = 0;
    createinfo.num_uniform_buffers = 0;
    createinfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
    createinfo.code = compiled;
    createinfo.code_size = compiled_len;
    createinfo.entrypoint = "main";

    createinfo.stage =
        is_vertex ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;
    return SDL_CreateGPUShader(m_gpu.get(), &createinfo);
}

void SDL3Wrapper::render() {
    // acquire command buffer and swap texture
    auto command_buffer = SDL_AcquireGPUCommandBuffer(m_gpu.get());
    CHECK_CREATE(command_buffer, "Command buffer");

    SDL_GPUTexture *swapchain_texture;
    bool acquired_swapchain_texture = SDL_AcquireGPUSwapchainTexture(
        command_buffer, m_window.get(), &swapchain_texture, nullptr, nullptr);
    CHECK_CREATE(acquired_swapchain_texture, "Swapchain texture")

    if (swapchain_texture == nullptr) {
        /* Swapchain is unavailable, cancel work */
        SDL_CancelGPUCommandBuffer(command_buffer);
        return;
    }

    // set up color and depth target
    SDL_GPUColorTargetInfo color_target;
    SDL_zero(color_target);

    color_target.load_op = SDL_GPU_LOADOP_CLEAR;
    color_target.store_op = SDL_GPU_STOREOP_STORE;
    color_target.texture = swapchain_texture;

    SDL_GPUDepthStencilTargetInfo depth_target;
    SDL_zero(depth_target);
    depth_target.clear_depth = 1.0f;
    depth_target.load_op = SDL_GPU_LOADOP_CLEAR;
    depth_target.store_op = SDL_GPU_STOREOP_DONT_CARE;
    depth_target.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
    depth_target.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
    depth_target.texture = m_depth_texture;
    depth_target.cycle = true;

    auto pass =
        SDL_BeginGPURenderPass(command_buffer, &color_target, 1, &depth_target);

    // bindings
    SDL_BindGPUGraphicsPipeline(pass, m_pipeline);

    auto vert_binding = SDL_GPUBufferBinding{
        .buffer = m_vertex_buffer,
        .offset = 0,
    };
    SDL_BindGPUVertexBuffers(pass, 0, &vert_binding, 1);

    auto tex_binding = SDL_GPUTextureSamplerBinding{
        .texture = m_texture,
        .sampler = m_texture_sampler,
    };
    SDL_BindGPUFragmentSamplers(pass, 0, &tex_binding, 1);

    SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
}

void SDL3Wrapper::setDepthTexture(Uint32 texture_width, Uint32 texture_height) {
    SDL_GPUTextureCreateInfo create_info;

    create_info.type = SDL_GPU_TEXTURETYPE_2D;
    create_info.format = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
    create_info.width = texture_width;
    create_info.height = texture_height;
    create_info.layer_count_or_depth = 1;
    create_info.num_levels = 1;
    create_info.sample_count = SDL_GPU_SAMPLECOUNT_1;
    create_info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
    create_info.props = 0;

    m_depth_texture = SDL_CreateGPUTexture(m_gpu.get(), &create_info);
    CHECK_CREATE(m_depth_texture, "Depth Texture");
}
