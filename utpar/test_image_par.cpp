#include <gtest/gtest.h>
#include <image_par.hpp>
#include <pixel.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

// Prueba el constructor y la verificación de las dimensiones (ancho y alto)
TEST(ImageSOA, ConstructorYDimensiones) {
  ImageSOA const img(40, 20);
  EXPECT_EQ(img.width(), 40);
  EXPECT_EQ(img.height(), 20);
}

// Prueba que establecer un píxel y luego recuperarlo da el mismo valor (viaje de ida y vuelta)
TEST(ImageSOA, SetGetPixelIdaVuelta) {
  ImageSOA img(32, 16);
  Pixel const p{12, 34, 56};
  img.set_pixel(10, 5, p);  // x=10 (columna), y=5 (fila)
  Pixel const obtenido = img.get_pixel(10, 5);
  EXPECT_EQ(obtenido.r, 12);
  EXPECT_EQ(obtenido.g, 34);
  EXPECT_EQ(obtenido.b, 56);
}

// Prueba que las coordenadas fuera de rango lanzan una excepción
TEST(ImageSOA, FueraDeRangoLanzaExcepcion) {
  ImageSOA img(10, 5);
  // Coordenadas negativas
  EXPECT_THROW(img.set_pixel(-1, 0, Pixel{0, 0, 0}), std::out_of_range);
  EXPECT_THROW(img.set_pixel(0, -1, Pixel{0, 0, 0}), std::out_of_range);
  // Coordenadas iguales a ancho o alto (índices [0..w-1], [0..h-1])
  EXPECT_THROW(img.set_pixel(10, 0, Pixel{0, 0, 0}), std::out_of_range);  // x == width
  EXPECT_THROW(img.set_pixel(0, 5, Pixel{0, 0, 0}), std::out_of_range);   // y == height
}

// Límite: el constructor con dimensiones no positivas debe fallar
TEST(ImageSOA, ConstructorRechazaNoPositivas) {
  EXPECT_ANY_THROW(ImageSOA(0, 10));
  EXPECT_ANY_THROW(ImageSOA(10, 0));
  EXPECT_ANY_THROW(ImageSOA(-1, 5));
  EXPECT_ANY_THROW(ImageSOA(5, -1));
}

// Límite: set/get en la esquina inferior-derecha (último píxel válido)
TEST(ImageSOA, LimiteSetGetPixel) {
  ImageSOA img(3, 2);
  Pixel const p{255, 128, 0};
  img.set_pixel(2, 1, p);  // última columna (x=2), última fila (y=1)
  Pixel const obtenido = img.get_pixel(2, 1);
  EXPECT_EQ(obtenido.r, 255);
  EXPECT_EQ(obtenido.g, 128);
  EXPECT_EQ(obtenido.b, 0);
}

// Comprueba el orden row-major en write_ppm_p3 para una imagen de 2x2
TEST(ImageSOA, EscribirPPMOrden2x2) {
  ImageSOA img(2, 2);
  // Rellenar en orden row-major: (x=0,y=0), (x=1,y=0), (x=0,y=1), (x=1,y=1)
  img.set_pixel(0, 0, Pixel{1, 2, 3});
  img.set_pixel(1, 0, Pixel{4, 5, 6});
  img.set_pixel(0, 1, Pixel{7, 8, 9});
  img.set_pixel(1, 1, Pixel{10, 11, 12});

  std::ostringstream os;
  img.write_ppm_p3(os);
  std::string salida = os.str();

  // Función lambda auxiliar para encontrar la posición de la cadena de píxel
  auto pos = [&](char const * s) {
    auto p = salida.find(s);
    return p == std::string::npos ? std::string::npos : p;
  };

  auto p1 = pos("1 2 3");
  auto p2 = pos("4 5 6");
  auto p3 = pos("7 8 9");
  auto p4 = pos("10 11 12");

  // Asegurarse de que se encuentran todas las cadenas de píxel
  EXPECT_NE(p1, std::string::npos);
  EXPECT_NE(p2, std::string::npos);
  EXPECT_NE(p3, std::string::npos);
  EXPECT_NE(p4, std::string::npos);

  // Comprobar el orden esperado row-major (p1 antes que p2, p2 antes que p3, etc.)
  EXPECT_LT(p1, p2);  // (0,0) antes que (1,0)
  EXPECT_LT(p2, p3);  // Fin de la primera fila antes del inicio de la segunda
  EXPECT_LT(p3, p4);  // (0,1) antes que (1,1)
}
