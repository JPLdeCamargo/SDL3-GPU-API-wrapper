#include <array>

namespace Math {

using Matrix3D = std::array<float, 16>;
class Matrix {
    /*
     * Simulates desktop's glRotatef. The matrix is returned in column-major
     * order.
     */
    static Matrix3D get_rotation(float angle, float x, float y, float z);

    /*
     * Simulates gluPerspectiveMatrix
     */
    static Matrix3D get_perspective_matrix(float fovy, float aspect,
                                           float znear, float zfar);
    /*
     * Multiplies lhs by rhs and writes out to r. All matrices are 4x4 and
     * column major. In-place multiplication is supported.
     */
    static Matrix3D multiply_matrix(const Matrix3D& lhs, const Matrix3D& rhs);
};
}  // namespace Math