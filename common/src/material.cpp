#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <material.hpp>
#include <random>
#include <string>
#include <utility>
#include <vector.hpp>

Matte::Matte(std::string name, render::vector reflectance)
    : Material(std::move(name)), reflectance(reflectance) { }

Metal::Metal(std::string name, render::vector reflectance, double fuzz)
    : Material(std::move(name)), reflectance(reflectance), fuzz(std::clamp(fuzz, 0.0, 1.0)) { }

Refractive::Refractive(std::string name, double ior) : Material(std::move(name)), ior(ior) { }

std::pair<render::vector, render::vector> Matte::scatter(
    [[maybe_unused]] render::vector const & in_dir, render::vector const & normal,
    std::mt19937_64 & rng) const {
  render::vector const unit_normal = render::vector::normalize(normal);

  // el rayo reflejado se genera en una dirección aleatoria
  std::uniform_real_distribution<double> ud(-1.0, 1.0);
  render::vector const rnd{ud(rng), ud(rng), ud(rng)};
  render::vector dir = render::vector::add(unit_normal, rnd);
  if (std::fabs(dir.x) < 1e-8 and std::fabs(dir.y) < 1e-8 and std::fabs(dir.z) < 1e-8) {
    // evitar vector demasiado pequeño
    dir = unit_normal;
  }

  return {render::vector::normalize(dir), reflectance};
}

std::pair<render::vector, render::vector> Metal::scatter(render::vector const & in_dir,
                                                         render::vector const & normal,
                                                         std::mt19937_64 & rng) const {
  render::vector const unit_normal = render::vector::normalize(normal);

  // reflexión inicial, dir = in_dir − 2(in_dir · normal) · normal
  render::vector const dir = render::vector::sub(
      in_dir, render::vector::muld(unit_normal, 2.0 * render::vector::dotp(in_dir, unit_normal)));
  // agregar fuzz
  std::uniform_real_distribution<double> ud(-fuzz, fuzz);
  render::vector const diff{ud(rng), ud(rng), ud(rng)};
  render::vector const res = render::vector::add(render::vector::normalize(dir), diff);

  return {render::vector::normalize(res), reflectance};
}

std::pair<render::vector, render::vector> Refractive::scatter(
    render::vector const & in_dir, render::vector const & normal,
    [[maybe_unused]] std::mt19937_64 & rng) const {
  render::vector const unit_normal = render::vector::normalize(normal);
  double const cos_theta =
      std::min(render::vector::dotp(render::vector::muld(in_dir, -1.0), unit_normal), 1.0);
  // max(0, 1-cos_theta^2) para evitar raíz cuadrada de número negativo
  double const sin_theta = std::sqrt(std::max(0.0, 1.0 - cos_theta * cos_theta));
  // si el producto escalar in_dir y normal es positivo, estamos fuera del objeto -> ior
  // si es negativo, estamos dentro -> 1/ior
  bool const front_face  = render::vector::dotp(in_dir, unit_normal) < 0;
  double const ref_ratio = front_face ? (1.0 / ior) : ior;
  if (ref_ratio * sin_theta > 1.0) {
    render::vector const refr = render::vector::sub(
        in_dir, render::vector::muld(unit_normal, 2.0 * render::vector::dotp(in_dir, unit_normal)));
    return {
      render::vector::normalize(refr), {1.0, 1.0, 1.0}
    };
  }  // else
  // u = ref_ratio (in_dir + (cosθ) normal)
  render::vector const u = render::vector::muld(
      render::vector::add(in_dir, render::vector::muld(unit_normal, cos_theta)), ref_ratio);
  // v = -(sqrt(abs(1 - ||u||^2))) normal
  double const u2           = u.magnitude() * u.magnitude();
  double const a            = std::abs(1.0 - u2);
  render::vector const v    = render::vector::muld(unit_normal, -std::sqrt(a));
  render::vector const refr = render::vector::add(u, v);
  return {
    render::vector::normalize(refr), {1.0, 1.0, 1.0}
  };
}
