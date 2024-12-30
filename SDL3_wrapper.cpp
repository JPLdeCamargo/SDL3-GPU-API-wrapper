#include "SDL3_wrapper.h"

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
    std::shared_ptr<std::vector<std::vector<float>>> meshes) {
    SDL_Surface *screen_surface = SDL_GetWindowSurface(m_window.get());
    const SDL_PixelFormatDetails *details =
        SDL_GetPixelFormatDetails(screen_surface->format);

    CHECK_CREATE(details, "Pixel details");

    m_meshes = meshes;
    init_render();

    Uint32 color = SDL_MapRGB(details, NULL, 255, 0, 0);

    SDL_FillSurfaceRect(screen_surface, NULL, color);

    SDL_UpdateWindowSurface(m_window.get());

    SDL_Event e;
    bool quit = false;
    while (quit == false) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) quit = true;
        }
    }
}

void SDL3Wrapper::init_render() {
    // Creating vertex buffer
    SDL_GPUBufferCreateInfo vertex_buffer_info;
    vertex_buffer_info.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    vertex_buffer_info.size = sizeof(m_meshes.get());
    vertex_buffer_info.props = 0;

    m_vertex_buffer = SDL_CreateGPUBuffer(m_gpu.get(), &vertex_buffer_info);
    CHECK_CREATE(m_vertex_buffer, "Static vertex buffer");

    // Creating transfer buffer
    SDL_GPUTransferBufferCreateInfo transfer_buffer_info;
    transfer_buffer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_buffer_info.size = sizeof(m_meshes.get());
    transfer_buffer_info.props = 0;
    SDL_GPUTransferBuffer *transfer_buffer =
        SDL_CreateGPUTransferBuffer(m_gpu.get(), &transfer_buffer_info);
    CHECK_CREATE(transfer_buffer, "Vertex transfer buffer");

    /* We just need to upload the static data once. */
    void *map = SDL_MapGPUTransferBuffer(m_gpu.get(), transfer_buffer, false);
    SDL_memcpy(map, m_meshes.get(), sizeof(m_meshes.get()));
    SDL_UnmapGPUTransferBuffer(m_gpu.get(), transfer_buffer);

    // Passing data with transfer buffer ????
    SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(m_gpu.get());
    SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);
    SDL_GPUTransferBufferLocation buf_location;
    buf_location.transfer_buffer = transfer_buffer;
    buf_location.offset = 0;

    SDL_GPUBufferRegion dst_region;
    dst_region.buffer = m_vertex_buffer;
    dst_region.offset = 0;
    dst_region.size = sizeof(m_meshes.get());
    SDL_UploadToGPUBuffer(copy_pass, &buf_location, &dst_region, false);
    SDL_EndGPUCopyPass(copy_pass);
    SDL_SubmitGPUCommandBuffer(cmd_buffer);

    SDL_ReleaseGPUTransferBuffer(m_gpu.get(), transfer_buffer);
}