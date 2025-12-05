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

namespace render {

  ImageAOS render_image_aos(Scene const & escena, Config const & config, Camera & camara,
                            ImageAOS & img) {
    RenderContext render{escena, config, std::mt19937_64(config.ray_rng_seed),
                         std::mt19937_64(config.material_rng_seed)};

    // filas -> altura (rows), columnas -> ancho (columns)
    for (int fila = 0; fila < camara.alto_imagen; ++fila) {
      for (int col = 0; col < camara.ancho_imagen; ++col) {
        render::vector acumulado{0.0, 0.0, 0.0};
        // muestreo por píxel
        for (int s = 0; s < config.samples_per_pixel; ++s) {
          ray const rayo = camara.generar_ray(fila, col, 0.0, 0.0);
          acumulado =
              render::vector::add(acumulado, aos_calcular_color(rayo, config.max_depth, render));
        }
        // promedio
        acumulado = render::vector::divd(acumulado, static_cast<double>(config.samples_per_pixel));
        // corrección gamma
        acumulado = render::vector{std::pow(acumulado.x, 1.0 / config.gamma),
                                   std::pow(acumulado.y, 1.0 / config.gamma),
                                   std::pow(acumulado.z, 1.0 / config.gamma)};
        // Clamp values to [0, 1]
        int const r = static_cast<int>(255.99 * std::clamp(acumulado.x, 0.0, 1.0));
        int const g = static_cast<int>(255.99 * std::clamp(acumulado.y, 0.0, 1.0));
        int const b = static_cast<int>(255.99 * std::clamp(acumulado.z, 0.0, 1.0));

        // ImageAOS::set_pixel expects (x=column, y=row)
        img.set_pixel(col, fila, Pixel{r, g, b});
      }
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
