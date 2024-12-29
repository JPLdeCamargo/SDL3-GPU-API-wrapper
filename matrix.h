#include <array>

namespace Math {
class Matrix {
    /*
     * Simulates desktop's glRotatef. The matrix is returned in column-major
     * order.
     */
    static void rotate_matrix(float angle, float x, float y, float z,
                              std::array<float, 16>& r);
};
}  // namespace Math