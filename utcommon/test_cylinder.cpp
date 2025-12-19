#include "../include/cylinder.hpp"
#include "../include/ray.hpp"
#include "../include/vector.hpp"
#include <gtest/gtest.h>

using namespace render;

// Pruebas unitarias para la clase Cylinder
TEST(test_cylinder, RayoAtraviesaLateralmente) {
  // Cilindro centrado en el origen, eje vertical Y
  Cylinder const cyl(vector{0, 0, 0}, 1.0, vector{0, 2.0, 0}, "metal");

  // Rayo que atraviesa lateralmente por el eje X
  ray const r(vector{-3, 0, 0}, vector{1, 0, 0});
  auto interseccion = cyl.collision(r);
  if (!interseccion) {
    FAIL() << "Expected collision but got none.";
  }
  auto const & inter = *interseccion;
  EXPECT_NEAR(inter.lambda, 2.0, 1e-6);  // distancia al punto más cercano
  EXPECT_NEAR(inter.punto_interseccion.x, -1.0, 1e-6);
  EXPECT_NEAR(inter.punto_interseccion.y, 0.0, 1e-6);
  EXPECT_NEAR(inter.punto_interseccion.z, 0.0, 1e-6);
  EXPECT_NEAR(inter.vector_normal.x, -1.0, 1e-6);
}

// Rayo que no intersecta el cilindro
TEST(test_cylinder, RayoNoAtraviesa) {
  Cylinder const cyl(vector{0, 0, 0}, 1.0, vector{0, 2.0, 0}, "metal");
  ray const r(vector{-3, 3, 0}, vector{1, 0, 0});  // pasa por arriba del cilindro
  auto inter = cyl.collision(r);
  EXPECT_FALSE(inter.has_value());
}

// Rayo que intersecta la base superior del cilindro
TEST(test_cylinder, RayoPasaPorBaseSuperior) {
  Cylinder const cyl(vector{0, 0, 0}, 1.0, vector{0, 2.0, 0}, "metal");
  ray const r(vector{0, 3, 0}, vector{0, -1, 0});  // hacia abajo
  auto interseccion = cyl.collision(r);
  if (!interseccion) {
    FAIL() << "Expected collision but got none.";
  }
  auto const & inter = *interseccion;
  EXPECT_NEAR(inter.punto_interseccion.y, 1.0, 1e-6);  // base superior en y = +1
  EXPECT_NEAR(inter.vector_normal.y, 1.0, 1e-6);
}

// Rayo que intersecta la base inferior del cilindro
TEST(test_cylinder, RayoPasaPorBaseInferior) {
  Cylinder const cyl(vector{0, 0, 0}, 1.0, vector{0, 2.0, 0}, "metal");
  ray const r(vector{0, -3, 0}, vector{0, 1, 0});  // hacia arriba
  auto interseccion = cyl.collision(r);
  if (!interseccion) {
    FAIL() << "Expected collision but got none.";
  }
  auto const & inter = *interseccion;
  EXPECT_NEAR(inter.punto_interseccion.y, -1.0, 1e-6);  // base inferior en y = -1
  EXPECT_NEAR(inter.vector_normal.y, -1.0, 1e-6);
}

// Rayo que empieza dentro del cilindro y sale por el lateral
TEST(test_cylinder, RayoEmpiezaDentroDelCilindro) {
  Cylinder const cyl(vector{0, 0, 0}, 1.0, vector{0, 2.0, 0}, "metal");
  ray const r(vector{0, 0, 0}, vector{1, 0, 0});
  auto interseccion = cyl.collision(r);
  if (!interseccion) {
    FAIL() << "Expected collision but got none.";
  }
  auto const & inter = *interseccion;
  EXPECT_NEAR(inter.punto_interseccion.x, 1.0, 1e-6);
  EXPECT_NEAR(inter.vector_normal.x, -1.0, 1e-6);
}

// Rayo paralelo al eje del cilindro que no impacta
TEST(test_cylinder, RayoParaleloAlEjeSinImpacto) {
  Cylinder const cyl(vector{0, 0, 0}, 1.0, vector{0, 2.0, 0}, "metal");
  ray const r(vector{2, -3, 0}, vector{0, 1, 0});  // paralelo al eje, fuera del radio
  auto inter = cyl.collision(r);
  EXPECT_FALSE(inter.has_value());
}
