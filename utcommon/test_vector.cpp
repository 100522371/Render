#include <gtest/gtest.h>
#include <stdexcept>
#include <vector.hpp>

//Test vector con magnitud cero
TEST(test_vector, magnitude_zero) {
  render::vector const vec{0.0, 0.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 0.0);
}

//Test vector con magnitudes positivas
TEST(test_vector, magnitude_positive) {
  render::vector const vec{3.0, 4.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 5.0);
}

//Test vector con magnitudes negativas
TEST(test_vector, add_sub) {
  render::vector const a{1.0, 2.0, 3.0};
  render::vector const b{4.0, -1.0, 0.5};

  render::vector const sum = render::vector::add(a, b);
  EXPECT_DOUBLE_EQ(sum.x, 5.0);
  EXPECT_DOUBLE_EQ(sum.y, 1.0);
  EXPECT_DOUBLE_EQ(sum.z, 3.5);

  render::vector const diff = render::vector::sub(a, b);
  EXPECT_DOUBLE_EQ(diff.x, -3.0);
  EXPECT_DOUBLE_EQ(diff.y, 3.0);
  EXPECT_DOUBLE_EQ(diff.z, 2.5);
}

//Test vector con multiples escalados y divisiones
TEST(test_vector, muld_divd) {
  render::vector const v{2.0, -3.0, 4.5};
  render::vector const scaled = render::vector::muld(v, 2.5);
  EXPECT_DOUBLE_EQ(scaled.x, 5.0);
  EXPECT_DOUBLE_EQ(scaled.y, -7.5);
  EXPECT_DOUBLE_EQ(scaled.z, 11.25);

  render::vector const halved = render::vector::divd(v, 2.0);
  EXPECT_DOUBLE_EQ(halved.x, 1.0);
  EXPECT_DOUBLE_EQ(halved.y, -1.5);
  EXPECT_DOUBLE_EQ(halved.z, 2.25);
}

//Test division por cero lanza excepcion
TEST(test_vector, divd_by_zero_throws) {
  render::vector const v{1.0, 2.0, 3.0};
  auto func = [&]() { [[maybe_unused]] auto res = render::vector::divd(v, 0.0); };
  EXPECT_THROW(func(), std::invalid_argument);
}

//Test producto punto y cruz
TEST(test_vector, dot_and_cross) {
  render::vector const a{1.0, 0.0, 0.0};
  render::vector const b{0.0, 1.0, 0.0};

  double const dp = render::vector::dotp(a, b);
  EXPECT_DOUBLE_EQ(dp, 0.0);

  render::vector const cp = render::vector::crossp(a, b);
  // a x b = (0,0,1)
  EXPECT_DOUBLE_EQ(cp.x, 0.0);
  EXPECT_DOUBLE_EQ(cp.y, 0.0);
  EXPECT_DOUBLE_EQ(cp.z, 1.0);

  // check non-orthogonal
  render::vector const c{2.0, 3.0, 4.0};
  render::vector const d{5.0, 6.0, 7.0};
  EXPECT_DOUBLE_EQ(render::vector::dotp(c, d), 2.0 * 5.0 + 3.0 * 6.0 + 4.0 * 7.0);
}

//Test normalizacion de vector no nulo y nulo
TEST(test_vector, normalize_nonzero_and_zero) {
  render::vector const v{3.0, 0.0, 4.0};  // magnitude 5
  render::vector const n = render::vector::normalize(v);
  EXPECT_NEAR(n.x, 0.6, 1e-12);
  EXPECT_NEAR(n.y, 0.0, 1e-12);
  EXPECT_NEAR(n.z, 0.8, 1e-12);
  EXPECT_NEAR(n.magnitude(), 1.0, 1e-12);

  render::vector const z{0.0, 0.0, 0.0};
  render::vector const nz = render::vector::normalize(z);
  EXPECT_DOUBLE_EQ(nz.x, 0.0);
  EXPECT_DOUBLE_EQ(nz.y, 0.0);
  EXPECT_DOUBLE_EQ(nz.z, 0.0);
}

//Test multiplicacion componente a componente
TEST(test_vector, mul) {
  render::vector const a{1.0, 2.0, 3.0};
  render::vector const b{4.0, -1.0, 0.5};

  render::vector const prod = render::vector::mul(a, b);
  EXPECT_DOUBLE_EQ(prod.x, 4.0);
  EXPECT_DOUBLE_EQ(prod.y, -2.0);
  EXPECT_DOUBLE_EQ(prod.z, 1.5);
}
