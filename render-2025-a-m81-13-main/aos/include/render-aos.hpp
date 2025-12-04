#ifndef RENDER_AOS_HPP
#define RENDER_AOS_HPP

#include <camera.hpp>
#include <config.hpp>
#include <image_aos.hpp>
#include <random>
#include <scene.hpp>
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

  ImageAOS render_image_aos(Scene const & escena, Config const & config, Camera & cam,
                            ImageAOS & img);
  vector aos_calcular_color(ray const & r, int profundidad, RenderContext & render);
  vector aos_calcular_fondo(ray const & r, Config const & config);

}  // namespace render

#endif
