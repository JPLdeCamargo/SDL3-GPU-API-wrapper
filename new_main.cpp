#include <array>
#include <iostream>

#include "SDL3_wrapper.h"
#include "mesh.h"

using namespace Render;

auto square = std::vector<VertexData>{
    {-0.5f, -0.5f, 0.0f, 0.0f, 0.0f},  // bottom-left
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f},   // bottom-right
    {0.5f, 0.5f, 0.0f, 1.0f, 1.0f},    // upper-right
    {-0.5f, 0.5f, 0.0f, 0.0f, 1.0f},   // upper-left
};
auto indices = std::vector<Uint32>{0, 1, 2, 0, 3, 2};
int main() {
    auto wrapper = Render::SDL3Wrapper("Test", 1000, 1000, false);

    std::string path = "/home/jp203/projects/prototypes/sdl_hello/wall.jpg";
    auto mesh = SkinnedMesh(square, indices, path);
    auto mesh_vector = {mesh};
    auto ptr = std::make_shared<std::vector<SkinnedMesh>>(mesh_vector);
    wrapper.main_loop(ptr);
}