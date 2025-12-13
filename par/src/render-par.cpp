#include "render-par.hpp"
#include <algorithm>
#include <atomic>
#include <camera.hpp>
#include <cmath>
#include <config.hpp>
#include <cstdint>
#include <functional>
#include <image_par.hpp>
#include <intersection.hpp>
#include <material.hpp>
#include <optional>
#include <pixel.hpp>
#include <random>
#include <ray.hpp>
#include <scene.hpp>
#include <string>
#include <vector.hpp>

// includes de TBB
#include <tbb/blocked_range2d.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>
#include <tbb/task_arena.h>

namespace render {

  // Estructura para almacenar los generadores de números aleatorios por hilo
  struct ThreadLocalRNG {
    std::mt19937_64 ray_rng;
    std::mt19937_64 material_rng;
  };

  // Acepta parametros para el tipo de partitioner y el tamaño del grano
  ImageSOA render_image_soa(Scene const & escena, Config const & config, Camera & camara,
                            ImageSOA & img, std::string const & partitioner_type, int grain_size) {
    // determinamos el nº de hilos a usar
    int max_threads = tbb::this_task_arena::max_concurrency();
    if (max_threads <= 0) {
      max_threads = 32;  // fallback
    }

    std::vector<std::uint64_t> seeds_ray(static_cast<std::size_t>(max_threads));
    std::vector<std::uint64_t> seeds_material(static_cast<std::size_t>(max_threads));

    // generador maestro para semillas
    std::mt19937_64 master_ray_gen(config.ray_rng_seed);
    std::mt19937_64 master_mat_gen(config.material_rng_seed);
    // Llenamos los vectores con números aleatorios derivados del maestro
    // Usamos std::generate. std::ref asegura que el estado del maestro avance.
    std::ranges::generate(seeds_ray.begin(), seeds_ray.end(), std::ref(master_ray_gen));
    std::ranges::generate(seeds_material.begin(), seeds_material.end(), std::ref(master_mat_gen));
    // 3. Configurar TLS
    tbb::enumerable_thread_specific<ThreadLocalRNG> tls_rngs([&]() {
      static std::atomic<std::size_t> counter{0};
      std::size_t thread_id = counter++;

      // Usamos el ID del hilo para escoger una semilla pre-generada del vector
      std::size_t index = thread_id % seeds_ray.size();

      return ThreadLocalRNG{std::mt19937_64(seeds_ray[index]),
                            std::mt19937_64(seeds_material[index])};
    });

    tbb::blocked_range2d<int> rango =
        (grain_size > 0) ? tbb::blocked_range2d<int>(
                               0, camara.alto_imagen, static_cast<std::size_t>(grain_size), 0,
                               camara.ancho_imagen, static_cast<std::size_t>(grain_size))
                         : tbb::blocked_range2d<int>(0, camara.alto_imagen, 0, camara.ancho_imagen);

    // lambda que ejecutará cada hilo
    auto loop_body = [&](tbb::blocked_range2d<int> const & r) {
      ThreadLocalRNG & local_rngs = tls_rngs.local();
      for (int fila = r.rows().begin(); fila != r.rows().end(); ++fila) {
        for (int col = r.cols().begin(); col != r.cols().end(); ++col) {
          // pixel_id unico por pixel

          RenderContext ctx{escena, config, local_rngs.ray_rng, local_rngs.material_rng};

          render::vector acumulado{0.0, 0.0, 0.0};
          // generamos la distribucion uniforme para el anti-aliasing
          std::uniform_real_distribution<double> dist(-0.5, 0.5);

          for (int s = 0; s < config.samples_per_pixel; ++s) {
            // usamos ctx.ray_rng para generar los offsets aleatorios
            double u_offset = dist(*ctx.ray_rng);
            double v_offset = dist(*ctx.ray_rng);

            ray const rayo = camara.generar_ray(fila, col, u_offset, v_offset);
            // pasamos el contexto render al calcular_color
            acumulado =
                render::vector::add(acumulado, soa_calcular_color(rayo, config.max_depth, ctx));
          }
          // promedio
          acumulado =
              render::vector::divd(acumulado, static_cast<double>(config.samples_per_pixel));
          // corrección gamma
          acumulado = render::vector{std::pow(acumulado.x, 1.0 / config.gamma),
                                     std::pow(acumulado.y, 1.0 / config.gamma),
                                     std::pow(acumulado.z, 1.0 / config.gamma)};

          int const r = static_cast<int>(255.99 * std::clamp(acumulado.x, 0.0, 1.0));
          int const g = static_cast<int>(255.99 * std::clamp(acumulado.y, 0.0, 1.0));
          int const b = static_cast<int>(255.99 * std::clamp(acumulado.z, 0.0, 1.0));

          img.set_pixel(col, fila, Pixel{r, g, b});
        }
      }
    };

    // Selección de particionador
    if (partitioner_type == "simple") {
      tbb::parallel_for(rango, loop_body, tbb::simple_partitioner());
    } else if (partitioner_type == "static") {
      tbb::parallel_for(rango, loop_body, tbb::static_partitioner());
    } else {
      tbb::parallel_for(rango, loop_body, tbb::auto_partitioner());
    }

    return img;
  }

  vector soa_calcular_color(ray const & r, int profundidad, RenderContext & render) {
    if (profundidad <= 0) {
      return vector{0.0, 0.0, 0.0};
    }
    std::optional<Intersection> inter_mas_cercana;
    // Cambio: ya existe una función Scene::intersect que hace esto
    inter_mas_cercana = render.escena->intersect(r);
    if (!inter_mas_cercana.has_value()) {
      return soa_calcular_fondo(r, *render.config);
    }
    Material const * mat = render.escena->materialByName(inter_mas_cercana->nombre_material);
    if (mat == nullptr) {
      return vector{0.0, 0.0, 0.0};
    }
    // Generador aleatorio local
    // Obtener dirección reflejada y reflectancia según el tipo de material
    auto [scattered_dir, reflectance] =
        mat->scatter(r.direction, inter_mas_cercana->vector_normal, *render.material_rng);
    // Crear nuevo rayo desde el punto de intersección
    ray const scattered_ray(inter_mas_cercana->punto_interseccion, scattered_dir);
    // Recursividad: calcular color reflejado
    vector const color_rebote = soa_calcular_color(scattered_ray, profundidad - 1, render);
    // Atenuar el color reflejado con la reflectancia del material
    return vector::mul(reflectance, color_rebote);
  }

  vector soa_calcular_fondo(ray const & r, Config const & config) {
    render::vector const unit_dir = render::vector::normalize(r.direction);
    double const m                = (unit_dir.y + 1.0) * 0.5;

    return render::vector::add(render::vector::muld(config.background_light_color, 1.0 - m),
                               render::vector::muld(config.background_dark_color, m));
  }

}  // namespace render
