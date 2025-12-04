#include <gtest/gtest.h>
#include <image_aos.hpp>
#include <pixel.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

TEST(ImageAOS, ConstructorAndSize) {
  ImageAOS const img(40, 20);
  EXPECT_EQ(img.width(), 40);
  EXPECT_EQ(img.height(), 20);
}

TEST(ImageAOS, SetGetPixelRoundTrip) {
  ImageAOS img(32, 16);
  Pixel const p{12, 34, 56};
  img.set_pixel(10, 5, p);  // x=10 (col), y=5 (row)
  Pixel const got = img.get_pixel(10, 5);
  EXPECT_EQ(got.r, 12);
  EXPECT_EQ(got.g, 34);
  EXPECT_EQ(got.b, 56);
}

TEST(ImageAOS, OutOfRangeThrows) {
  ImageAOS img(10, 5);
  EXPECT_THROW(img.set_pixel(-1, 0, Pixel{0, 0, 0}), std::out_of_range);
  EXPECT_THROW(img.set_pixel(0, -1, Pixel{0, 0, 0}), std::out_of_range);
  EXPECT_THROW(img.set_pixel(10, 0, Pixel{0, 0, 0}), std::out_of_range);  // x == width
  EXPECT_THROW(img.set_pixel(0, 5, Pixel{0, 0, 0}), std::out_of_range);   // y == height
}

// Límite: constructor con tamaños no positivos debe fallar
TEST(ImageAOS, ConstructorRejectsNonPositive) {
  EXPECT_ANY_THROW(ImageAOS(0, 10));
  EXPECT_ANY_THROW(ImageAOS(10, 0));
  EXPECT_ANY_THROW(ImageAOS(-1, 5));
  EXPECT_ANY_THROW(ImageAOS(5, -1));
}

// Límite: set/get en la esquina inferior-derecha (último píxel)
TEST(ImageAOS, BoundarySetGetPixel) {
  ImageAOS img(3, 2);
  Pixel const p{255, 128, 0};
  img.set_pixel(2, 1, p);  // last column, last row
  Pixel const got = img.get_pixel(2, 1);
  EXPECT_EQ(got.r, 255);
  EXPECT_EQ(got.g, 128);
  EXPECT_EQ(got.b, 0);
}

// Comprueba orden row-major en write_ppm_p3 para 2x2 (relación de orden)
TEST(ImageAOS, WritePPMOrder2x2) {
  ImageAOS img(2, 2);
  // Fill row-major: (x=0,y=0), (x=1,y=0), (x=0,y=1), (x=1,y=1)
  img.set_pixel(0, 0, Pixel{1, 2, 3});
  img.set_pixel(1, 0, Pixel{4, 5, 6});
  img.set_pixel(0, 1, Pixel{7, 8, 9});
  img.set_pixel(1, 1, Pixel{10, 11, 12});

  std::ostringstream os;
  img.write_ppm_p3(os);
  std::string out = os.str();

  auto pos = [&](char const * s) {
    auto p = out.find(s);
    return p == std::string::npos ? std::string::npos : p;
  };

  auto p1 = pos("1 2 3");
  auto p2 = pos("4 5 6");
  auto p3 = pos("7 8 9");
  auto p4 = pos("10 11 12");

  EXPECT_NE(p1, std::string::npos);
  EXPECT_NE(p2, std::string::npos);
  EXPECT_NE(p3, std::string::npos);
  EXPECT_NE(p4, std::string::npos);

  // Comprobar el orden esperado row-major
  EXPECT_LT(p1, p2);
  EXPECT_LT(p2, p3);
  EXPECT_LT(p3, p4);
}
