#include <gtest/gtest.h>
#include <optional>
#include <ray.hpp>
#include <sphere.hpp>
#include <string>
#include <vector.hpp>

// Apunta hacia el centro de la esfera
TEST(test_sphere, RayoGolpeaCentro) {
  render::vector const center{0.0, 0.0, 0.0};
  double const radius = 1.0;
  render::Sphere const s(center, radius, "test_material");

  render::ray r;
  r.origin    = render::vector{0.0, 0.0, -5.0};
  r.direction = render::vector{0.0, 0.0, 1.0};

  auto res = s.collision(r);
  if (!res) {
    FAIL() << "Expected intersection but got none.";
  }
  auto const & hit = *res;
  EXPECT_NEAR(hit.lambda, 4.0, 1e-9);
  EXPECT_NEAR(hit.punto_interseccion.x, 0.0, 1e-9);
  EXPECT_NEAR(hit.punto_interseccion.y, 0.0, 1e-9);
  EXPECT_NEAR(hit.punto_interseccion.z, -1.0, 1e-9);
  EXPECT_EQ(hit.nombre_material, "test_material");
}

// Apunta hacia un punto fuera de la esfera
TEST(test_sphere, RayoNoGolpea) {
  render::vector const center{0.0, 0.0, 0.0};
  double const radius = 1.0;
  render::Sphere const s(center, radius, "test_material");

  render::ray r;
  r.origin    = render::vector{0.0, 5.0, -5.0};
  r.direction = render::vector{0.0, 0.0, 1.0};

  auto res = s.collision(r);
  EXPECT_FALSE(res.has_value());
}

// Rayo empieza dentro de la esfera
TEST(test_sphere, RayoEmpiezaDentro) {
  render::vector const center{0.0, 0.0, 0.0};
  double const radius = 1.0;
  render::Sphere const s(center, radius, "test_material");

  render::ray r;
  r.origin    = render::vector{0.0, 0.0, 0.0};  // dentro de la esfera
  r.direction = render::vector{0.0, 0.0, 1.0};

  auto res = s.collision(r);
  if (!res) {
    FAIL() << "Expected intersection but got none.";
  }
  auto const & hit = res.value();
  EXPECT_NEAR(hit.lambda, 1.0, 1e-9);
}
