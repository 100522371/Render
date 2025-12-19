#ifndef RENDER_PAR_HPP
#define RENDER_PAR_HPP

#include <camera.hpp>
#include <config.hpp>
#include <image_par.hpp>
#include <random>
#include <scene.hpp>
#include <vector.hpp>

namespace render {

  class ray;

  // Contexto compartido para el renderizado paralelo
  struct RenderContext {
    Scene const * escena;
    Config const * config;
    Camera * camara;
    ImageSOA * img;
    std::mt19937_64 * ray_rng;
    std::mt19937_64 * material_rng;

    RenderContext(Scene const & escena_, Config const & config_, Camera & camara_, ImageSOA & img_,
                  std::mt19937_64 & ray_rng_, std::mt19937_64 & material_rng_)
        : escena(&escena_), config(&config_), camara(&camara_), img(&img_), ray_rng(&ray_rng_),
          material_rng(&material_rng_) { }
  };

  // Renderiza la imagen usando el enfoque Structure of Arrays (SoA)
  ImageSOA render_image_soa(Scene const & escena, Config const & config, Camera & cam,
                            ImageSOA & img);
  std::pair<std::vector<std::uint64_t>, std::vector<std::uint64_t>> generar_semillas_soa(
      int hilos, Config const & config);

  void calcular_pixel_soa(int fila, int col, RenderContext & ctx);
  vector soa_calcular_color(ray const & r, int profundidad, RenderContext & render);
  vector soa_calcular_fondo(ray const & r, Config const & config);

}  // namespace render

#endif
