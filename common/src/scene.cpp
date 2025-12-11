#include <array>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cylinder.hpp>
#include <fstream>
#include <intersection.hpp>
#include <iostream>
#include <limits>
#include <material.hpp>
#include <memory>
#include <object.hpp>
#include <optional>
#include <ray.hpp>
#include <scene.hpp>
#include <sphere.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector.hpp>
#include <vector>

// helpers locales
namespace {

  using std::isspace;

  std::string trim_copy(std::string const & s) {
    size_t a = 0;
    while (a < s.size() and (isspace((unsigned char) s[a]) != 0)) {
      ++a;
    }
    size_t b = s.size();
    while (b > a and (isspace((unsigned char) s[b - 1]) != 0)) {
      --b;
    }
    return s.substr(a, b - a);
  }

  std::vector<std::string> split_ws(std::string const & s) {
    std::istringstream iss(s);
    std::vector<std::string> out;
    std::string t;
    while (iss >> t) {
      out.push_back(t);
    }
    return out;
  }

  bool is_number(std::string const & s) {
    std::istringstream iss(s);
    double d = 0.0;
    return (iss >> d) and (iss.peek() == EOF);
  }

  double to_double(std::string const & s) {
    return std::stod(s);
  }

  void throw_invalid_line(std::string const & raw, std::string const & kind) {
    throw std::runtime_error("Error: Invalid " + kind + " parameters\nLine: \"" + raw + "\"");
  }

  void throw_extra_line(std::string const & raw, std::string const & key,
                        std::string const & extra) {
    throw std::runtime_error("Error: Extra data after configuration value for key: [" +
                             key +
                             "]\nExtra: \"" +
                             extra +
                             "\"\nLine: \"" +
                             raw +
                             "\"");
  }

  void throw_unknown_entity(std::string const & tok) {
    throw std::runtime_error("Error: Unknown scene entity: " + tok);
  }

  void throw_material_exists(std::string const & raw, std::string const & name) {
    throw std::runtime_error(
        "Error: Material with name [" + name + "] already exists\nLine: \"" + raw + "\"");
  }

  void throw_material_not_found(std::string const & raw, std::string const & name) {
    throw std::runtime_error("Error: Material not found: [" + name + "]\n\nLine: \"" + raw + "\"");
  }

  // parsers para cada etiqueta (devuelven true si procesaron la línea)
  bool parse_matte(Scene & scene, std::vector<std::string> const & v, std::string const & raw) {
    if (v.size() != 4) {
      if (v.size() < 4) {
        throw_invalid_line(raw, "matte material");
      } else {
        throw_extra_line(raw, "matte:", v[4]);
      }
    }
    std::string const & name = v[0];
    if (scene.materials.contains(name)) {
      throw_material_exists(raw, name);
    }
    if (!is_number(v[1]) or !is_number(v[2]) or !is_number(v[3])) {
      throw_invalid_line(raw, "matte");
    }
    scene.materials[name] = std::make_unique<Matte>(
        name, render::vector{to_double(v[1]), to_double(v[2]), to_double(v[3])});
    return true;
  }

  bool parse_metal(Scene & scene, std::vector<std::string> const & v, std::string const & raw) {
    if (v.size() != 5) {
      if (v.size() < 5) {
        throw_invalid_line(raw, "metal material");
      } else {
        throw_extra_line(raw, "metal:", v[5]);
      }
    }
    std::string const & name = v[0];
    if (scene.materials.contains(name)) {
      throw_material_exists(raw, name);
    }
    for (std::size_t i = 1; i <= 4; ++i) {
      if (!is_number(v[i])) {
        throw_invalid_line(raw, "metal");
      }
    }
    scene.materials[name] = std::make_unique<Metal>(
        name, render::vector{to_double(v[1]), to_double(v[2]), to_double(v[3])}, to_double(v[4]));
    return true;
  }

  bool parse_refractive(Scene & scene, std::vector<std::string> const & v,
                        std::string const & raw) {
    if (v.size() != 2) {
      if (v.size() < 2) {
        throw_invalid_line(raw, "refractive material");
      } else {
        throw_extra_line(raw, "refractive:", v[2]);
      }
    }
    std::string const & name = v[0];
    if (scene.materials.contains(name)) {
      throw_material_exists(raw, name);
    }
    if (!is_number(v[1])) {
      throw_invalid_line(raw, "refractive");
    }
    scene.materials[name] = std::make_unique<Refractive>(name, to_double(v[1]));
    return true;
  }

