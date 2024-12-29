// gcc main.c -o main -lSDL3

#include <SDL3/SDL.h>

int windowW = 900;
int windowH = 900;

int main(int argc, char** argv) {
    int dispW, dispH;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("My GPU Test", windowW, windowH, 0);

    SDL_GPUShaderFormat supportFlags = SDL_GPU_SHADERFORMAT_SPIRV;
    SDL_GPUDevice* gpuDevice = SDL_CreateGPUDevice(supportFlags, true, NULL);
    SDL_ClaimWindowForGPUDevice(gpuDevice, win);

    int frameCount = 0;
    bool run = true;
    size_t startTick = SDL_GetTicks();
    while (run) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (ev.key.key) {
                        case SDLK_ESCAPE:
                            run = false;
                            break;
                    }
                    break;
                case SDL_EVENT_QUIT:
                    run = false;
                    break;
            }
        }

        SDL_GPUCommandBuffer* cmdBuffer =
            SDL_AcquireGPUCommandBuffer(gpuDevice);
        if (cmdBuffer) {
            uint32_t w, h;
            SDL_GPUTexture* swapChainTexture;
            SDL_AcquireGPUSwapchainTexture(cmdBuffer, win, &swapChainTexture, 0,
                                           0);

            SDL_GPURenderPass* renderPass = NULL;
            SDL_GPUColorTargetInfo colorInfo;
            SDL_zero(colorInfo);
            colorInfo.texture = swapChainTexture;
            colorInfo.clear_color.r = SDL_sin(frameCount / 100.0f) / 2 + 0.5f;
            colorInfo.clear_color.g = SDL_sin(frameCount / 120.0f) / 2 + 0.5f;
            colorInfo.clear_color.b = SDL_sin(frameCount / 133.0f) / 2 + 0.5f;
            colorInfo.clear_color.a = 1.0f;
            colorInfo.load_op = SDL_GPU_LOADOP_CLEAR;
            colorInfo.store_op = SDL_GPU_STOREOP_STORE;
            renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorInfo, 1, NULL);
            SDL_EndGPURenderPass(renderPass);
            SDL_SubmitGPUCommandBuffer(cmdBuffer);
            frameCount++;
        } else {
            SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
            run = false;
        }
    }

    SDL_Log("FPS: %ld frames per sec",
            (frameCount * 1000) / (SDL_GetTicks() - startTick));
    SDL_ReleaseWindowFromGPUDevice(gpuDevice, win);
    SDL_DestroyGPUDevice(gpuDevice);
    SDL_DestroyWindow(win);
    SDL_Quit();
}