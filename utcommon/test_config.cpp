#include <config.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

// Pruebas unitarias para la clase Config
TEST(test_config, default_values) {
  Config const cfg;
  EXPECT_EQ(cfg.aspect_w, 16);
  EXPECT_EQ(cfg.aspect_h, 9);
  EXPECT_EQ(cfg.image_width, 1'920);
  EXPECT_DOUBLE_EQ(cfg.gamma, 2.2);
  EXPECT_DOUBLE_EQ(cfg.camera_position.x, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_position.y, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_position.z, -10.0);
  EXPECT_DOUBLE_EQ(cfg.camera_target.x, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_target.y, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_target.z, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_north.x, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_north.y, 1.0);
  EXPECT_DOUBLE_EQ(cfg.camera_north.z, 0.0);
  EXPECT_DOUBLE_EQ(cfg.field_of_view, 90.0);
  EXPECT_EQ(cfg.samples_per_pixel, 20);
  EXPECT_EQ(cfg.max_depth, 5);
  EXPECT_EQ(cfg.material_rng_seed, 13);
  EXPECT_EQ(cfg.ray_rng_seed, 19);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.x, 0.25);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.y, 0.5);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.z, 1.0);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.x, 1.0);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.y, 1.0);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.z, 1.0);
}

// Pruebas de carga de configuración desde archivo
TEST(test_config, load_aspect_ratio) {
  std::string const path = "/tmp/test_config_aspect.txt";
  std::ofstream ofs(path);
  ofs << "aspect_ratio: 4 3\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_EQ(cfg.aspect_w, 4);
  EXPECT_EQ(cfg.aspect_h, 3);
  EXPECT_EQ(cfg.seen_keys().at("aspect_ratio:"), 1);

  std::filesystem::remove(path);
}

// Prueba de carga de image_width
TEST(test_config, load_image_width) {
  std::string const path = "/tmp/test_config_image_width.txt";
  std::ofstream ofs(path);
  ofs << "image_width: 1280\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_EQ(cfg.image_width, 1'280);
  EXPECT_EQ(cfg.seen_keys().at("image_width:"), 1);

  std::filesystem::remove(path);
}

// Prueba de carga de gamma
TEST(test_config, load_gamma) {
  std::string const path = "/tmp/test_config_gamma.txt";
  std::ofstream ofs(path);
  ofs << "gamma: 2.5\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.gamma, 2.5);
  EXPECT_EQ(cfg.seen_keys().at("gamma:"), 1);

  std::filesystem::remove(path);
}

// Prueba de carga de camera_position
TEST(test_config, load_camera_position) {
  std::string const path = "/tmp/test_config_camera_pos.txt";
  std::ofstream ofs(path);
  ofs << "camera_position: 1.5 -2.0 3.5\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.camera_position.x, 1.5);
  EXPECT_DOUBLE_EQ(cfg.camera_position.y, -2.0);
  EXPECT_DOUBLE_EQ(cfg.camera_position.z, 3.5);
  EXPECT_EQ(cfg.seen_keys().at("camera_position:"), 1);

  std::filesystem::remove(path);
}

// Prueba de carga de camera_target
TEST(test_config, load_camera_target) {
  std::string const path = "/tmp/test_config_camera_target.txt";
  std::ofstream ofs(path);
  ofs << "camera_target: 10.0 20.0 30.0\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.camera_target.x, 10.0);
  EXPECT_DOUBLE_EQ(cfg.camera_target.y, 20.0);
  EXPECT_DOUBLE_EQ(cfg.camera_target.z, 30.0);
  EXPECT_EQ(cfg.seen_keys().at("camera_target:"), 1);

  std::filesystem::remove(path);
}

// Prueba de carga de camera_north
TEST(test_config, load_camera_north) {
  std::string const path = "/tmp/test_config_camera_north.txt";
  std::ofstream ofs(path);
  ofs << "camera_north: 0.0 0.0 1.0\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.camera_north.x, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_north.y, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_north.z, 1.0);

  std::filesystem::remove(path);
}

// Prueba de carga de field_of_view
TEST(test_config, load_field_of_view) {
  std::string const path = "/tmp/test_config_fov.txt";
  std::ofstream ofs(path);
  ofs << "field_of_view: 120.5\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.field_of_view, 120.5);

  std::filesystem::remove(path);
}

// Prueba de carga de samples_per_pixel
TEST(test_config, load_samples_per_pixel) {
  std::string const path = "/tmp/test_config_spp.txt";
  std::ofstream ofs(path);
  ofs << "samples_per_pixel: 100\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_EQ(cfg.samples_per_pixel, 100);

  std::filesystem::remove(path);
}

// Prueba de carga de max_depth
TEST(test_config, load_max_depth) {
  std::string const path = "/tmp/test_config_max_depth.txt";
  std::ofstream ofs(path);
  ofs << "max_depth: 10\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_EQ(cfg.max_depth, 10);

  std::filesystem::remove(path);
}

