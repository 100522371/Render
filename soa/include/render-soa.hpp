#ifndef RENDER_SOA_HPP
#define RENDER_SOA_HPP

#include <camera.hpp>
#include <config.hpp>
#include <image_soa.hpp>
#include <random>
#include <scene.hpp>
#include <string>
#include <vector.hpp>

namespace render {

  class ray;

  struct RenderContext {
    Scene const * escena;
    Config const * config;
    std::mt19937_64 ray_rng;
    std::mt19937_64 material_rng;

    RenderContext(Scene const & escena_, Config const & config_, std::mt19937_64 ray_rng_,
                  std::mt19937_64 material_rng_)
        : escena(&escena_), config(&config_), ray_rng(ray_rng_), material_rng(material_rng_) { }
  };

  ImageSOA render_image_soa(Scene const & escena, Config const & config, Camera & cam,
                            ImageSOA & img, std::string const & partitioner_type = "auto",
                            int grain_size = 0);
  vector soa_calcular_color(ray const & r, int profundidad, RenderContext & render);
  vector soa_calcular_fondo(ray const & r, Config const & config);

}  // namespace render

#endif
