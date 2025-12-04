#ifndef SCENE_HPP
#define SCENE_HPP

#include <intersection.hpp>
#include <material.hpp>
#include <memory>
#include <object.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct Scene {
  // Materiales disponibles en la escena, indexados por nombre
  std::unordered_map<std::string, std::unique_ptr<Material>> materials;
  // Objetos presentes en la escena
  std::vector<std::unique_ptr<render::Object3D>> objects;

  void load_scene(std::string const & path);

  // Devuelve un puntero al material con el nombre dado, o nullptr si no existe
  Material const * materialByName(std::string const & name) const;
  std::optional<render::Intersection> intersect(render::ray const & r) const;
};

#endif
