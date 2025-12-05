#ifndef RENDER_SPHERE_HPP
#define RENDER_SPHERE_HPP

#include <intersection.hpp>
#include <object.hpp>
#include <optional>
#include <string>

namespace render {

  class Sphere : public Object3D {
  public:
    // Constructor
    Sphere(render::vector const & c, double r, std::string const & m);

    // Método de intersección con un rayo
    [[nodiscard]] std::optional<Intersection> collision(ray const & r) const override;
    // necesita el destructor virtual de la clase base
  };

}  // namespace render

#endif
