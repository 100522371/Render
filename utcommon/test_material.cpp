#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <material.hpp>
#include <numbers>
#include <random>
#include <tuple>
#include <vector.hpp>
#include <vector>

namespace {

  // Helper para comparar dos vectores con tolerancia
  void ExpectVectorNear(render::vector const & a, render::vector const & b, double tol = 1e-9) {
    EXPECT_NEAR(a.x, b.x, tol);
    EXPECT_NEAR(a.y, b.y, tol);
    EXPECT_NEAR(a.z, b.z, tol);
  }

  // Helper para verificar que un vector está normalizado (magnitud ≈ 1)
  void ExpectNormalized(render::vector const & v, double tol = 1e-9) {
    double const m = v.magnitude();
    EXPECT_NEAR(m, 1.0, tol);
  }

  // Helper para verificar que un vector está en el hemisferio correcto (dotp positivo)
  void ExpectInHemisphere(render::vector const & dir, render::vector const & normal) {
    double const dp = render::vector::dotp(dir, normal);
    EXPECT_GE(dp, -1e-6);  // Permitir pequeño error numérico
  }

}  // namespace

//clase de test para materiales
class MaterialTest : public ::testing::Test {
protected:
  std::mt19937_64 rng{123'456'789ULL};

  Matte matte_gray{"matte_gray", render::vector(0.5, 0.5, 0.5)};
  Matte matte_colored{"matte_colored", render::vector(0.8, 0.3, 0.1)};
  Metal metal_perfect{"metal_perfect", render::vector(0.9, 0.9, 0.9), 0.0};
  Metal metal_fuzzy{"metal_fuzzy", render::vector(0.7, 0.5, 0.3), 0.3};
  Metal metal_very_fuzzy{"metal_very_fuzzy", render::vector(0.6, 0.6, 0.6), 1.0};
  Refractive glass{"glass", 1.5};
  Refractive diamond{"diamond", 2.417};
  Refractive water{"water", 1.333};
};

// ==================== MATTE TESTS ====================

// Verificar que la dirección retornada está normalizada
TEST_F(MaterialTest, matte_returns_normalized_direction) {
  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal = render::vector::normalize(render::vector(0.0, 1.0, 0.0));

  for (int i = 0; i < 20; ++i) {  // probamos varias veces para cubrir el componente aleatorio
    auto [dir, refl] = matte_gray.scatter(in_dir, normal, rng);
    ExpectNormalized(dir, 1e-8);
  }
}

// Verificar que el reflectance retornado es el correcto
TEST_F(MaterialTest, matte_returns_correct_reflectance) {
  render::vector const in_dir(0.0, 0.0, 1.0);
  render::vector const normal(0.0, 0.0, 1.0);

  auto [dir, refl] = matte_colored.scatter(in_dir, normal, rng);
  ExpectVectorNear(refl, render::vector(0.8, 0.3, 0.1), 1e-12);
}

// Verificar que la dirección retornada está en el hemisferio definido por la normal
TEST_F(MaterialTest, matte_direction_in_hemisphere) {
  render::vector const in_dir = render::vector::normalize(render::vector(0.5, -1.0, 0.3));
  render::vector const normal = render::vector::normalize(render::vector(0.0, 1.0, 0.0));

  for (int i = 0; i < 50; ++i) {  // probamos varias veces para cubrir el componente aleatorio
    auto [dir, refl] = matte_gray.scatter(in_dir, normal, rng);
    ExpectInHemisphere(dir, normal);
  }
}