  bool parse_sphere(Scene & scene, std::vector<std::string> const & v, std::string const & raw) {
    if (v.size() != 5) {
      if (v.size() < 5) {
        throw_invalid_line(raw, "sphere");
      } else {
        throw_extra_line(raw, "sphere:", v[5]);
      }
    }
    for (std::size_t i = 0; i < 4; ++i) {
      if (!is_number(v[i])) {
        throw_invalid_line(raw, "sphere");
      }
    }
    double const r = to_double(v[3]);
    if (r <= 0) {
      throw_invalid_line(raw, "sphere");
    }
    if (!scene.materials.contains(v[4])) {
      throw_material_not_found(raw, v[4]);
    }
    scene.objects.emplace_back(std::make_unique<render::Sphere>(
        render::vector{to_double(v[0]), to_double(v[1]), to_double(v[2])}, r, v[4]));
    return true;
  }

  bool parse_cylinder(Scene & scene, std::vector<std::string> const & v, std::string const & raw) {
    if (v.size() != 8) {
      if (v.size() < 8) {
        throw_invalid_line(raw, "cylinder");
      } else {
        throw_extra_line(raw, "cylinder:", v[8]);
      }
    }
    for (std::size_t i = 0; i < 7; ++i) {
      if (!is_number(v[i])) {
        throw_invalid_line(raw, "cylinder");
      }
    }
    double const r = to_double(v[3]);
    if (r <= 0) {
      throw_invalid_line(raw, "cylinder");
    }
    if (!scene.materials.contains(v[7])) {
      throw_material_not_found(raw, v[7]);
    }
    scene.objects.emplace_back(std::make_unique<render::Cylinder>(
        render::vector{to_double(v[0]), to_double(v[1]), to_double(v[2])}, r,
        render::vector{to_double(v[4]), to_double(v[5]), to_double(v[6])}, v[7]));
    return true;
  }

  // tabla de dispatch para los parsers
  using Parser = bool (*)(Scene &, std::vector<std::string> const &, std::string const &);

  std::array<std::pair<char const *, Parser>, 5> const & scene_parsers() {
    static std::array<std::pair<char const *, Parser>, 5> const parsers = {
      {
       {"matte", parse_matte},
       {"metal", parse_metal},
       {"refractive", parse_refractive},
       {"sphere", parse_sphere},
       {"cylinder", parse_cylinder},
       }
    };
    return parsers;
  }

}  // namespace

void Scene::load_scene(std::string const & path) {
  std::ifstream ifs(path);
  if (!ifs) {
    throw std::runtime_error("Error: Could not open scene file");
  }
  std::string line;
  while (std::getline(ifs, line)) {
    std::string const raw = line;
    line                  = trim_copy(line);
    if (line.empty()) {
      continue;
    }
    auto tok = split_ws(line);
    if (tok.empty()) {
      continue;
    }
    std::string label = tok[0];
    if (label.back() != ':') {
      throw_unknown_entity(tok[0]);
    }
    std::string const tag = label.substr(0, label.size() - 1);
    std::vector<std::string> const v(tok.begin() + 1, tok.end());

    // find parser in small dispatch table
    Parser parser = nullptr;
    for (auto const & p : scene_parsers()) {
      if (tag == p.first) {
        parser = p.second;
        break;
      }
    }
    if (parser == nullptr) {
      throw_unknown_entity(tag);
    }
    parser(*this, v, raw);
  }
}

std::optional<render::Intersection> Scene::intersect(render::ray const & r) const {
  std::optional<render::Intersection> closest_hit;
  double closest_lambda = std::numeric_limits<double>::infinity();

  double const min_lambda = 1e-3;

  for (auto const & obj : objects) {
    auto hit = obj->collision(r);
    if (hit and hit->lambda > min_lambda and hit->lambda < closest_lambda) {
      closest_lambda = hit->lambda;
      closest_hit    = hit;
    }
  }

  return closest_hit;
}

Material const * Scene::materialByName(std::string const & name) const {
  auto it = materials.find(name);
  if (it == materials.end()) {
    return nullptr;
  }
  return it->second.get();
}
