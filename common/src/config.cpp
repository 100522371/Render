#include <cctype>
#include <cmath>
#include <config.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector.hpp>
#include <vector>

namespace {

  // Funciones auxiliares
  inline std::string trim(std::string const & s) {
    size_t const a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) {
      return "";
    }
    size_t const b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
  }

  inline std::vector<std::string> split_ws(std::string const & s) {
    std::vector<std::string> out;
    std::string cur;
    std::stringstream iss(s);
    while (iss >> cur) {
      out.push_back(cur);
    }
    return out;
  }

  [[noreturn]] void error_exit(std::string const & msg, std::string const & line = "") {
    if (line.empty()) {
      std::cerr << msg << "\n";
    } else {
      std::cerr << msg << "\nLine: \"" << line << "\"\n";
    }
    std::exit(EXIT_FAILURE);
  }

}  // namespace

void Config::load_config(std::string const & path) {
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    error_exit("Error: Cannot open configuration file: " + path);
  }
  using Handler = void (Config::*)(std::string const &, std::string const &);  // puntero a metodos
  static std::unordered_map<std::string, Handler> const handlers = {
    {          "aspect_ratio:",           &Config::set_aspect_ratio},
    {           "image_width:",            &Config::set_image_width},
    {                 "gamma:",                  &Config::set_gamma},
    {       "camera_position:",        &Config::set_camera_position},
    {         "camera_target:",          &Config::set_camera_target},
    {          "camera_north:",           &Config::set_camera_north},
    {         "field_of_view:",          &Config::set_field_of_view},
    {     "samples_per_pixel:",      &Config::set_samples_per_pixel},
    {             "max_depth:",              &Config::set_max_depth},
    {     "material_rng_seed:",      &Config::set_material_rng_seed},
    {          "ray_rng_seed:",           &Config::set_ray_rng_seed},
    { "background_dark_color:",  &Config::set_background_dark_color},
    {"background_light_color:", &Config::set_background_light_color}
  };  // tabla de handlers
  std::string raw;
  while (std::getline(ifs, raw)) {
    std::string const line = trim(raw);
    if (line.empty()) {
      continue;
    }
    size_t const colon = line.find(':');
    if (colon == std::string::npos) {
      error_exit("Error: Unknown configuration key: [" + line + " ]");
    }
    std::string const key  = trim(line.substr(0, colon + 1));
    std::string const rest = trim(line.substr(colon + 1));
    auto it                = handlers.find(key);
    if (it == handlers.end()) {
      error_exit("Error: Unknown configuration key: [" + key + " ]");
    }
    Handler const h = it->second;  // call handler
    (this->*h)(raw, rest);
  }
}

void Config::set_aspect_ratio(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 2) {
    error_exit("Error: Invalid value for key: [aspect_ratio: ]", raw);
  }
  try {
    int const w = stoi(t[0]);
    int const h = stoi(t[1]);
    if (w <= 0 or h <= 0) {
      throw std::logic_error("non-positive");
    }
    aspect_w = w;
    aspect_h = h;
    _seen["aspect_ratio:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [aspect_ratio: ]", raw);
  }
}

void Config::set_image_width(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [image_width: ]", raw);
  }
  try {
    int const w = stoi(t[0]);
    if (w <= 0) {
      throw std::logic_error("non-positive");
    }
    image_width = w;
    _seen["image_width:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [image_width: ]", raw);
  }
}

void Config::set_gamma(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [gamma: ]", raw);
  }
  try {
    double const g = stod(t[0]);
    if (g <= 0) {
      throw std::logic_error("non-positive");
    }
    gamma = g;
    _seen["gamma:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [gamma: ]", raw);
  }
}

void Config::set_camera_position(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 3) {
    error_exit("Error: Invalid value for key: [camera_position: ]", raw);
  }
  try {
    camera_position.x = stod(t[0]);
    camera_position.y = stod(t[1]);
    camera_position.z = stod(t[2]);
    _seen["camera_position:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [camera_position: ]", raw);
  }
}

void Config::set_camera_target(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 3) {
    error_exit("Error: Invalid value for key: [camera_target: ]", raw);
  }
  try {
    camera_target.x = stod(t[0]);
    camera_target.y = stod(t[1]);
    camera_target.z = stod(t[2]);
    _seen["camera_target:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [camera_target: ]", raw);
  }
}

void Config::set_camera_north(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 3) {
    error_exit("Error: Invalid value for key: [camera_north: ]", raw);
  }
  try {
    camera_north.x = stod(t[0]);
    camera_north.y = stod(t[1]);
    camera_north.z = stod(t[2]);
    _seen["camera_north:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [camera_north: ]", raw);
  }
}

void Config::set_field_of_view(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [field_of_view: ]", raw);
  }
  try {
    field_of_view = stod(t[0]);
    _seen["field_of_view:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [field_of_view: ]", raw);
  }
}

void Config::set_samples_per_pixel(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [samples_per_pixel: ]", raw);
  }
  try {
    int const v = stoi(t[0]);
    if (v <= 0) {
      throw std::logic_error("non-positive");
    }
    samples_per_pixel = v;
    _seen["samples_per_pixel:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [samples_per_pixel: ]", raw);
  }
}

void Config::set_max_depth(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [max_depth: ]", raw);
  }
  try {
    int const v = stoi(t[0]);
    if (v <= 0) {
      throw std::logic_error("non-positive");
    }
    max_depth = v;
    _seen["max_depth:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [max_depth: ]", raw);
  }
}

void Config::set_material_rng_seed(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [material_rng_seed: ]", raw);
  }
  try {
    long long const v = stoll(t[0]);
    if (v <= 0) {
      throw std::logic_error("non-positive");
    }
    material_rng_seed = static_cast<std::uint64_t>(v);
    _seen["material_rng_seed:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [material_rng_seed: ]", raw);
  }
}

void Config::set_ray_rng_seed(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 1) {
    error_exit("Error: Invalid value for key: [ray_rng_seed: ]", raw);
  }
  try {
    unsigned long long const v = stoull(t[0]);
    if (v == 0) {
      error_exit("Error: Invalid value for key: [ray_rng_seed: ]", raw);
    }
    ray_rng_seed = static_cast<std::uint64_t>(v);
    _seen["ray_rng_seed:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [ray_rng_seed: ]", raw);
  }
}

void Config::set_background_dark_color(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 3) {
    error_exit("Error: Invalid value for key: [background_dark_color: ]", raw);
  }
  try {
    double const a = stod(t[0]), b = stod(t[1]), c = stod(t[2]);
    if (a < 0 or a > 1 or b < 0 or b > 1 or c < 0 or c > 1) {
      throw std::logic_error("range");
    }
    background_dark_color = {a, b, c};
    _seen["background_dark_color:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [background_dark_color: ]", raw);
  }
}

void Config::set_background_light_color(std::string const & raw, std::string const & rest) {
  std::vector<std::string> t = split_ws(rest);
  if (t.size() != 3) {
    error_exit("Error: Invalid value for key: [background_light_color: ]", raw);
  }
  try {
    double const a = stod(t[0]), b = stod(t[1]), c = stod(t[2]);
    if (a < 0 or a > 1 or b < 0 or b > 1 or c < 0 or c > 1) {
      throw std::logic_error("range");
    }
    background_light_color = {a, b, c};
    _seen["background_light_color:"]++;
  } catch (...) {
    error_exit("Error: Invalid value for key: [background_light_color: ]", raw);
  }
}
