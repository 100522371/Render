#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <random>
#include <string>
#include <utility>
#include <vector.hpp>

class Material {
public:
  std::string name;

  Material(Material const &)             = delete;
  Material & operator=(Material const &) = delete;
  Material(Material &&)                  = delete;
  Material & operator=(Material &&)      = delete;

  // Returns a pair (scattered_direction, reflectance)
  // in_dir es un vector unitario y el vector normal es la normal de la intersección
  virtual std::pair<render::vector, render::vector> scatter(render::vector const & in_dir,
                                                            render::vector const & normal,
                                                            std::mt19937_64 & rng) const = 0;

  virtual ~Material() = default;

protected:
  explicit Material(std::string name) : name(std::move(name)) { }  // Constructor base
};

class Matte : public Material {
  render::vector reflectance;

public:
  Matte(std::string name, render::vector reflectance);
  std::pair<render::vector, render::vector> scatter(render::vector const & in_dir,
                                                    render::vector const & normal,
                                                    std::mt19937_64 & rng) const override;
};

class Metal : public Material {
  render::vector reflectance;
  double fuzz;

public:
  Metal(std::string name, render::vector reflectance, double fuzz);
  std::pair<render::vector, render::vector> scatter(render::vector const & in_dir,
                                                    render::vector const & normal,
                                                    std::mt19937_64 & rng) const override;
};

class Refractive : public Material {
  double ior;

public:
  Refractive(std::string name, double ior);
  std::pair<render::vector, render::vector> scatter(render::vector const & in_dir,
                                                    render::vector const & normal,
                                                    std::mt19937_64 & rng) const override;
};

#endif
