#ifndef IMAGE_AOS_HPP
#define IMAGE_AOS_HPP

#include <ostream>
#include <pixel.hpp>
#include <vector>

class ImageAOS {
public:
  ImageAOS(int width, int height);
  [[nodiscard]] int width() const noexcept;
  [[nodiscard]] int height() const noexcept;

  void set_pixel(int x, int y, Pixel color);
  [[nodiscard]] Pixel get_pixel(int x, int y) const;

  /**
   * Write the image in PPM P3 format to the given output stream.
   * The format written matches the specification used in the course:
   *   P3
   *   <width> <height>
   *   255
   *   R G B  (one pixel per line, row-major order starting at row 0 col 0 which is top-left)
   */
  void write_ppm_p3(std::ostream & os) const;

private:
  int w_;
  int h_;
  std::vector<Pixel> pixels_;  // size = w_ * h_
  static inline void validate_coords(int x, int y, int w, int h);
};

#endif
