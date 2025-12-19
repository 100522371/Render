#include <cstddef>
#include <image_par.hpp>
#include <iostream>
#include <pixel.hpp>
#include <sstream>
#include <stdexcept>

// Función auxiliar para validar coordenadas de píxeles
inline void ImageSOA::validate_coords(int x, int y, int w, int h) {
  if (x < 0 or x >= w or y < 0 or y >= h) {
    std::ostringstream oss;
    oss << "pixel coordinates out of range: (" << x << "," << y << ") for size " << w << "x" << h;
    throw std::out_of_range(oss.str());
  }
}

// Constructor de la clase ImageSOA
ImageSOA::ImageSOA(int width, int height) : w_(width), h_(height) {
  if (w_ <= 0 or h_ <= 0) {
    throw std::invalid_argument("width and height must be positive");
  }
  std::size_t const n = static_cast<std::size_t>(w_) * static_cast<std::size_t>(h_);
  R_.assign(n, 0);
  G_.assign(n, 0);
  B_.assign(n, 0);
}

// Métodos de la clase ImageSOA altura y ancho
int ImageSOA::width() const noexcept {
  return w_;
}

int ImageSOA::height() const noexcept {
  return h_;
}

// Métodos para establecer y obtener píxeles
void ImageSOA::set_pixel(int x, int y, Pixel color) {
  validate_coords(x, y, w_, h_);
  std::size_t const idx =
      static_cast<std::size_t>(y) * static_cast<std::size_t>(w_) + static_cast<std::size_t>(x);
  R_[idx] = color.r;
  G_[idx] = color.g;
  B_[idx] = color.b;
}

Pixel ImageSOA::get_pixel(int x, int y) const {
  validate_coords(x, y, w_, h_);
  std::size_t const idx =
      static_cast<std::size_t>(y) * static_cast<std::size_t>(w_) + static_cast<std::size_t>(x);
  return Pixel{R_[idx], G_[idx], B_[idx]};
}

// Método para escribir la imagen en formato PPM (P3)
void ImageSOA::write_ppm_p3(std::ostream & os) const {
  os << "P3\n" << w_ << ' ' << h_ << '\n' << "255\n";
  for (int y = 0; y < h_; ++y) {
    for (int x = 0; x < w_; ++x) {
      std::size_t const idx =
          static_cast<std::size_t>(y) * static_cast<std::size_t>(w_) + static_cast<std::size_t>(x);
      os << int(R_[idx]) << ' ' << int(G_[idx]) << ' ' << int(B_[idx]) << '\n';
    }
  }
}
