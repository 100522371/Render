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
    vector const dr_per = vector::sub(
        r.direction, vector::muld(ejes, vector::dotp(r.direction, ejes)));  // perpendicular dr
    vector const rc_per =
        vector::sub(rc, vector::muld(ejes, vector::dotp(rc, ejes)));  // perpendicular rc
    double const a = vector::dotp(dr_per, dr_per);
    double const b = 2.0 * vector::dotp(dr_per, rc_per);
    double const c = vector::dotp(rc_per, rc_per) - radius * radius;
    if (std::abs(a) < 1e-8) {  // Protección para rayos paralelos (a muy cercano a 0)
      return std::nullopt;
    }
    double const discriminante = b * b - 4.0 * a * c;
    if (discriminante < 1e-8) {  // Igual que en Sphere: si discriminante negativo, fuera.
      return std::nullopt;
    }
    double const raiz    = std::sqrt(discriminante);
    double const lambda1 = (-b - raiz) / (2.0 * a), lambda2 = (-b + raiz) / (2.0 * a);
    double const epsilon = 1e-3;
    if (lambda1 > epsilon) {  // 1. Comprobamos la primera intersección (entrada)
      vector const punto_interseccion = vector::add(r.origin, vector::muld(r.direction, lambda1));
      double const altura_int         = vector::dotp(vector::sub(punto_interseccion, center), ejes);
      if (altura_int <= altura / 2.0 and altura_int >= -altura / 2.0) {
        vector normal = vector::sub(punto_interseccion, center);
        normal        = vector::sub(normal, vector::muld(ejes, vector::dotp(normal, ejes)));
        normal        = vector::normalize(normal);
        return Intersection{punto_interseccion, normal, lambda1, material_name};
      }
    }
    if (lambda2 > epsilon) {  // 2. Si la primera no valía, probamos la segunda (salida)
      vector const punto_interseccion = vector::add(r.origin, vector::muld(r.direction, lambda2));
      double const altura_int         = vector::dotp(vector::sub(punto_interseccion, center), ejes);
      if (altura_int <= altura / 2.0 and altura_int >= -altura / 2.0) {
        vector normal = vector::sub(punto_interseccion, center);
        normal        = vector::sub(normal, vector::muld(ejes, vector::dotp(normal, ejes)));
        normal        = vector::normalize(normal);
        return Intersection{punto_interseccion, normal, lambda2, material_name};
      }
    }
    return std::nullopt;
  }

  std::optional<Intersection> render::Cylinder::intersect_base(ray const & r, vector const & P,
                                                               vector const & n) const {
    double const denominador = vector::dotp(r.direction, n);
    if (std::fabs(denominador) < 1e-6) {
      return std::nullopt;  // rayo paralelo a la base
    }
    double const numerador = vector::dotp(vector::sub(P, r.origin), n);
    double const lambda    = numerador / denominador;
    if (lambda < 1e-3) {
      return std::nullopt;  // detras/se ignora
    }
    vector const punto_interseccion =
        vector::add(r.origin, vector::muld(r.direction, lambda));  // vector punto interseccion
    if (vector::sub(punto_interseccion, P).magnitude() <= radius) {
      vector const normal_base = vector::normalize(n);
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
    if (lambda1 > 1e-3) {
      return lambda1;  // primera solución válida
    }
    if (lambda2 > 1e-3) {
      return lambda2;  // segunda solución válida
    }
    return std::nullopt;  // ambas soluciones son negativas
  }

}  // namespace render
