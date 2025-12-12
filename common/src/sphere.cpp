#include <algorithm>
#include <cmath>
#include <intersection.hpp>
#include <optional>
#include <ray.hpp>
#include <sphere.hpp>
#include <string>
#include <vector.hpp>

namespace render {

  Sphere::Sphere(vector const & c, double r, std::string const & m) {
    center        = c;
    radius        = r;
    material_name = m;
  }

  // metodo que calcula la interseccion de un rayo con la esfera

  std::optional<Intersection> Sphere::collision(ray const & r) const {
    vector const rc            = vector::sub(r.origin, center);  // Vector desde or hasta center
    double const a             = vector::dotp(r.direction, r.direction);
    double const b             = 2 * vector::dotp(r.direction, rc);
    double const c             = vector::dotp(rc, rc) - radius * radius;
    double const discriminante = (b * b) - 4 * (a * c);  // discriminante(lambda)

    if (discriminante < 1e-8) {
      return std::nullopt;  // no hay intersección
    }

    double const raiz    = std::sqrt(discriminante);
    double const lambda1 = (-b - raiz) / (2.0 * a);
    double const lambda2 = (-b + raiz) / (2.0 * a);
    double lambda        = 0.0;
    if (lambda1 >= 1e-3 and lambda2 >= 1e-3) {
      lambda = std::min(lambda1, lambda2);
    } else if (lambda1 >= 1e-3) {
      lambda = lambda1;
    } else if (lambda2 >= 1e-3) {
      lambda = lambda2;
    } else {
      return std::nullopt;  // ambas intersecciones son negativas
    }

    vector const punto_interseccion =
        vector::add(r.origin, vector::muld(r.direction, lambda));  // vector punto interseccion
    vector normal =
        vector::sub(punto_interseccion, center);  // vector normal                 // normalizar
    normal = vector::normalize(normal);

    /*if (vector::dotp(r.direction, normal) > 0.0) {
      normal = vector::muld(normal, -1.0);  // invertir normal si el rayo entra
    }*/
    return Intersection(punto_interseccion, normal, lambda, material_name);
  }

}  // namespace render
