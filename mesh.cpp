#include "mesh.h"

using namespace Render;

void SkinnedMesh::load_texture(std::string path) {
    int n_channels;
    texture = stbi_load(path.c_str(), &width, &height, &n_channels, 0);
}