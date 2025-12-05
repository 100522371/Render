#ifndef RENDER_RAY_HPP
#define RENDER_RAY_HPP

#include <vector.hpp>

namespace render {

  class ray {
  public:
    vector origin;
    vector direction;

    // vector no tiene ctor por defecto, inicializamos explícitamente
    ray() noexcept : origin{0.0, 0.0, 0.0}, direction{0.0, 0.0, 1.0} { }

    explicit ray(vector const & o, vector const & d) noexcept
        : origin{o}, direction{vector::normalize(d)} { }

    [[nodiscard]] vector at(double t) const noexcept {
      return vector::add(origin, vector::muld(direction, t));
    }
  };

}  // namespace render

#endif
