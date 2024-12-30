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

void SDL3Wrapper::main_loop(
    std::shared_ptr<std::vector<std::array<SDL3Wrapper::VertexData, 3>>>
        meshes) {
    // SDL_Surface *screen_surface = SDL_GetWindowSurface(m_window.get());
    // const SDL_PixelFormatDetails *details =
    //     SDL_GetPixelFormatDetails(screen_surface->format);

    // CHECK_CREATE(details, "Pixel details");

    m_meshes = meshes;
    init_render();

    // Uint32 color = SDL_MapRGB(details, NULL, 255, 0, 0);

    // SDL_FillSurfaceRect(screen_surface, NULL, color);

    // SDL_UpdateWindowSurface(m_window.get());

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
    Uint32 size = sizeof(m_meshes->at(0));

    // Creating vertex buffer
    SDL_GPUBufferCreateInfo create_buff_info;
    create_buff_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    create_buff_info.size = size;
    create_buff_info.props = 0;

    m_vertex_buffer = SDL_CreateGPUBuffer(m_gpu.get(), &create_buff_info);
    CHECK_CREATE(m_vertex_buffer, "Static vertex buffer");

    // Creating transfer buffer
    SDL_GPUTransferBufferCreateInfo transfer_buffer_info;
    transfer_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer_info.size = size;
    transfer_buffer_info.props = 0;
    auto transfer_buffer =
        SDL_CreateGPUTransferBuffer(m_gpu.get(), &transfer_buffer_info);
    CHECK_CREATE(transfer_buffer, "Vertex transfer buffer");

    /* We just need to upload the static data once. */
    void *map = SDL_MapGPUTransferBuffer(m_gpu.get(), transfer_buffer, false);
    SDL_memcpy(map, &m_meshes->at(0), size);
    SDL_UnmapGPUTransferBuffer(m_gpu.get(), transfer_buffer);

    // Passing data with transfer buffer ????
    auto cmd_buffer = SDL_AcquireGPUCommandBuffer(m_gpu.get());
    auto copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);
    SDL_GPUTransferBufferLocation buf_location;
    buf_location.transfer_buffer = transfer_buffer;
    buf_location.offset = 0;

    SDL_GPUBufferRegion dst_region;
    dst_region.buffer = m_vertex_buffer;
    dst_region.offset = 0;
    dst_region.size = size;
    SDL_UploadToGPUBuffer(copy_pass, &buf_location, &dst_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd_buffer);

    SDL_ReleaseGPUTransferBuffer(m_gpu.get(), transfer_buffer);

    // Loading shaders
    auto vertex_shader = load_shader(vertex_spv, vertex_spv_len, true);
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
    SDL_GPUVertexAttribute vertex_attributes[1];
    vertex_attributes[0].buffer_slot = 0;
    vertex_attributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertex_attributes[0].location = 0;
    vertex_attributes[0].offset = 0;

    pipeline_info.vertex_input_state.num_vertex_buffers = 1;
    pipeline_info.vertex_input_state.vertex_buffer_descriptions =
        &vertex_buffer_desc;
    pipeline_info.vertex_input_state.num_vertex_attributes = 1;
    pipeline_info.vertex_input_state.vertex_attributes =
        (SDL_GPUVertexAttribute *)&vertex_attributes;

    pipeline_info.props = 0;

    m_pipeline = SDL_CreateGPUGraphicsPipeline(m_gpu.get(), &pipeline_info);
    CHECK_CREATE(m_pipeline, "Render Pipeline");

    /* These are reference-counted; once the pipeline is created, you don't need
     * to keep these. */
    SDL_ReleaseGPUShader(m_gpu.get(), vertex_shader);
    SDL_ReleaseGPUShader(m_gpu.get(), frag_shader);

    // setup depth texture
    Uint32 texture_w, texture_h;
    SDL_GetWindowSizeInPixels(m_window.get(), (int *)&texture_w,
                              (int *)&texture_h);
    setDepthTexture(texture_w, texture_h);
}

SDL_GPUShader *SDL3Wrapper::load_shader(const unsigned char (&compiled)[],
                                        const unsigned int compiled_len,
                                        bool is_vertex) {
    SDL_GPUShaderCreateInfo createinfo;
    createinfo.num_samplers = 0;
    createinfo.num_storage_buffers = 0;
    createinfo.num_storage_textures = 0;
    createinfo.num_uniform_buffers = is_vertex ? 1 : 0;
    SDL_GPUShaderFormat format = SDL_GetGPUShaderFormats(m_gpu.get());
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
    Uint32 texture_w, texture_h;
    bool acquired_swapchain_texture = SDL_AcquireGPUSwapchainTexture(
        command_buffer, m_window.get(), &swapchain_texture, &texture_w,
        &texture_h);
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

    // set up vertex binding
    SDL_GPUBufferBinding vertex_binding;
    vertex_binding.buffer = m_vertex_buffer;
    vertex_binding.offset = 0;

    auto pass =
        SDL_BeginGPURenderPass(command_buffer, &color_target, 1, &depth_target);
    SDL_BindGPUGraphicsPipeline(pass, m_pipeline);
    SDL_BindGPUVertexBuffers(pass, 0, &vertex_binding, 1);
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