#include <algorithm>
#include <cmath>
#include <cylinder.hpp>
#include <intersection.hpp>
#include <optional>
#include <ray.hpp>
#include <string>
#include <vector.hpp>

namespace render {

  Cylinder::Cylinder(vector const & c, double r, vector a, std::string const & m)
      : ejes{vector::normalize(a)}, altura{a.magnitude()} {
    center        = c;
    radius        = r;
    material_name = m;
  }

  std::optional<Intersection> Cylinder::collision(ray const & r) const {
    auto lateral            = intersect_lateral(r);
    vector const p_superior = vector::add(center, vector::muld(ejes, altura / 2.0));
    vector const p_inferior = vector::add(center, vector::muld(ejes, -altura / 2.0));

    auto base_superior = intersect_base(r, p_superior, ejes);
    auto base_inferior = intersect_base(r, p_inferior, vector::muld(ejes, -1.0));

    std::optional<Intersection> cercano = lateral;
    if (base_superior and (not cercano or base_superior->lambda < cercano->lambda)) {
      cercano = base_superior;
    }
    if (base_inferior and (not cercano or base_inferior->lambda < cercano->lambda)) {
      cercano = base_inferior;
    }
    return cercano;
  }

  std::optional<Intersection> render::Cylinder::intersect_lateral(ray const & r) const {
    vector const rc = vector::sub(r.origin, center);  // Vector desde el center al origen del rayo

    vector const dr_perpendicular = vector::sub(
        r.direction,
        vector::muld(ejes, vector::dotp(r.direction, ejes)));  // Componente perpendicular dr
    vector const rc_perpendicular =
        vector::sub(rc, vector::muld(ejes, vector::dotp(rc, ejes)));  // Componente perpendicular rc
    double const a = vector::dotp(dr_perpendicular, dr_perpendicular);
    double const b = 2.0 * vector::dotp(dr_perpendicular, rc_perpendicular);
    double const c = vector::dotp(rc_perpendicular, rc_perpendicular) - radius * radius;

    auto lambda_opt = ecuacion_cuadratica(a, b, c);
    if (not lambda_opt.has_value()) {
      return std::nullopt;  // no hay intersección
    }
    double const lambda = *lambda_opt;

    vector const punto_interseccion =
        vector::add(r.origin, vector::muld(r.direction, lambda));  // vector punto interseccion
    double const altura_interseccion =
        vector::dotp(vector::sub(punto_interseccion, center), ejes);  // altura desde el center
    if (altura_interseccion > altura / 2.0 or altura_interseccion < -altura / 2.0) {
      return std::nullopt;  // fuera de los limites del cilindro
    }
    vector normal = vector::sub(punto_interseccion, center);
    normal        = vector::sub(normal, vector::muld(ejes, vector::dotp(normal, ejes)));

    if (vector::dotp(r.direction, normal) > 1e-6) {
      normal = vector::muld(normal, -1.0);  // invertir normal si el rayo entra
    }
    return Intersection{punto_interseccion, normal, lambda, material_name};
  }

  std::optional<Intersection> render::Cylinder::intersect_base(ray const & r, vector const & P,
                                                               vector const & n) const {
    double const denominador = vector::dotp(r.direction, n);
    if (std::fabs(denominador) < 1e-6) {
      return std::nullopt;  // rayo paralelo a la base
    }
    double const numerador = vector::dotp(vector::sub(P, r.origin), n);
    double const lambda    = numerador / denominador;
    if (lambda < 1e-6) {
      return std::nullopt;  // detras/se ignora
    }
    vector const punto_interseccion =
        vector::add(r.origin, vector::muld(r.direction, lambda));  // vector punto interseccion
    if (vector::sub(punto_interseccion, P).magnitude() <= radius) {
      vector normal_base = n;
      if (vector::dotp(r.direction, normal_base) > 0.0) {
        // hacia adentro(positivo), se cambia signo
        normal_base = vector::muld(normal_base, -1.0);
        normal_base = vector::normalize(normal_base);
      }
      return Intersection{punto_interseccion, normal_base, lambda, material_name};
    }
    return std::nullopt;  // fuera del radio
  }

  std::optional<double> render::Cylinder::ecuacion_cuadratica(double a, double b, double c) {
    double const discriminante = b * b - 4 * a * c;
    if (discriminante < 1e-8) {
      return std::nullopt;  // no hay solución real
    }
    double const raiz = std::sqrt(discriminante);
    double lambda1    = (-b - raiz) / (2.0 * a);
    double lambda2    = (-b + raiz) / (2.0 * a);
    if (lambda1 >= 1e-6 and lambda2 >= 1 - 1e-6) {
      return std::min(lambda1, lambda2);
    }
    if (lambda1 >= 1e-6) {
      return lambda1;
    }
    if (lambda2 >= 1e-6) {
      return lambda2;
    }
    return std::nullopt;  // ambas soluciones son negativas
  }

}  // namespace render
