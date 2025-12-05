#ifndef RENDER_CYLINDER_HPP
#define RENDER_CYLINDER_HPP

#include <intersection.hpp>
#include <object.hpp>
#include <optional>
#include <string>

namespace render {

  class Cylinder : public Object3D {
    vector ejes;
    double altura{};

  public:
    // Constructor
    Cylinder(render::vector const & c, double r, render::vector a, std::string const & m);

    [[nodiscard]] std::optional<Intersection> collision(ray const & r) const override;

  private:
    [[nodiscard]] std::optional<Intersection> intersect_lateral(ray const & r) const;
    [[nodiscard]] std::optional<Intersection> intersect_base(ray const & r, vector const & P,
                                                             vector const & n) const;
    [[nodiscard]] static std::optional<double> ecuacion_cuadratica(double a, double b, double c);
  };

}  // namespace render

#endif
