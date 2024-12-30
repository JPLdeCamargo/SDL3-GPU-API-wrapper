#include <array>
#include <iostream>

#include "SDL3_wrapper.h"

using namespace Render;

auto triangle = std::array<SDL3Wrapper::VertexData, 3>{
    {{0.0f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f}, {-0.5f, 0.5f, 0.0f}}};
// std::vector<float> triangle = {
//     -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f,
// };

using meshes = std::vector<std::array<SDL3Wrapper::VertexData, 3>>;

int main() {
    std::cout << sizeof(triangle) << std::endl;
    auto wrapper = Render::SDL3Wrapper("Test", 500, 500, false);
    auto to_render = std::make_shared<meshes>(meshes{triangle});
    wrapper.main_loop(to_render);
}