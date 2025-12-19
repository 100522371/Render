#ifndef IMAGE_PAR_HPP
#define IMAGE_PAR_HPP

#include <ostream>
#include <pixel.hpp>
#include <vector>

//Clase que representa una imagen en formato Structure of Arrays (SoA)
class ImageSOA {
public:
  ImageSOA(int width, int height);
  [[nodiscard]] int width() const noexcept;
  [[nodiscard]] int height() const noexcept;

  void set_pixel(int x, int y, Pixel color);
  [[nodiscard]] Pixel get_pixel(int x, int y) const;

  void write_ppm_p3(std::ostream & os) const;

private:
  int w_;
  int h_;
  std::vector<int> R_;
  std::vector<int> G_;
  std::vector<int> B_;
  static inline void validate_coords(int x, int y, int w, int h);
};
#endif
