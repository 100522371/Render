#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <ray.hpp>
#include <scene.hpp>
#include <string>
#include <system_error>

// --------------- Scene::load_scene tests ----------------

//Test de casos válidos
TEST(test_load_scene, basic_materials_and_objects) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_ok.txt";
  std::ofstream ofs(path);
  ofs << "matte: matte_red 1 0 0\n"
         "metal: metal_s 0.8 0.8 0.8 0.0\n"
         "refractive: glass 1.5\n"
         "sphere: 0 0 0 1 glass\n"
         "cylinder: 1 0 0 0.5 0 1 0 metal_s\n";
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));
  EXPECT_NE(scene.materialByName("matte_red"), nullptr);
  EXPECT_NE(scene.materialByName("metal_s"), nullptr);
  EXPECT_NE(scene.materialByName("glass"), nullptr);
  EXPECT_EQ(scene.objects.size(), 2U);

  std::filesystem::remove(path);
}

// Test de manejo de espacios en blanco y líneas vacías
TEST(test_load_scene, whitespace_and_empty_lines) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_ws.txt";
  std::ofstream ofs(path);
  ofs << "\n"
         "  matte:   m   0.1   0.2   0.3\n"
         "\n"
         "sphere: 0 0 0 0.5 m\n";
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));
  EXPECT_NE(scene.materialByName("m"), nullptr);
  EXPECT_EQ(scene.objects.size(), 1U);

  std::filesystem::remove(path);
}

// Test de múltiples objetos que usan el mismo material
TEST(test_load_scene, multiple_objects_same_material) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_multiobj.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 0.2 0.2 0.2\n"
         "sphere: 0 0 0 1 m\n"
         "sphere: 2 0 0 0.5 m\n"
         "cylinder: 1 1 1 0.3 0 1 0 m\n";
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));
  EXPECT_EQ(scene.materials.size(), 1U);
  EXPECT_EQ(scene.objects.size(), 3U);

  std::filesystem::remove(path);
}

// ======================== Invalid Cases =============================

// Test de nombre de material duplicado
TEST(test_load_scene, duplicate_material_name) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_dupmat.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 1 0 0\n"
         "metal: m 0.5 0.5 0.5 0.1\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("already exists") != std::string::npos or
                msg.find("Material with name") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de parámetros insuficientes para material
TEST(test_load_scene, insufficient_material_parameters) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_fewmatparam.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 0.1 0.2 0.3\n"
         "sphere: 0 0 0 0.5 m\n";
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));
  EXPECT_NE(scene.materialByName("m"), nullptr);
  EXPECT_EQ(scene.objects.size(), 1U);

  std::filesystem::remove(path);
}

// Test de parámetros inválidos para material
TEST(test_load_scene, invalid_material_parameters) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_badmatparam.txt";
  std::ofstream ofs(path);
  ofs << "matte: m a b c\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Invalid matte") != std::string::npos or
                msg.find("Invalid matte material") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de tokens extra después de definición de material
TEST(test_load_scene, material_extra_tokens_after) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_extramaterial.txt";
  std::ofstream ofs(path);
  ofs << "refractive: r 1.5 extra\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Extra data") != std::string::npos or
                msg.find("Extra data after configuration value") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de objeto que referencia un material inexistente
TEST(test_load_scene, object_refers_missing_material) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_missingmat.txt";
  std::ofstream ofs(path);
  ofs << "sphere: 0 0 0 1 no_such_mat\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_NE(msg.find("Material not found"), std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de parámetros inválidos para objeto (radio negativo)
TEST(test_load_scene, sphere_invalid_parameters) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_inv_radius.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 1 1 1\n"
         "sphere: 0 0 0 -1 m\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Invalid sphere") != std::string::npos or
                msg.find("Invalid sphere parameters") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de parámetros insuficientes para objeto
TEST(test_load_scene, cylinder_too_few_params) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_cyl_few.txt";
  std::ofstream ofs(path);
  ofs << "cylinder: 0 0 0 1 0 1\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Invalid cylinder") != std::string::npos);
  }

  std::filesystem::remove(path);
}

//Test de tokens extra después de definición de objeto
TEST(test_load_scene, object_extra_tokens_after) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_obj_extra.txt";
  std::ofstream ofs(path);
  ofs << "cylinder: 0 0 0 1 0 1 0 mat extra\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Extra data") != std::string::npos or
                msg.find("Extra data after configuration value") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de parámetros no numéricos para objeto
