#ifndef OBJECT3D_HPP
#define OBJECT3D_HPP

#include <intersection.hpp>
#include <optional>
#include <ray.hpp>
#include <string>
#include <vector.hpp>

namespace render {

  class Object3D {
  public:
    std::string material_name;
    double radius{};
    vector center;

    Object3D()                             = default;
    Object3D(Object3D const &)             = default;
    Object3D & operator=(Object3D const &) = default;
    Object3D(Object3D &&)                  = default;
    Object3D & operator=(Object3D &&)      = default;
    virtual ~Object3D()                    = default;

    // Método de intersección con un rayo
    [[nodiscard]] virtual std::optional<Intersection> collision(ray const & r) const = 0;
  };

}  // namespace render
#endif
