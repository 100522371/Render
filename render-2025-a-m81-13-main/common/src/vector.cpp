#include <cmath>
#include <stdexcept>
#include <vector.hpp>

namespace render {

  double vector::magnitude() const {
    return std::sqrt(x * x + y * y + z * z);
  }

  vector vector::sub(vector const & a, vector const & b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
  }

  vector vector::add(vector const & a, vector const & b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
  }

  vector vector::muld(vector const & a, double s) {
    return {a.x * s, a.y * s, a.z * s};
  }

  vector vector::divd(vector const & a, double s) {
    if (s == 0.0) {
      throw std::invalid_argument("division by zero");
    }
    return {a.x / s, a.y / s, a.z / s};
  }

  double vector::dotp(vector const & a, vector const & b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

  vector vector::crossp(vector const & a, vector const & b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
  }

  vector vector::normalize(vector const & v) {
    double const n = v.magnitude();
    if (n == 0.0) {
      return v;
    }
    return divd(v, n);
  }

}  // namespace render
