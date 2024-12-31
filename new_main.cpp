#include <array>
#include <iostream>

#include "SDL3_wrapper.h"
#include "mesh.h"

using namespace Render;

auto triangle = std::vector<VertexData>{
    {-0.5f, -0.5f, 0.0f, 0.0f, 0.0f},
    {0.5f, -0.5f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.5f, 0.0f, 0.5f, 1.0f},
};

int main() {
    auto wrapper = Render::SDL3Wrapper("Test", 500, 500, false);

    std::string path = "/home/jp203/projects/prototypes/sdl_hello/wall.jpg";
    auto mesh = SkinnedMesh(triangle, path);
    auto mesh_vector = {mesh};
    auto ptr = std::make_shared<std::vector<SkinnedMesh>>(mesh_vector);
    wrapper.main_loop(ptr);
}