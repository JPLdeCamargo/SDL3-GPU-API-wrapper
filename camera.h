#include "matrix.h"
#include "vec3.h"

namespace Render {
class Camera {
   public:
    Math::Vector3<double> position;
    Math::Vector3<double> forward;
    Math::Vector3<double> left;
    Math::Vector3<double> up;

   private:
    Math::Matrix3D m_mvp;

   public:
    Camera() {};
};
}  // namespace Render