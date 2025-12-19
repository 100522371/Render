#include <camera.hpp>
#include <cmath>
#include <config.hpp>
#include <gtest/gtest.h>
#include <ray.hpp>
#include <vector.hpp>

using namespace render;

// Pruebas unitarias para la clase Camera
TEST(test_camera, RayoCentralApuntaAlDestino) {
  Config cfg;
  cfg.camera_position = {0.0, 0.0, -10.0};
  cfg.camera_target   = {0.0, 0.0, 0.0};
  cfg.camera_north    = {0.0, 1.0, 0.0};
  cfg.field_of_view   = 90.0;
  cfg.image_width     = 100;
  cfg.aspect_w        = 16;
  cfg.aspect_h        = 9;
  cfg.ray_rng_seed    = 42;

  Camera cam(cfg);

  int const row = cfg.image_width * cfg.aspect_h / cfg.aspect_w / 2;
  int const col = cfg.image_width / 2;

  // Generamos el rayo central sin jitter
  auto ray = cam.generar_ray(row, col, 0.0, 0.0);

  // Vector dirección esperado hacia el destino
  vector const expected_dir =
      vector::normalize(vector::sub(cfg.camera_target, cfg.camera_position));

  // Normalizamos la dirección generada
  vector const dir_norm = vector::normalize(ray.direction);

  // Verificamos que el ángulo entre ambos vectores sea muy pequeño
  double const cos_theta = vector::dotp(dir_norm, expected_dir);
  EXPECT_NEAR(cos_theta, 1.0, 1e-3);  // tolerancia pequeña para permitir pequeñas desviaciones
}

// Verifica que el campo de visión afecta la dirección de los rayos generados
TEST(test_camera, CampoVisionAfectaRayos) {
  Config cfg;
  cfg.camera_position = {0.0, 0.0, -10.0};
  cfg.camera_target   = {0.0, 0.0, 0.0};
  cfg.camera_north    = {0.0, 1.0, 0.0};
  cfg.field_of_view   = 45.0;
  cfg.image_width     = 100;
  cfg.aspect_w        = 1;
  cfg.aspect_h        = 1;
  cfg.ray_rng_seed    = 1'234;

  Camera cam(cfg);

  // Rayo en esquina superior izquierda y otro en inferior derecha
  auto ray_tl = cam.generar_ray(0, 0, 0.0, 0.0);
  auto ray_br = cam.generar_ray(cfg.image_width - 1, cfg.image_width - 1, 0.0, 0.0);

  // Los rayos no deben ser paralelos (el campo de visión los separa)
  vector const d1  = vector::normalize(ray_tl.direction);
  vector const d2  = vector::normalize(ray_br.direction);
  double const dot = vector::dotp(d1, d2);
  EXPECT_LT(dot, 1.0);  // no paralelos
}

// Verifica que el jitter aleatorio cambie la dirección del rayo generado
TEST(test_camera, JitterAleatorioCambiaRayo) {
  Config cfg;
  cfg.camera_position = {0.0, 0.0, -10.0};
  cfg.camera_target   = {0.0, 0.0, 0.0};
  cfg.camera_north    = {0.0, 1.0, 0.0};
  cfg.field_of_view   = 90.0;
  cfg.image_width     = 50;
  cfg.aspect_w        = 1;
  cfg.aspect_h        = 1;
  cfg.ray_rng_seed    = 123;

  Camera cam(cfg);

  auto ray1 = cam.generar_ray(25, 25, 0.0, 0.0);
  auto ray2 = cam.generar_ray(25, 25, 0.0, 0.0);

  // Los rayos deben tener direcciones distintas (por el jitter)
  double const diff = std::abs(ray1.direction.x - ray2.direction.x) +
                      std::abs(ray1.direction.y - ray2.direction.y) +
                      std::abs(ray1.direction.z - ray2.direction.z);
  EXPECT_GT(diff, 1e-6);
}
