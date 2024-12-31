#include "mesh.h"

using namespace Render;

void SkinnedMesh::load_texture(std::string path) {
    texture = IMG_Load(path.c_str());
    if (texture == nullptr) {
        std::cerr << "Failed to load texture: " << SDL_GetError() << std::endl;
    }
}