// Prueba de carga de material_rng_seed
TEST(test_config, load_material_rng_seed) {
  std::string const path = "/tmp/test_config_mat_rng.txt";
  std::ofstream ofs(path);
  ofs << "material_rng_seed: 12345\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_EQ(cfg.material_rng_seed, 12'345);

  std::filesystem::remove(path);
}

// Prueba de carga de ray_rng_seed
TEST(test_config, load_ray_rng_seed) {
  std::string const path = "/tmp/test_config_ray_rng.txt";
  std::ofstream ofs(path);
  ofs << "ray_rng_seed: 98765\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_EQ(cfg.ray_rng_seed, 98'765);

  std::filesystem::remove(path);
}

// Prueba de carga de background colors
TEST(test_config, load_background_colors) {
  std::string const path = "/tmp/test_config_bg.txt";
  std::ofstream ofs(path);
  ofs << "background_dark_color: .1 .2 .3\n";  // comprobar que se lee bien .num
  ofs << "background_light_color: .9 .8 .7\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.x, 0.1);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.y, 0.2);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.z, 0.3);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.x, 0.9);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.y, 0.8);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.z, 0.7);

  std::filesystem::remove(path);
}

// Prueba de carga de múltiples claves
TEST(test_config, load_multiple_keys) {
  std::string const path = "/tmp/test_config_multiple.txt";
  std::ofstream ofs(path);
  ofs << "field_of_view: 75.0\n";
  ofs << "samples_per_pixel: 50\n";
  ofs << "max_depth: 15\n";
  ofs << "camera_position: 5.0 6.0 7.0\n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.field_of_view, 75.0);
  EXPECT_EQ(cfg.samples_per_pixel, 50);
  EXPECT_EQ(cfg.max_depth, 15);
  EXPECT_DOUBLE_EQ(cfg.camera_position.x, 5.0);
  EXPECT_DOUBLE_EQ(cfg.camera_position.y, 6.0);
  EXPECT_DOUBLE_EQ(cfg.camera_position.z, 7.0);

  std::filesystem::remove(path);
}

// Prueba de carga con espacios en blanco adicionales
TEST(test_config, load_with_whitespace) {
  std::string const path = "/tmp/test_config_ws.txt";
  std::ofstream ofs(path);
  ofs << "   field_of_view:   60.0   \n";
  ofs << "\n";
  ofs << "  samples_per_pixel:  25  \n";
  ofs.close();

  Config cfg;
  cfg.load_config(path);
  EXPECT_DOUBLE_EQ(cfg.field_of_view, 60.0);
  EXPECT_EQ(cfg.samples_per_pixel, 25);

  std::filesystem::remove(path);
}

// Prueba de archivo no encontrado
TEST(test_config, file_not_found_exits) {
  Config cfg;
  EXPECT_EXIT(cfg.load_config("/nonexistent/path/config.txt"),
              ::testing::ExitedWithCode(EXIT_FAILURE), "Error: Cannot open configuration file");
}

// Prueba de línea malformada (sin ':')
TEST(test_config, missing_colon) {
  std::string const path = "/tmp/test_config_missing_colon.txt";
  std::ofstream ofs(path);
  ofs << "field_of_view 90.0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Unknown configuration key");

  std::filesystem::remove(path);
}

