#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <iostream>
#include <string>
#include <vector>

#pragma once

namespace Render {

typedef struct VertexData {
    float x, y, z;
    float texture_x, texture_y;
} VertexData;

class SkinnedMesh {
   public:
    std::vector<VertexData> vertices;
    SDL_Surface* texture;

   public:
    SkinnedMesh(std::vector<VertexData> vertices, std::string texture_path) {
        load_texture(texture_path);
        this->vertices = vertices;
    };
    ~SkinnedMesh() { SDL_DestroySurface(texture); };

   private:
    void load_texture(std::string texture_path);
};
}  // namespace Render