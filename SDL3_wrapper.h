#include <SDL3/SDL.h>

#include <memory>
#include <string>
#include <vector>

namespace Render {
class SDL3Wrapper {
   public:
    typedef struct VertexData {
        float x, y, z;
    } VertexData;

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

    SDL_GPUBuffer* m_vertex_buffer;

    SDL_GPUGraphicsPipeline* m_pipeline;

    SDL_GPUTexture* m_depth_texture;

    std::shared_ptr<std::vector<std::array<SDL3Wrapper::VertexData, 3>>>
        m_meshes;

   private:
    void init_window(std::string title, int width, int height,
                     bool is_fullscreen);
    void init_render();

    SDL_GPUShader* load_shader(const unsigned char (&compiled)[],
                               const unsigned int compiled_len, bool is_vertex);

    void render();

    void setDepthTexture(Uint32 texture_width, Uint32 texture_height);

   public:
    SDL3Wrapper(std::string title, int width, int height, bool is_fullscreen) {
        init_window(title, width, height, is_fullscreen);
    }
    ~SDL3Wrapper() {
        // Will be released first
        if (m_depth_texture != nullptr)
            SDL_ReleaseGPUTexture(m_gpu.get(), m_depth_texture);
        if (m_vertex_buffer != nullptr)
            SDL_ReleaseGPUBuffer(m_gpu.get(), m_vertex_buffer);
        if (m_pipeline != nullptr)
            SDL_ReleaseGPUGraphicsPipeline(m_gpu.get(), m_pipeline);
        SDL_ReleaseWindowFromGPUDevice(m_gpu.get(), m_window.get());
    }
    void main_loop(
        std::shared_ptr<std::vector<std::array<SDL3Wrapper::VertexData, 3>>>
            meshes);
};
}  // namespace Render