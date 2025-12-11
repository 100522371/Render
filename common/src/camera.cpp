#include <camera.hpp>
#include <cmath>
#include <config.hpp>
#include <numbers>
#include <random>
#include <ray.hpp>
#include <vector.hpp>

namespace render {

  Camera::Camera(Config const & config)
      : ancho_imagen{config.image_width},
        alto_imagen{static_cast<int>(
            config.image_width * config.aspect_h / static_cast<double>(config.aspect_w))},
        posicion{config.camera_position}, destino{config.camera_target}, norte{config.camera_north},
        campo_de_vision{config.field_of_view}, rng(config.ray_rng_seed) {
    ventana_proyeccion();
  }

  void Camera::ventana_proyeccion() {
    // Determinación del vector focal
    vector const vf = vector::sub(posicion, destino);

    // Determinación de la distancia focal
    double const df = vf.magnitude();

    // Determinación de la altura de la ventana de proyección
    double const alpha_radianes = (campo_de_vision * std::numbers::pi) / 180.0;
    double const hp             = 2.0 * df * std::tan(alpha_radianes / 2.0);

    // Determinación de la anchura de la ventana de proyección
    double const wp = hp * (ancho_imagen / static_cast<double>(alto_imagen));

    // Determinación de los vectores directores u y v
    vector const vf_normalizado = vector::normalize(vf);
    vector const u              = vector::normalize(vector::crossp(norte, vf_normalizado));
    vector const v              = vector::crossp(vf_normalizado, u);

    // Determinación de los vectores horizontal y vertical
    horizontal = vector::muld(u, wp);
    vertical   = vector::muld(vector::muld(v, -1.0), hp);

    // Determinación del origen de la ventana de proyección
    delta_x = vector::divd(horizontal, static_cast<double>(ancho_imagen));
    delta_y = vector::divd(vertical, static_cast<double>(alto_imagen));

    origen = vector::add(vector::sub(posicion, vf),  // P - vf
                         vector::sub(                // 0.5*(Δx+Δy) - 0.5*(ph+pv)
                             vector::muld(vector::add(delta_x, delta_y), 0.5),
                             vector::muld(vector::add(horizontal, vertical), 0.5)));
  }

  ray Camera::generar_ray(int row, int col, double rand_x, double rand_y) {
    // Posicion Q
    vector const Q =
        vector::add(origen, vector::add(vector::muld(delta_x, static_cast<double>(col) + rand_x),
                                        vector::muld(delta_y, static_cast<double>(row) + rand_y)));
    vector const dr = vector::sub(Q, posicion);  // Direccion del rayo
    return ray{posicion, dr};
  }

}  // namespace render