// Prueba de clave desconocida
TEST(test_config, unknown_key_exits) {
  std::string const path = "/tmp/test_config_unknown.txt";
  std::ofstream ofs(path);
  ofs << "unknown_parameter: 123\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Unknown configuration key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_aspect_ratio_1) {
  std::string const path = "/tmp/test_config_invalid_aspect.txt";
  std::ofstream ofs(path);
  ofs << "aspect_ratio: 16\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_aspect_ratio_2) {
  std::string const path = "/tmp/test_config_invalid_aspect2.txt";
  std::ofstream ofs(path);
  ofs << "aspect_ratio: 16 9 4\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_aspect_ratio_3) {
  std::string const path = "/tmp/test_config_invalid_aspect3.txt";
  std::ofstream ofs(path);
  ofs << "aspect_ratio: -16 9\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_aspect_ratio_4) {
  std::string const path = "/tmp/test_config_invalid_aspect4.txt";
  std::ofstream ofs(path);
  ofs << "aspect_ratio: a 9\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_image_width) {
  std::string const path = "/tmp/test_config_invalid_width.txt";
  std::ofstream ofs(path);
  ofs << "image_width: -800\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_image_width_size) {
  std::string const path = "/tmp/test_config_invalid_width.txt";
  std::ofstream ofs(path);
  ofs << "image_width: \n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_image_width_type) {
  std::string const path = "/tmp/test_config_invalid_width.txt";
  std::ofstream ofs(path);
  ofs << "image_width: a\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_gamma) {
  std::string const path = "/tmp/test_config_invalid_gamma.txt";
  std::ofstream ofs(path);
  ofs << "gamma: 0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_gamma_size) {
  std::string const path = "/tmp/test_config_invalid_gamma_size.txt";
  std::ofstream ofs(path);
  ofs << "gamma: \n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_gamma_type) {
  std::string const path = "/tmp/test_config_invalid_gamma_type.txt";
  std::ofstream ofs(path);
  ofs << "gamma: two\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_camera_position) {
  std::string const path = "/tmp/test_config_invalid_cam_pos.txt";
  std::ofstream ofs(path);
  ofs << "camera_position: 1.0 two 3.0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_position_vector) {
  std::string const path = "/tmp/test_config_invalid_vec.txt";
  std::ofstream ofs(path);
  ofs << "camera_position: 1.0 2.0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_camera_target) {
  std::string const path = "/tmp/test_config_invalid_cam_target.txt";
  std::ofstream ofs(path);
  ofs << "camera_target: 1.0 2.0 three\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_target_vector) {
  std::string const path = "/tmp/test_config_invalid_vec.txt";
  std::ofstream ofs(path);
  ofs << "camera_target: 1.0 2.0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_camera_north) {
  std::string const path = "/tmp/test_config_invalid_cam_north.txt";
  std::ofstream ofs(path);
  ofs << "camera_north: x y z\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_north_vector) {
  std::string const path = "/tmp/test_config_invalid_vec.txt";
  std::ofstream ofs(path);
  ofs << "camera_north: 1.0 2.0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_field_of_view) {
  std::string const path = "/tmp/test_config_invalid_fov.txt";
  std::ofstream ofs(path);
  ofs << "field_of_view: ninety\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_field_of_view_size) {
  std::string const path = "/tmp/test_config_invalid_fov_size.txt";
  std::ofstream ofs(path);
  ofs << "field_of_view: 90 45\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_samples_per_pixel) {
  std::string const path = "/tmp/test_config_invalid_spp.txt";
  std::ofstream ofs(path);
  ofs << "samples_per_pixel: -5\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_samples_per_pixel_size) {
  std::string const path = "/tmp/test_config_invalid_spp_size.txt";
  std::ofstream ofs(path);
  ofs << "samples_per_pixel: 5 4\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_samples_per_pixel_type) {
  std::string const path = "/tmp/test_config_invalid_spp.txt";
  std::ofstream ofs(path);
  ofs << "samples_per_pixel: ab\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_max_depth) {
  std::string const path = "/tmp/test_config_invalid_depth.txt";
  std::ofstream ofs(path);
  ofs << "max_depth: 0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_max_depth_size) {
  std::string const path = "/tmp/test_config_invalid_depth_size.txt";
  std::ofstream ofs(path);
  ofs << "max_depth: 4 1\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_max_depth_type) {
  std::string const path = "/tmp/test_config_invalid_depth.txt";
  std::ofstream ofs(path);
  ofs << "max_depth: a\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_color_range_1) {
  std::string const path = "/tmp/test_config_invalid_color_1.txt";
  std::ofstream ofs(path);
  ofs << "background_dark_color: 1.5 0.5 0.5\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_color_range_2) {
  std::string const path = "/tmp/test_config_invalid_color_2.txt";
  std::ofstream ofs(path);
  ofs << "background_light_color: 1.5 0.5 0.5\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_color_size_1) {
  std::string const path = "/tmp/test_config_invalid_color_1.txt";
  std::ofstream ofs(path);
  ofs << "background_dark_color: 1.0 0.5\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_color_size_2) {
  std::string const path = "/tmp/test_config_invalid_color_2.txt";
  std::ofstream ofs(path);
  ofs << "background_light_color: 1.0 0.5 0.5 0.0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_color_type_1) {
  std::string const path = "/tmp/test_config_invalid_color_1.txt";
  std::ofstream ofs(path);
  ofs << "background_dark_color: one two three\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_color_type_2) {
  std::string const path = "/tmp/test_config_invalid_color_2.txt";
  std::ofstream ofs(path);
  ofs << "background_light_color: one two three\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_ray_rng_seed_zero) {
  std::string const path = "/tmp/test_config_ray_zero.txt";
  std::ofstream ofs(path);
  ofs << "ray_rng_seed: 0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_ray_rng_seed_size) {
  std::string const path = "/tmp/test_config_ray_size.txt";
  std::ofstream ofs(path);
  ofs << "ray_rng_seed: 1 3\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_ray_rng_seed_type) {
  std::string const path = "/tmp/test_config_ray_type.txt";
  std::ofstream ofs(path);
  ofs << "ray_rng_seed: one\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");

  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_material_rng_seed_zero) {
  std::string const path = "/tmp/test_config_mat_zero.txt";
  std::ofstream ofs(path);
  ofs << "material_rng_seed: 0\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");
  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_material_rng_seed_size) {
  std::string const path = "/tmp/test_config_mat_size.txt";
  std::ofstream ofs(path);
  ofs << "material_rng_seed: \n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");
  std::filesystem::remove(path);
}

// Pruebas de valores inválidos para diferentes claves
TEST(test_config, invalid_material_rng_seed_type) {
  std::string const path = "/tmp/test_config_mat_type.txt";
  std::ofstream ofs(path);
  ofs << "material_rng_seed: one\n";
  ofs.close();

  Config cfg;
  EXPECT_EXIT(cfg.load_config(path), ::testing::ExitedWithCode(EXIT_FAILURE),
              "Error: Invalid value for key");
  std::filesystem::remove(path);
}
