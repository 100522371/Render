#include <algorithm>
#include <camera.hpp>
#include <cmath>
#include <config.hpp>
#include <image_aos.hpp>
#include <intersection.hpp>
#include <material.hpp>
#include <optional>
#include <pixel.hpp>
#include <random>
#include <ray.hpp>
#include <render-aos.hpp>
#include <scene.hpp>
#include <vector.hpp>

// Includes TBB
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>
#include <tbb/partitioner.h>

namespace render {

  // Acepta parametros para el tipo de partitioner y el tamaño del grano
  ImageAOS render_image_aos(Scene const & escena, Config const & config, Camera & camara,
                            ImageAOS & img, std::string const & partitioner_type, int grain_size) {
    // El constructor de blocked_range2d admite el grano para filas y columnas.
    // blocked_range2d(row_begin, row_end, row_grain, col_begin, col_end, col_grain)
    // Si grain_size es 0 o negativo, dejamos que TBB decida (usamos constructor simple)
    // Usamos static_cast para evitar errores de conversión de signo
    tbb::blocked_range2d<int> rango =
        (grain_size > 0) ? tbb::blocked_range2d<int>(
                               0, camara.alto_imagen, static_cast<std::size_t>(grain_size), 0,
                               camara.ancho_imagen, static_cast<std::size_t>(grain_size))
                         : tbb::blocked_range2d<int>(0, camara.alto_imagen, 0, camara.ancho_imagen);

    // Lambda que ejecutará cada hilo
    auto loop_body = [&](tbb::blocked_range2d<int> const & r) {
      for (int fila = r.rows().begin(); fila != r.rows().end(); ++fila) {
        for (int col = r.cols().begin(); col != r.cols().end(); ++col) {
          // Calculamos pixel_id unico por pixel para generar las semillas
          std::size_t pixel_id =
              static_cast<std::size_t>(fila) * static_cast<std::size_t>(camara.ancho_imagen) +
              static_cast<std::size_t>(col);

          // Creamos un RenderContex con semillas unicas por pixel
          // evita condiciones de carrera en los generadores de numeros aleatorios
          RenderContext ctx{
            escena, config,
            std::mt19937_64(config.ray_rng_seed + pixel_id),      // Semilla única por píxel
            std::mt19937_64(config.material_rng_seed + pixel_id)  // Semilla única por píxel
          };

          render::vector acumulado{0.0, 0.0, 0.0};

          // Necesitamos una distribución uniforme para el jittering
          std::uniform_real_distribution<double> dist(0.0, 1.0);

          for (int s = 0; s < config.samples_per_pixel; ++s) {
            // usamos ctx.ray_rng para generar los offsets aleatorios
            double u_offset = dist(ctx.ray_rng);  // Usamos el generador local
            double v_offset = dist(ctx.ray_rng);

            // Pasamos los offsets a generar_ray
            ray const rayo = camara.generar_ray(fila, col, u_offset, v_offset);
            acumulado =
                render::vector::add(acumulado, aos_calcular_color(rayo, config.max_depth, ctx));
          }

          // Promediamos el color acumulado
          acumulado =
              render::vector::divd(acumulado, static_cast<double>(config.samples_per_pixel));

          acumulado = render::vector{std::pow(acumulado.x, 1.0 / config.gamma),
                                     std::pow(acumulado.y, 1.0 / config.gamma),
                                     std::pow(acumulado.z, 1.0 / config.gamma)};

          int const r_val = static_cast<int>(255.99 * std::clamp(acumulado.x, 0.0, 1.0));
          int const g_val = static_cast<int>(255.99 * std::clamp(acumulado.y, 0.0, 1.0));
          int const b_val = static_cast<int>(255.99 * std::clamp(acumulado.z, 0.0, 1.0));

          // ImageAOS, set_pixel es thread-safe porque cada hilo escribe en píxeles distintos
          img.set_pixel(col, fila, Pixel{r_val, g_val, b_val});
        }
      }
    };

    // Elegimos uno de los 3 partitioners segun el parametro recibido
    if (partitioner_type == "simple") {
      tbb::parallel_for(rango, loop_body, tbb::simple_partitioner());
    } else if (partitioner_type == "static") {
      tbb::parallel_for(rango, loop_body, tbb::static_partitioner());
    } else {
      // Por defecto "auto" (o si el string es "auto" o cualquier otra cosa)
      tbb::parallel_for(rango, loop_body, tbb::auto_partitioner());
    }

    return img;
  }

  vector aos_calcular_color(ray const & r, int profundidad, RenderContext & render) {
    if (profundidad <= 0) {
      return vector{0.0, 0.0, 0.0};
    }
    std::optional<Intersection> inter_mas_cercana;
    // Cambio: ya existe una función Scene::intersect que hace esto
    inter_mas_cercana = render.escena->intersect(r);
    if (!inter_mas_cercana.has_value()) {
      return aos_calcular_fondo(r, *render.config);
    }
    Material const * mat = render.escena->materialByName(inter_mas_cercana->nombre_material);
    if (mat == nullptr) {
      return vector{0.0, 0.0, 0.0};
    }
    // Generador aleatorio local
    // Obtener dirección reflejada y reflectancia según el tipo de material
    auto [scattered_dir, reflectance] =
        mat->scatter(r.direction, inter_mas_cercana->vector_normal, render.material_rng);
    // Crear nuevo rayo desde el punto de intersección
    ray const scattered_ray(inter_mas_cercana->punto_interseccion, scattered_dir);
    // Recursividad: calcular color reflejado
    vector const color_rebote = aos_calcular_color(scattered_ray, profundidad - 1, render);
    // Atenuar el color reflejado con la reflectancia del material
    return vector::mul(reflectance, color_rebote);
  }

  vector aos_calcular_fondo(ray const & r, Config const & config) {
    render::vector const unit_dir = render::vector::normalize(r.direction);
    double const m                = (unit_dir.y + 1.0) * 0.5;

    return render::vector::add(render::vector::muld(config.background_light_color, 1.0 - m),
                               render::vector::muld(config.background_dark_color, m));
  }

}  // namespace render
