#include <algorithm>
#include <atomic>
#include <camera.hpp>
#include <cmath>
#include <config.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <image_par.hpp>
#include <intersection.hpp>
#include <material.hpp>
#include <optional>
#include <pixel.hpp>
#include <random>
#include <ray.hpp>
#include <render-par.hpp>
#include <scene.hpp>
#include <utility>
#include <vector.hpp>
#include <vector>

// includes de TBB
#include <oneapi/tbb/blocked_range2d.h>
#include <oneapi/tbb/enumerable_thread_specific.h>
#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/partitioner.h>

namespace render {

  // Estructura para almacenar los generadores de números aleatorios por hilo
  struct ThreadLocalRNG {
    std::mt19937_64 ray_rng;
    std::mt19937_64 material_rng;
  };

  // funciones auxiliares
  std::pair<std::vector<std::uint64_t>, std::vector<std::uint64_t>> generar_semillas_soa(
      int hilos, Config const & config) {
    std::vector<std::uint64_t> seeds_ray(static_cast<std::size_t>(hilos));
    std::vector<std::uint64_t> seeds_material(static_cast<std::size_t>(hilos));
    std::mt19937_64 master_ray_gen(config.ray_rng_seed);
    std::mt19937_64 master_mat_gen(config.material_rng_seed);
    std::ranges::generate(seeds_ray.begin(), seeds_ray.end(), std::ref(master_ray_gen));
    std::ranges::generate(seeds_material.begin(), seeds_material.end(), std::ref(master_mat_gen));
    return {seeds_ray, seeds_material};
  }

  // Estructura para pasar el contexto de renderizado a las funciones
  void calcular_pixel_soa(int fila, int col, RenderContext & ctx) {
    render::vector acumulado{0.0, 0.0, 0.0};
    // generamos la distribucion uniforme para el anti-aliasing
    std::uniform_real_distribution<double> dist(-0.5, 0.5);
    Config const & config = *ctx.config;
    for (int s = 0; s < config.samples_per_pixel; ++s) {
      // usamos ctx.ray_rng para generar los offsets aleatorios
      double const u_offset = dist(*ctx.ray_rng);
      double const v_offset = dist(*ctx.ray_rng);

      ray const rayo = ctx.camara->generar_ray(fila, col, u_offset, v_offset);
      // pasamos el contexto render al calcular_color
      acumulado = render::vector::add(acumulado, soa_calcular_color(rayo, config.max_depth, ctx));
    }
    // promedio
    acumulado = render::vector::divd(acumulado, static_cast<double>(config.samples_per_pixel));
    // corrección gamma
    acumulado = render::vector{std::pow(acumulado.x, 1.0 / config.gamma),
                               std::pow(acumulado.y, 1.0 / config.gamma),
                               std::pow(acumulado.z, 1.0 / config.gamma)};

    int const r = static_cast<int>(255.99 * std::clamp(acumulado.x, 0.0, 1.0));
    int const g = static_cast<int>(255.99 * std::clamp(acumulado.y, 0.0, 1.0));
    int const b = static_cast<int>(255.99 * std::clamp(acumulado.z, 0.0, 1.0));

    ctx.img->set_pixel(col, fila, Pixel{r, g, b});
  }

  // Función principal de renderizado en paralelo usando TBB y SOA
  ImageSOA render_image_soa(Scene const & escena, Config const & config, Camera & camara,
                            ImageSOA & img) {
    constexpr int max_threads = 64;
    constexpr int grain_size  = 1;
    // 1. Configurar el número máximo de hilos
    oneapi::tbb::global_control const control(oneapi::tbb::global_control::max_allowed_parallelism,
                                              static_cast<std::size_t>(max_threads));
    // generacion de semillas
    auto [seeds_ray, seeds_material] = generar_semillas_soa(max_threads, config);
    // 3. Configurar TLS
    oneapi::tbb::enumerable_thread_specific<ThreadLocalRNG> tls_rngs([&]() {
      static std::atomic<std::size_t> counter{0};
      std::size_t const thread_id = counter++;
      std::size_t const index     = thread_id % seeds_ray.size();
      return ThreadLocalRNG{std::mt19937_64(seeds_ray[index]),
                            std::mt19937_64(seeds_material[index])};
    });
    oneapi::tbb::blocked_range2d<int> const rango =
        (grain_size > 0)
            ? oneapi::tbb::blocked_range2d<int>(
                  0, camara.alto_imagen, static_cast<std::size_t>(grain_size), 0,
                  camara.ancho_imagen, static_cast<std::size_t>(grain_size))
            : oneapi::tbb::blocked_range2d<int>(0, camara.alto_imagen, 0, camara.ancho_imagen);
    // lambda que ejecutará cada hilo
    auto loop_body = [&](oneapi::tbb::blocked_range2d<int> const & r) {
      ThreadLocalRNG & local_rngs = tls_rngs.local();
      RenderContext ctx{escena, config, camara, img, local_rngs.ray_rng, local_rngs.material_rng};
      for (int fila = r.rows().begin(); fila != r.rows().end(); ++fila) {
        for (int col = r.cols().begin(); col != r.cols().end(); ++col) {
          // pixel_id unico por pixel
          calcular_pixel_soa(fila, col, ctx);
        }
      }
    };
    // ejecucion con el particionador optimo
    oneapi::tbb::parallel_for(rango, loop_body, oneapi::tbb::simple_partitioner());
    return img;
  }

  // Función recursiva para calcular el color de un rayo usando SOA
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

  // Función para calcular el color de fondo basado en la dirección del rayo
  vector soa_calcular_fondo(ray const & r, Config const & config) {
    render::vector const unit_dir = render::vector::normalize(r.direction);
    double const m                = (unit_dir.y + 1.0) * 0.5;

    return render::vector::add(render::vector::muld(config.background_light_color, 1.0 - m),
                               render::vector::muld(config.background_dark_color, m));
  }

}  // namespace render