TEST(test_load_scene, object_invalid_numeric_parameter) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_obj_non_numeric.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 0.1 0.1 0.1\n"
         "sphere: x y z 1 m\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Invalid sphere") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de línea sin el ':' separador
TEST(test_load_scene, missing_colon_label) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_missingcolon.txt";
  std::ofstream ofs(path);
  ofs << "matte m 1 0 0\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Unknown scene entity") != std::string::npos or
                msg.find("Invalid") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de archivo vacío o con solo espacios en blanco
TEST(test_load_scene, empty_file_or_only_whitespace) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_empty.txt";
  std::ofstream ofs(path);
  ofs << "\n\n   \n";
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));
  EXPECT_EQ(scene.materials.size(), 0U);
  EXPECT_EQ(scene.objects.size(), 0U);

  std::filesystem::remove(path);
}

// Test de objeto definido antes que su material
TEST(test_load_scene, object_before_material) {
  // el objeto hace referencia a un material que se define después
  Scene scene;
  std::string const path = "/tmp/test_load_scene_obj_before_mat.txt";
  std::ofstream ofs(path);
  ofs << "sphere: 0 0 0 1 mat\n"
         "matte: mat 0.1 0.2 0.3\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_NE(msg.find("Material not found"), std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de entidad desconocida
TEST(test_load_scene, unknown_entity) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_unk_ent.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 0.1 0.1 0.1\n"
         "triangle: 0 0 0 m\n";
  ofs.close();

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Unknown scene entity") != std::string::npos);
  }

  std::filesystem::remove(path);
}

// Test de archivo inexistente
TEST(test_load_scene, missing_file) {
  Scene scene;
  std::string const path = "/tmp/test_load_scene_missing_file.txt";
  std::error_code ec;
  std::filesystem::remove(path, ec);

  try {
    scene.load_scene(path);
    FAIL() << "Expected exception";
  } catch (std::exception const & e) {
    std::string const msg = e.what();
    EXPECT_TRUE(msg.find("Could not open scene file") != std::string::npos or
                msg.find("Could not open") != std::string::npos);
  }
}

// -- -- -- -- -- -- -- - Scene::intersect tests -- -- -- -- -- -- -- --

// Test de rayo que no intersecta ningún objeto en escena vacía
TEST(test_intersect_scene, Intersect_NoHit) {
  Scene const scene;
  render::ray r;
  r.origin    = render::vector{0.0, 0.0, -5.0};
  r.direction = render::vector{0.0, 1.0, 0.0};  // apunta lejos, no intersecta nada en escena vacía

  auto res = scene.intersect(r);
  EXPECT_FALSE(res.has_value());
}

// Test de rayo que intersecta un solo objeto
TEST(test_intersect_scene, Intersect_SeleccionaElMasCercano) {
  Scene scene;
  std::string const path = "/tmp/test_intersect_two_spheres.txt";
  std::ofstream ofs(path);
  ofs << "matte: a 1 0 0\n"
         "matte: b 0 1 0\n"
         "sphere: 0 0 0 1 a\n"   // esfera cercana (esperada)
         "sphere: 0 0 5 1 b\n";  // esfera lejana
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));

  render::ray r;
  r.origin    = render::vector{0.0, 0.0, -5.0};
  r.direction = render::vector{0.0, 0.0, 1.0};

  auto res = scene.intersect(r);
  if (!res) {
    FAIL() << "Expected intersection but got none.";
  }
  auto const & hit = res.value();
  EXPECT_NEAR(hit.lambda, 4.0, 1e-6);  // coincide con la esfera en z=0
  EXPECT_EQ(hit.nombre_material, "a");

  std::filesystem::remove(path);
}

// Test de rayo que empieza dentro de un objeto
TEST(test_intersect_scene, Intersect_RayoEmpiezaDentroSeleccionaSalida) {
  Scene scene;
  std::string const path = "/tmp/test_intersect_inside.txt";
  std::ofstream ofs(path);
  ofs << "matte: m 0.2 0.2 0.8\n"
         "sphere: 0 0 0 2 m\n";  // radio 2
  ofs.close();

  EXPECT_NO_THROW(scene.load_scene(path));

  render::ray r;
  r.origin    = render::vector{0.0, 0.0, 0.0};  // dentro de la esfera
  r.direction = render::vector{0.0, 0.0, 1.0};

  auto res = scene.intersect(r);
  if (!res) {
    FAIL() << "Expected intersection but got none.";
  }
  auto const & hit = res.value();
  EXPECT_NEAR(hit.lambda, 2.0, 1e-6);  // distancia hasta salir de la esfera

  std::filesystem::remove(path);
}
