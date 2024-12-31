#include <string>
#include <vector>

#pragma once

#include "stb_image.h"

namespace Render {

typedef struct VertexData {
    float x, y, z;
    float texture_x, texture_y;
} VertexData;

class SkinnedMesh {
   public:
    std::vector<VertexData> vertices;
    unsigned char* texture;
    int texture_size;
    int width, height;

   public:
    SkinnedMesh(std::vector<VertexData> vertices, std::string texture_path) {
        load_texture(texture_path);
        this->vertices = vertices;
    };
    ~SkinnedMesh() { stbi_image_free(texture); };

   private:
    void load_texture(std::string texture_path);
};
}  // namespace Render