#include "matrix.h"

#include <SDL3/SDL.h>
using namespace Math;

Matrix3D get_rotation(float angle, float x, float y, float z) {
    float radians, c, s, c1, u[3], length;
    int i, j;

    radians = angle * SDL_PI_F / 180.0f;

    c = SDL_cosf(radians);
    s = SDL_sinf(radians);

    c1 = 1.0f - SDL_cosf(radians);

    length = (float)SDL_sqrt(x * x + y * y + z * z);

    u[0] = x / length;
    u[1] = y / length;
    u[2] = z / length;

    Matrix3D rotation_m;

    rotation_m[15] = 1.0;

    for (i = 0; i < 3; i++) {
        rotation_m[i * 4 + (i + 1) % 3] = u[(i + 2) % 3] * s;
        rotation_m[i * 4 + (i + 2) % 3] = -u[(i + 1) % 3] * s;
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            rotation_m[i * 4 + j] += c1 * u[i] * u[j] + (i == j ? c : 0.0f);
        }
    }
}
Matrix3D get_perspective_matrix(float fovy, float aspect, float znear,
                                float zfar) {
    int i;
    float f;

    f = 1.0f / SDL_tanf(fovy * 0.5f);

    Matrix3D perspective;

    perspective[0] = f / aspect;
    perspective[5] = f;
    perspective[10] = (znear + zfar) / (znear - zfar);
    perspective[11] = -1.0f;
    perspective[14] = (2.0f * znear * zfar) / (znear - zfar);
    perspective[15] = 0.0f;

    return perspective;
}

Matrix3D multiply_matrix(const Matrix3D& lhs, const Matrix3D& rhs) {
    Matrix3D res;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res[j * 4 + i] = 0.0;

            for (int k = 0; k < 4; k++) {
                res[j * 4 + i] += lhs[k * 4 + i] * rhs[j * 4 + k];
            }
        }
    }
    return res;
}