#include "SDL3_wrapper.h"

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

void SDL3Wrapper::main_loop() {
    SDL_Surface *screen_surface = SDL_GetWindowSurface(m_window.get());
    const SDL_PixelFormatDetails *details =
        SDL_GetPixelFormatDetails(screen_surface->format);

    CHECK_CREATE(details, "Pixel details");

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