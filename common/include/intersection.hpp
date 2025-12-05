#ifndef RENDER_INTERSECTION_HPP
#define RENDER_INTERSECTION_HPP

#include <string>
#include <vector.hpp>

namespace render {

  struct Intersection {
    vector punto_interseccion;
    vector vector_normal;
    double lambda;
    std::string nombre_material;

    Intersection(vector p, vector n, double l, std::string m)
        : punto_interseccion{p}, vector_normal{n}, lambda{l}, nombre_material{std::move(m)} { }
  };

}  // namespace render

#endif
