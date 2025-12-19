#ifndef RENDER_CAMERA_HPP
#define RENDER_CAMERA_HPP

#include <config.hpp>
#include <random>
#include <ray.hpp>
#include <vector.hpp>

namespace render {

  // Cámara que genera rayos para cada píxel de la imagen
  class Camera {
  public:
    explicit Camera(Config const & config);
    int ancho_imagen;
    int alto_imagen;
    [[nodiscard]] ray generar_ray(int row, int col, double rand_x, double rand_y);

    std::mt19937_64 & get_rng() { return rng; }

  private:
    vector posicion;
    vector destino;
    vector norte;
    double campo_de_vision;

    vector origen;
    vector horizontal;
    vector vertical;
    vector delta_x;
    vector delta_y;

    std::mt19937_64 rng;

    void ventana_proyeccion();
  };

}  // namespace render

#endif
