#ifndef PIXEL_HPP
#define PIXEL_HPP

struct Pixel {
  int r;
  int g;
  int b;

  bool operator==(Pixel const & o) const noexcept { return r == o.r and g == o.g and b == o.b; }

  bool operator!=(Pixel const & o) const noexcept { return !(*this == o); }
};

#endif