// Verificar que la dirección varía con diferentes llamadas (componente aleatorio)
TEST_F(MaterialTest, matte_produces_varied_directions) {
  // rng ocasionalmente puede producir la misma dirección
  // pero en 30 intentos deberíamos ver variación significativa
  render::vector const in_dir(0.0, -1.0, 0.0);
  render::vector const normal(0.0, 1.0, 0.0);

  std::vector<render::vector> seen;
  for (int i = 0; i < 30; ++i) {
    auto [dir, refl] = matte_gray.scatter(in_dir, normal, rng);
    // Redondear por 1e6 para permitir que dirs parecidos sean considerados iguales
    seen.push_back(render::vector::muld(dir, 1e6));
  }  // seen es un std::vector de las direcciones vistas, redondeadas a 1e6
  // Verificar que hay variación real comparando pares de direcciones
  int found_significant_difference =
      0;  // contamos las veces que encontramos diferencias significativas
  for (size_t i = 0; i < seen.size() - 1; ++i) {
    for (size_t j = i + 1; j < seen.size(); ++j) {
      render::vector const diff = render::vector::sub(seen[i], seen[j]);
      if (diff.magnitude() > 0.1) {  // Diferencia significativa
        found_significant_difference++;
      }  // sale del loop interno
    }
  }
  EXPECT_GE(found_significant_difference, 1);
}

//Tests para casos especiales de Matte
TEST_F(MaterialTest, matte_handles_perpendicular_incidence) {
  // in_dir y normal son perpendiculares
  render::vector const in_dir(0.0, -1.0, 0.0);
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = matte_gray.scatter(in_dir, normal, rng);
  ExpectNormalized(dir, 1e-8);
  ExpectInHemisphere(dir, normal);
}

