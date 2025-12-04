#ifndef RENDER_VECTOR_HPP
#define RENDER_VECTOR_HPP

#include <cmath>

namespace render {

  class vector {
  public:
    double x, y, z;

    vector() : x{0}, y{0}, z{0} { }

    vector(double cx, double cy, double cz) : x{cx}, y{cy}, z{cz} { }

    [[nodiscard]] double magnitude() const;
    [[nodiscard]] static vector normalize(vector const & v);

    [[nodiscard]] static vector sub(vector const & a, vector const & b);
    [[nodiscard]] static vector add(vector const & a, vector const & b);

    [[nodiscard]] static vector muld(vector const & a, double s);
    [[nodiscard]] static vector divd(vector const & a, double s);

    static vector mul(vector const & a, vector const & b) noexcept {
      return {a.x * b.x, a.y * b.y, a.z * b.z};
    }

    [[nodiscard]] static double dotp(vector const & a, vector const & b);
    [[nodiscard]] static vector crossp(vector const & a, vector const & b);
  };

}  // namespace render

#endif
