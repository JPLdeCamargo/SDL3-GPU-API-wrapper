// #include <SDL3/SDL.h>
// #include <SDL3/SDL_gpu.h>

// #include <iostream>
// #include <memory>
// #include <vector>

// struct Context {
//     Context() {
//         if (SDL_Init(SDL_INIT_VIDEO) < 0) {
//             std::cerr << SDL_GetError() << std::endl;
//         }
//         std::cout << "init done" << std::endl;
//     }
//     ~Context() {
//         std::cout << "context quit" << std::endl;
//         SDL_Quit();
//     }
// };

// struct Vertex {
//     std::array<float, 3> position;
//     std::array<float, 3> color;
// };

// const std::vector<Vertex> cube = {
//     {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // Red
//     {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},   // Green
//     {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},    // Blue
//     {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},   // Yellow
//     {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},   // Cyan
//     {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},    // Magenta
//     {{0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}},     // White
//     {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}},    // Black
// };

// int main(int, char **) {
//     auto context = std::make_unique<Context>;

//     std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window(
//         SDL_CreateWindow("SDL Hello", 500, 500, 0), SDL_DestroyWindow);

//     if (window == nullptr) {
//         std::cerr << SDL_GetError() << std::endl;
//         return -1;
//     }

//     std::unique_ptr<SDL_GPUDevice, void (*)(SDL_GPUDevice *)> gpu(
//         SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, 0, NULL),
//         SDL_DestroyGPUDevice);

//     if (gpu == nullptr) {
//         std::cerr << SDL_GetError() << std::endl;
//         return -1;
//     }
//     if (!SDL_ClaimWindowForGPUDevice(gpu.get(), window.get())) {
//         std::cerr << SDL_GetError() << std::endl;
//         return -1;
//     }

//     SDL_Surface *screen_surface = SDL_GetWindowSurface(window.get());
//     const SDL_PixelFormatDetails *details =
//         SDL_GetPixelFormatDetails(screen_surface->format);

//     if (details == nullptr) {
//         std::cerr << SDL_GetError() << std::endl;
//         return -1;
//     }

//     Uint32 color = SDL_MapRGB(details, NULL, 255, 0, 0);

//     SDL_FillSurfaceRect(screen_surface, NULL, color);

//     SDL_UpdateWindowSurface(window.get());

//     SDL_Event e;
//     bool quit = false;
//     while (quit == false) {
//         while (SDL_PollEvent(&e)) {
//             if (e.type == SDL_EVENT_QUIT) quit = true;

//             SDL_GPUCommandBuffer *cmdBuffer =
//                 SDL_AcquireGPUCommandBuffer(gpu.get());

//             if (cmdBuffer == nullptr) {
//                 std::cerr << SDL_GetError() << std::endl;
//                 return -1;
//             }
//             uint32_t w, h;
//             SDL_GPUTexture *swapChainTexture =
//                 SDL_AcquireGPUSwapchainTexture(cmdBuffer, window.get(), &w,
//                 &h);
//             SDL_GPURenderPass *renderPass = NULL;
//             SDL_GPUColorTargetInfo colorInfo;
//             SDL_zero(colorInfo);
//             colorInfo.texture = swapChainTexture;
//             colorInfo.clear_color.r = 0.1f;
//             colorInfo.clear_color.g = 0.5f;
//             colorInfo.clear_color.b = 0.1f;
//             colorInfo.clear_color.a = 1.0f;
//             colorInfo.load_op = SDL_GPU_LOADOP_CLEAR;
//             colorInfo.store_op = SDL_GPU_STOREOP_STORE;
//             renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorInfo, 1,
//             NULL); SDL_EndGPURenderPass(renderPass);
//             SDL_SubmitGPUCommandBuffer(cmdBuffer);
//             frameCount++;
//         }
//     }
// }
