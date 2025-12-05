#include <cstddef>
#include <image_aos.hpp>
#include <iostream>
#include <pixel.hpp>
#include <sstream>
#include <stdexcept>

inline void ImageAOS::validate_coords(int x, int y, int w, int h) {
  if (x < 0 or x >= w or y < 0 or y >= h) {
    std::ostringstream oss;
    oss << "pixel coordinates out of range: (" << x << "," << y << ") for size " << w << "x" << h;
    throw std::out_of_range(oss.str());
  }
}

ImageAOS::ImageAOS(int width, int height) : w_(width), h_(height) {
  if (w_ <= 0 or h_ <= 0) {
    throw std::invalid_argument("width and height must be positive");
  }
  pixels_.assign(static_cast<size_t>(w_) * static_cast<size_t>(h_), Pixel{0, 0, 0});
}

int ImageAOS::width() const noexcept {
  return w_;
}

int ImageAOS::height() const noexcept {
  return h_;
}

void ImageAOS::set_pixel(int x, int y, Pixel color) {
  validate_coords(x, y, w_, h_);
  // map (x=column, y=row) to row-major index: idx = row * width + col
  std::size_t const idx =
      static_cast<std::size_t>(y) * static_cast<std::size_t>(w_) + static_cast<std::size_t>(x);
  pixels_[idx] = color;
}

Pixel ImageAOS::get_pixel(int x, int y) const {
  validate_coords(x, y, w_, h_);
  std::size_t const idx =
      static_cast<std::size_t>(y) * static_cast<std::size_t>(w_) + static_cast<std::size_t>(x);
  return pixels_[idx];
}

void ImageAOS::write_ppm_p3(std::ostream & os) const {
  // Header
  os << "P3\n" << w_ << ' ' << h_ << '\n' << "255\n";
  // Pixels: one pixel per line: "R G B\n"
  for (int y = 0; y < h_; ++y) {
    for (int x = 0; x < w_; ++x) {
      Pixel const c = get_pixel(x, y);
      os << c.r << ' ' << c.g << ' ' << c.b << '\n';
    }
  }
}
