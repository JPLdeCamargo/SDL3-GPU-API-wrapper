namespace Math {

template <typename T>
class Vector3 {
   public:
    T x, y, z;

   public:
    Vector3() {}
    Vector3(T x, T y, T z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    Vector3 operator+(const Vector3<T>& rhs) {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }
    Vector3 operator-(const Vector3<T>& rhs) {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }
    Vector3 operator*(const Vector3<T>& lhs const Vector3<T>& rhs) {
        return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
    }
};
}  // namespace Math