// Verificar que maneja bien ángulos rasantes
TEST_F(MaterialTest, matte_handles_grazing_incidence) {
  // in_dir casi paralelo a la superficie
  render::vector const in_dir = render::vector::normalize(render::vector(0.999, -0.001, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = matte_gray.scatter(in_dir, normal, rng);
  ExpectNormalized(dir, 1e-8);
  ExpectInHemisphere(dir, normal);
}

// Verificar que maneja bien el caso donde la dirección resultante podría ser cercana a cero
TEST_F(MaterialTest, matte_when_direction_near_zero) {
  // Caso extremo: normal muy pequeño puede provocar dir ≈ 0
  render::vector const in_dir(1.0, 0.0, 0.0);
  render::vector const normal(1e-10, 1e-10, 1e-10);

  auto [dir, refl] = matte_gray.scatter(in_dir, normal, rng);
  ExpectNormalized(dir, 1e-6);
  // Debe retornar algo válido, no NaN
  EXPECT_FALSE(std::isnan(dir.x));
  EXPECT_FALSE(std::isnan(dir.y));
  EXPECT_FALSE(std::isnan(dir.z));
}

// ==================== METAL TESTS ====================

// Verificar que la dirección retornada está normalizada
TEST_F(MaterialTest, metal_perfect_reflection_no_fuzz) {
  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal = render::vector::normalize(render::vector(0.0, 1.0, 0.0));

  auto [dir, refl] = metal_perfect.scatter(in_dir, normal, rng);

  // Calcular reflexión perfecta: r = d - 2(d·n)n
  render::vector const expected = render::vector::normalize(render::vector::sub(
      in_dir, render::vector::muld(normal, 2.0 * render::vector::dotp(in_dir, normal))));

  ExpectNormalized(dir, 1e-8);
  ExpectVectorNear(dir, expected, 1e-6);
  ExpectVectorNear(refl, render::vector(0.9, 0.9, 0.9), 1e-12);
}

// Verificar que la dirección retornada está normalizada incluso con fuzz
TEST_F(MaterialTest, metal_returns_normalized_with_fuzz) {
  render::vector const in_dir = render::vector::normalize(render::vector(0.6, -0.8, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  for (int i = 0; i < 20; ++i) {  // probamos varias veces para cubrir el componente aleatorio
    auto [dir, refl] = metal_fuzzy.scatter(in_dir, normal, rng);
    ExpectNormalized(dir, 1e-8);
  }
}

// Verificar que el reflectance retornado es el correcto
TEST_F(MaterialTest, metal_fuzz_creates_variation) {
  render::vector const in_dir = render::vector::normalize(render::vector(0.5, -0.5, 0.5));
  render::vector const normal = render::vector::normalize(render::vector(0.0, 1.0, 0.0));

  // r = d - 2(d·n)n
  render::vector const perfect = render::vector::normalize(render::vector::sub(
      in_dir, render::vector::muld(normal, 2.0 * render::vector::dotp(in_dir, normal))));

  std::vector<render::vector> seen;
  int saw_difference               = 0;  // Verificar que al menos una vez difiere del perfecto
  int found_significant_difference = 0;
  for (int i = 0; i < 30; ++i) {
    auto [dir, refl] = metal_fuzzy.scatter(in_dir, normal, rng);

    if (std::fabs(dir.x - perfect.x) > 1e-4 or
        std::fabs(dir.y - perfect.y) > 1e-4 or
        std::fabs(dir.z - perfect.z) > 1e-4)
    {
      saw_difference++;
    }
    seen.push_back(render::vector::muld(dir, 1e6));
  }
  for (size_t i = 0; i < seen.size() - 1; ++i) {
    for (size_t j = i + 1; j < seen.size(); ++j) {
      render::vector const diff = render::vector::sub(seen[i], seen[j]);
      if (diff.magnitude() > 0.1) {  // Diferencia significativa
        found_significant_difference++;
      }  // sale del loop interno
    }
  }

  EXPECT_GE(saw_difference, 1);                // Al menos una vez difiere del perfecto
  EXPECT_GE(found_significant_difference, 1);  // Variación significativa entre resultados
}

// Verificar que con fuzz=1.0 la variación es alta
TEST_F(MaterialTest, metal_max_fuzz_creates_large_variation) {
  // Con fuzz=1.0, la variación debe ser muy alta
  // in_dir perpendicular a normal para máxima dispersión
  render::vector const in_dir(0.0, -1.0, 0.0);
  render::vector const normal(0.0, 1.0, 0.0);

  std::vector<double> x_coords, y_coords, z_coords;

  for (int i = 0; i < 50; ++i) {
    auto [dir, refl] = metal_very_fuzzy.scatter(in_dir, normal, rng);
    x_coords.push_back(dir.x);
    y_coords.push_back(dir.y);
    z_coords.push_back(dir.z);
  }

  // Calcular desviación estándar de cada componente
  auto calc_stddev = [](std::vector<double> const & vals) {
    double sum = 0.0;
    for (double const v : vals) {
      sum += v;
    }
    double const mean = sum / static_cast<double>(vals.size());

    double var = 0.0;
    for (double const v : vals) {
      double const diff = v - mean;
      var += diff * diff;
    }
    return std::sqrt(var / static_cast<double>(vals.size()));
  };

  // Desviación estándar en X y Z
  double const stddev_x = calc_stddev(x_coords);
  double const stddev_z = calc_stddev(z_coords);

  // Con fuzz=1.0, los componentes X y Z deben tener alta variación
  // (Y debe estar cerca de 1.0 con menos variación por ser perpendicular)
  EXPECT_GT(stddev_x, 0.2);  // Alta dispersión en X
  EXPECT_GT(stddev_z, 0.2);  // Alta dispersión en Z

  // Verificar que no todas las direcciones son idénticas
  bool all_same = true;
  for (size_t i = 1; i < x_coords.size(); ++i) {
    if (std::fabs(x_coords[i] - x_coords[0]) > 0.01) {
      all_same = false;
      break;
    }
  }
  EXPECT_FALSE(all_same);
}

// Verificar que con fuzz=0.0 la variación es mínima comparado con fuzz=1.0
TEST_F(MaterialTest, metal_fuzz_zero_vs_one_comparison) {
  render::vector const in_dir(0.0, -1.0, 0.0);
  render::vector const normal(0.0, 1.0, 0.0);

  Metal const metal_no_fuzz{"no_fuzz", render::vector(0.9, 0.9, 0.9), 0.0};
  Metal const metal_max_fuzz{"max_fuzz", render::vector(0.9, 0.9, 0.9), 1.0};

  std::vector<double> angles_no_fuzz, angles_max_fuzz;
  render::vector const perfect(0.0, 1.0, 0.0);

  for (int i = 0; i < 30; ++i) {
    auto [dir0, refl0] = metal_no_fuzz.scatter(in_dir, normal, rng);
    auto [dir1, refl1] = metal_max_fuzz.scatter(in_dir, normal, rng);

    double const cos0 = render::vector::dotp(dir0, perfect);
    double const cos1 = render::vector::dotp(dir1, perfect);

    angles_no_fuzz.push_back(std::acos(std::clamp(cos0, -1.0, 1.0)));
    angles_max_fuzz.push_back(std::acos(std::clamp(cos1, -1.0, 1.0)));
  }

  // Calcular promedios
  double avg_no_fuzz = 0.0, avg_max_fuzz = 0.0;
  for (size_t i = 0; i < 30; ++i) {
    avg_no_fuzz += angles_no_fuzz[i];
    avg_max_fuzz += angles_max_fuzz[i];
  }
  avg_no_fuzz /= 30.0;
  avg_max_fuzz /= 30.0;

  // Con fuzz=0, desviación debe ser ~0
  EXPECT_LT(avg_no_fuzz, 0.01);  // < ~0.5 grados

  // Con fuzz=1.0, desviación debe ser significativa
  EXPECT_GT(avg_max_fuzz, 0.3);  // > ~17 grados

  // La diferencia debe ser clara
  EXPECT_GT(avg_max_fuzz, avg_no_fuzz * 10);
}

// Verificar que maneja bien el caso de incidencia perpendicular
TEST_F(MaterialTest, metal_handles_perpendicular_incidence) {
  render::vector const in_dir(0.0, -1.0, 0.0);
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = metal_perfect.scatter(in_dir, normal, rng);
  ExpectNormalized(dir, 1e-8);
  // Reflexión perpendicular debe invertir dirección
  ExpectVectorNear(dir, render::vector(0.0, 1.0, 0.0), 1e-6);
}

// Verificar que maneja bien ángulos rasantes
TEST_F(MaterialTest, metal_when_result_near_zero) {
  render::vector const in_dir(1.0, 0.0, 0.0);
  render::vector const normal(1e-10, 1e-10, 1e-10);

  auto [dir, refl] = metal_fuzzy.scatter(in_dir, normal, rng);
  ExpectNormalized(dir, 1e-6);
  EXPECT_FALSE(std::isnan(dir.x));
  EXPECT_FALSE(std::isnan(dir.y));
  EXPECT_FALSE(std::isnan(dir.z));
}

// ==================== REFRACTIVE TESTS ====================

// Verificar que la dirección retornada está normalizada
TEST_F(MaterialTest, refractive_returns_normalized_direction) {
  render::vector const in_dir = render::vector::normalize(render::vector(0.0, -1.0, 0.2));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = glass.scatter(in_dir, normal, rng);
  ExpectNormalized(dir, 1e-8);
}

// Verificar que el reflectance retornado es el correcto
TEST_F(MaterialTest, refractive_returns_white_attenuation) {
  render::vector const in_dir = render::vector::normalize(render::vector(0.5, -0.5, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = glass.scatter(in_dir, normal, rng);
  ExpectVectorNear(refl, render::vector(1.0, 1.0, 1.0), 1e-12);
}

// Verificar que la refracción dobla el rayo hacia la normal al entrar a un medio más denso
TEST_F(MaterialTest, refractive_bends_toward_normal_entering) {
  // Rayo entrando desde aire a vidrio, debe doblarse hacia la normal
  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = glass.scatter(in_dir, normal, rng);

  // El ángulo con la normal debe ser menor (más cercano a la normal)
  double const cos_in  = std::fabs(render::vector::dotp(in_dir, normal));
  double const cos_out = std::fabs(render::vector::dotp(dir, normal));

  EXPECT_GT(cos_out, cos_in);  // Más cercano a la normal
}

// Verificar que la refracción dobla el rayo alejándose de la normal al salir a un medio menos denso
TEST_F(MaterialTest, refractive_total_internal_reflection) {
  // no existe refracción, solo reflexión -> TIR(total internal reflection)
  // in_dir casi tangencial -> angulo de incidencia respecto a la normal alto -> sin theta alto
  // dotp negativo -> front_face = true
  render::vector const in_dir = render::vector::normalize(render::vector(0.95, 0.0, -0.31));
  render::vector const normal(0.0, 0.0, 1.0);

  // Para simular "desde dentro", usamos un material con ior efectivo inverso
  // o verificamos que el resultado es reflexión
  Refractive const from_inside{"from_inside", 0.67};  // ref_ratio = 1/0.67 aproximadamente 1.5

  auto [dir, refl] = from_inside.scatter(in_dir, normal, rng);

  // ref_ratio * sin theta > 1 -> reflexión
  render::vector const expected_reflect = render::vector::normalize(render::vector::sub(
      in_dir, render::vector::muld(normal, 2.0 * render::vector::dotp(in_dir, normal))));

  ExpectVectorNear(dir, expected_reflect, 1e-5);
}

// Verificar incidencia perpendicular
TEST_F(MaterialTest, refractive_perpendicular_incidence) {
  render::vector const in_dir(0.0, -1.0, 0.0);
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = glass.scatter(in_dir, normal, rng);

  ExpectNormalized(dir, 1e-8);
  // Incidencia perpendicular: dirección debe ser casi la misma -> no hay desviación significativa
  ExpectVectorNear(dir, render::vector(0.0, -1.0, 0.0), 1e-5);
}

// Verificar ángulo rasante
TEST_F(MaterialTest, refractive_high_ior_more_bending) {
  // Comparar ior_vidrio vs ior_diamante para el mismo ángulo de incidencia
  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir_glass, refl_glass]     = glass.scatter(in_dir, normal, rng);
  auto [dir_diamond, refl_diamond] = diamond.scatter(in_dir, normal, rng);

  double const cos_glass   = std::fabs(render::vector::dotp(dir_glass, normal));
  double const cos_diamond = std::fabs(render::vector::dotp(dir_diamond, normal));

  // Diamante (ior mayor) dobla más hacia la normal
  EXPECT_GT(cos_diamond, cos_glass);
}

// Verificar comportamiento al entrar vs salir del material
TEST_F(MaterialTest, refractive_entering_vs_exiting) {
  render::vector const normal(0.0, 1.0, 0.0);
  // Entrando (aire -> vidrio): dotp < 0 -> front_face true
  render::vector const in_entering = render::vector::normalize(render::vector(0.5, -0.866, 0.0));
  // Saliendo (vidrio -> aire): dotp > 0 -> front_face false
  render::vector const in_exiting = render::vector::normalize(render::vector(0.5, 0.866, 0.0));

  auto [dir_enter, refl_enter] = glass.scatter(in_entering, normal, rng);
  auto [dir_exit, refl_exit]   = glass.scatter(in_exiting, normal, rng);

  double const cos_enter = std::fabs(render::vector::dotp(dir_enter, normal));
  double const cos_exit  = std::fabs(render::vector::dotp(dir_exit, normal));

  // Deben diferir porque los ratios usados son distintos:
  // entering -> ref_ratio = 1/ior, exiting -> ref_ratio = ior
  EXPECT_NE(cos_enter, cos_exit);
}

// Verificar que la refracción sigue aproximadamente la ley de Snell
TEST_F(MaterialTest, refractive_snells_law_approximation) {
  // Verificar que sigue aproximadamente la ley de Snell: n1*sin(θ1) = n2*sin(θ2)
  double const angle_in = std::numbers::pi / 4.0;  // 45 grados
  render::vector const in_dir =
      render::vector::normalize(render::vector(std::sin(angle_in), -std::cos(angle_in), 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = glass.scatter(in_dir, normal, rng);

  double const sin_in  = std::sin(angle_in);
  double const cos_out = render::vector::dotp(dir, normal);
  double const sin_out = std::sqrt(std::max(0.0, 1.0 - cos_out * cos_out));

  // n1 * sin(θ1) ≈ n2 * sin(θ2)
  // 1.0 * sin_in ≈ 1.5 * sin_out
  double const expected_sin_out = sin_in / 1.5;

  EXPECT_NEAR(sin_out, expected_sin_out, 1e-3);
}

// ==================== EDGE CASES & INVALID SCENARIOS ====================

// Verificar que con ior=1 no hay desviación
TEST_F(MaterialTest, refractive_with_ior_one) {
  Refractive const vacuum{"vacuum", 1.0};

  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = vacuum.scatter(in_dir, normal, rng);

  ExpectNormalized(dir, 1e-8);
  // Con ior=1, el rayo debe pasar sin desviarse
  ExpectVectorNear(dir, in_dir, 1e-5);
}

// Verificar que los materiales manejan normales no normal
TEST_F(MaterialTest, all_materials_handle_non_normalized_normal) {
  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal_unnormalized(0.0, 2.0, 0.0);  // No normalizado

  // Los materiales deberían manejar esto internamente o el usuario debe normalizar
  // Verificamos que no crashea y produce salida válida
  auto [dir_matte, refl_matte] = matte_gray.scatter(in_dir, normal_unnormalized, rng);
  auto [dir_metal, refl_metal] = metal_perfect.scatter(in_dir, normal_unnormalized, rng);
  auto [dir_refr, refl_refr]   = glass.scatter(in_dir, normal_unnormalized, rng);

  EXPECT_FALSE(std::isnan(dir_matte.x));
  EXPECT_FALSE(std::isnan(dir_metal.x));
  EXPECT_FALSE(std::isnan(dir_refr.x));
}

// Verificar que maneja bien ángulos rasantes extremos
TEST_F(MaterialTest, refractive_extreme_grazing_angle) {
  // Ángulo casi tangencial
  render::vector const in_dir = render::vector::normalize(render::vector(0.9999, -0.01, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir, refl] = glass.scatter(in_dir, normal, rng);

  ExpectNormalized(dir, 1e-8);
  EXPECT_FALSE(std::isnan(dir.x));
  EXPECT_FALSE(std::isnan(dir.y));
  EXPECT_FALSE(std::isnan(dir.z));
}

TEST_F(MaterialTest, metal_fuzz_clamped_to_valid_range) {
  // El constructor debería clampear fuzz a [0, 1]
  Metal const metal_negative{"neg", render::vector(0.5, 0.5, 0.5), -0.5};
  Metal const metal_large{"large", render::vector(0.5, 0.5, 0.5), 2.5};

  render::vector const in_dir = render::vector::normalize(render::vector(1.0, -1.0, 0.0));
  render::vector const normal(0.0, 1.0, 0.0);

  // No debe crashear
  auto [dir_neg, refl_neg]     = metal_negative.scatter(in_dir, normal, rng);
  auto [dir_large, refl_large] = metal_large.scatter(in_dir, normal, rng);

  ExpectNormalized(dir_neg, 1e-8);
  ExpectNormalized(dir_large, 1e-8);
}

// Verificar reproducibilidad con la misma semilla
TEST_F(MaterialTest, reproducibility_with_same_seed) {
  std::mt19937_64 rng1{42};
  std::mt19937_64 rng2{42};

  render::vector const in_dir = render::vector::normalize(render::vector(0.3, -0.7, 0.5));
  render::vector const normal(0.0, 1.0, 0.0);

  auto [dir1, refl1] = matte_gray.scatter(in_dir, normal, rng1);
  auto [dir2, refl2] = matte_gray.scatter(in_dir, normal, rng2);

  ExpectVectorNear(dir1, dir2, 1e-12);
}
