#include <vector>

#include "SDL3_wrapper.h"

using namespace Render;

std::vector<float> triangle = {
    -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f,
};

using meshes = std::vector<std::vector<float>>;

int main() {
    auto wrapper = Render::SDL3Wrapper("Test", 500, 500, false);

    auto to_render = std::make_shared<meshes>(meshes{triangle});
    wrapper.main_loop(to_render);
}