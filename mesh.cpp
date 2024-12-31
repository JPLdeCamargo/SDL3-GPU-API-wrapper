#include "mesh.h"

#include <iostream>

using namespace Render;

void SkinnedMesh::load_texture(std::string path) {
    texture = IMG_Load(path.c_str());
    if (texture == nullptr) {
        std::cerr << "Failed to load texture: " << SDL_GetError() << std::endl;
    }

    auto format = SDL_PIXELFORMAT_ABGR8888;
    if (texture->format != format) {
        std::cout << "formating" << std::endl;
        SDL_Surface *next = SDL_ConvertSurface(texture, format);
        SDL_DestroySurface(texture);
        texture = next;
    }
}