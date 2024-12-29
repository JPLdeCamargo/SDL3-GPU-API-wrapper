#include <SDL3/SDL.h>

#include <memory>
#include <string>

namespace Render {
class SDL3Wrapper {
   private:
    struct Context {
        Context() { SDL_Init(SDL_INIT_VIDEO); }
        ~Context() { SDL_Quit(); }
    };

   private:
    // must be declared first, in order to delete last
    std::unique_ptr<Context> m_context;

    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> m_window = {
        nullptr, SDL_DestroyWindow};
    std::unique_ptr<SDL_GPUDevice, decltype(&SDL_DestroyGPUDevice)> m_gpu = {
        nullptr, SDL_DestroyGPUDevice};

   private:
    void init_window(std::string title, int width, int height,
                     bool is_fullscreen);

   public:
    SDL3Wrapper(std::string title, int width, int height, bool is_fullscreen) {
        init_window(title, width, height, is_fullscreen);
    }
    ~SDL3Wrapper() {
        SDL_ReleaseWindowFromGPUDevice(m_gpu.get(), m_window.get());
    }
    void main_loop();
};
}  // namespace Render