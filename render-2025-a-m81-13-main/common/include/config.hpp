#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector.hpp>

struct Config {
public:
  // Defaults
  int aspect_w    = 16;
  int aspect_h    = 9;
  int image_width = 1'920;
  double gamma    = 2.2;
  render::vector camera_position{0.0, 0.0, -10.0};
  render::vector camera_target{0.0, 0.0, 0.0};
  render::vector camera_north{0.0, 1.0, 0.0};
  double field_of_view       = 90.0;
  int samples_per_pixel      = 20;
  int max_depth              = 5;
  uint64_t material_rng_seed = 13;
  uint64_t ray_rng_seed      = 19;

  render::vector background_dark_color{0.25, 0.5, 1.0};
  render::vector background_light_color{1.0, 1.0, 1.0};

  Config() = default;

  void load_config(std::string const & path);

  // Para monitorear las parámetros vistos al parsear el archivo
  [[nodiscard]] std::unordered_map<std::string, int> const & seen_keys() const noexcept {
    return _seen;
  }

private:
  //  metodos auxiliares para setear los valores
  void set_aspect_ratio(std::string const & raw, std::string const & rest);
  void set_image_width(std::string const & raw, std::string const & rest);
  void set_gamma(std::string const & raw, std::string const & rest);
  void set_camera_position(std::string const & raw, std::string const & rest);
  void set_camera_target(std::string const & raw, std::string const & rest);
  void set_camera_north(std::string const & raw, std::string const & rest);
  void set_field_of_view(std::string const & raw, std::string const & rest);
  void set_samples_per_pixel(std::string const & raw, std::string const & rest);
  void set_max_depth(std::string const & raw, std::string const & rest);
  void set_material_rng_seed(std::string const & raw, std::string const & rest);
  void set_ray_rng_seed(std::string const & raw, std::string const & rest);
  void set_background_dark_color(std::string const & raw, std::string const & rest);
  void set_background_light_color(std::string const & raw, std::string const & rest);

  // Para monitorear los parámetros vistos
  std::unordered_map<std::string, int> _seen;
};

#endif
