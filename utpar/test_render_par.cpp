#include <algorithm>
#include <camera.hpp>
#include <cmath>
#include <config.hpp>
#include <gtest/gtest.h>
#include <image_par.hpp>  // Usamos ImageSOA
#include <pixel.hpp>
#include <random>
#include <ray.hpp>
#include <render-par.hpp>  // Incluimos render-soa.hpp
#include <scene.hpp>
#include <vector.hpp>

// Comprueba que el constructor de RenderContext inicializa los punteros correctamente.
TEST(RenderSOA, RenderContextConstructorInitializesPointers) {
  Scene const escena;
  Config const cfg;
  render::Camera camara(cfg);
  ImageSOA img(1, 1);
  std::mt19937_64 rgen(123);
  std::mt19937_64 mgen(456);

  render::RenderContext const ctx{escena, cfg, camara, img, rgen, mgen};

  // Los miembros públicos usados en el código son 'escena' y 'config'
  EXPECT_EQ(ctx.escena, &escena);
  EXPECT_EQ(ctx.config, &cfg);
  EXPECT_EQ(ctx.camara, &camara);
  EXPECT_EQ(ctx.img, &img);
}

// profundidad == 0 debe devolver vector {0,0,0}
TEST(RenderSOA, SoaCalcularColor_DepthZeroReturnsBlack) {
  Scene const escena;
  Config const cfg;
  render::Camera camara(cfg);
  ImageSOA img(1, 1);
  std::mt19937_64 rgen(1);
  std::mt19937_64 mgen(2);
  render::RenderContext ctx{escena, cfg, camara, img, rgen, mgen};

  render::ray r;
  r.direction = render::vector{0.0, 0.0, 1.0};

  render::vector const got = render::soa_calcular_color(r, 0, ctx);

  EXPECT_DOUBLE_EQ(got.x, 0.0);
  EXPECT_DOUBLE_EQ(got.y, 0.0);
  EXPECT_DOUBLE_EQ(got.z, 0.0);
}

// Si no hay intersección, soa_calcular_color debe igualar al calcular el fondo
TEST(RenderSOA, SoaCalcularColor_NoIntersectionReturnsBackground) {
  Scene const escena;  // escena vacía -> no intersecciones
  Config cfg;
  cfg.background_dark_color  = render::vector{0.1, 0.2, 0.3};
  cfg.background_light_color = render::vector{0.9, 0.8, 0.7};

  render::Camera camara(cfg);
  ImageSOA img(1, 1);
  std::mt19937_64 rgen(11);
  std::mt19937_64 mgen(22);
  render::RenderContext ctx{escena, cfg, camara, img, rgen, mgen};

  render::ray r;
  r.direction = render::vector{0.0, 1.0, 0.0};  // apunta hacia arriba

  render::vector const got    = render::soa_calcular_color(r, 5, ctx);
  render::vector const expect = render::soa_calcular_fondo(r, cfg);

  EXPECT_DOUBLE_EQ(got.x, expect.x);
  EXPECT_DOUBLE_EQ(got.y, expect.y);
  EXPECT_DOUBLE_EQ(got.z, expect.z);
}

// Prueba de render_image_soa en caso simple sin objetos: cada píxel debe corresponder
// al color de fondo calculado para el rayo generado por la cámara.
TEST(RenderSOA, RenderImageSOA_NoObjectsProducesBackgroundPixels) {
  Scene const escena;  // sin objetos
  Config cfg;
  cfg.background_dark_color  = render::vector{0.05, 0.06, 0.07};
  cfg.background_light_color = render::vector{0.8, 0.7, 0.6};
  // Seeds fijos
  // Nota: render::render_image_soa crea su propio RenderContext con semillas del config.
  cfg.ray_rng_seed      = 12'345;
  cfg.material_rng_seed = 67'890;

  // Cámara pequeña y determinista para que el test sea reproducible
  render::Camera cam(cfg);
  cam.ancho_imagen = 2;
  cam.alto_imagen  = 1;

  ImageSOA img(2, 1);

  // Ejecutar render (no debe lanzar)
  ImageSOA const out = render::render_image_soa(escena, cfg, cam, img);
  (void) out;  // algunos builds devuelven img por valor; ignorar el return

  // Cámara de referencia que reproduce la misma secuencia interna de RNG usada por el renderer.
  render::Camera cam_ref(cfg);

  // Para cada píxel, reconstruimos el rayo que genera la cámara en la misma posición
  // y comprobamos el color esperado (fondo -> conversión a Pixel).
  for (int fila = 0; fila < cam.alto_imagen; ++fila) {
    for (int col = 0; col < cam.ancho_imagen; ++col) {
      // El renderer llama a camara.generar_ray(fila,col,0.0,0.0) y la propia Camera añade
      // jitter usando su RNG interno. Para reproducir exactamente los mismos rays,
      // usamos una cámara de referencia `cam_ref` inicializada igual que la pasada al
      // renderer.
      render::vector accum{0.0, 0.0, 0.0};
      for (int s = 0; s < std::max(1, cfg.samples_per_pixel); ++s) {
        // renderer pasa (0.0, 0.0) como offsets; Camera internamente consume su RNG
        render::ray const rr = cam_ref.generar_ray(fila, col, 0.0, 0.0);
        // escena vacía -> color = fondo
        render::vector const colv = render::soa_calcular_fondo(rr, cfg);
        accum                     = render::vector::add(accum, colv);
      }

      // Post-process: average, gamma (sqrt), clamp, scale to 0..255
      auto clamp01 = [](double v) {
        if (v < 0.0) {
          return 0.0;
        }
        if (v > 1.0) {
          return 1.0;
        }
        return v;
      };
      double scale = 1.0 / std::max(1, cfg.samples_per_pixel);
      auto to_byte = [&](double c) {
        double v = scale * c;
        v        = std::sqrt(v);
        return static_cast<int>(255.99 * clamp01(v));
      };
      int const er = to_byte(accum.x);
      int const eg = to_byte(accum.y);
      int const eb = to_byte(accum.z);

      Pixel const p = img.get_pixel(col, fila);  // get_pixel(x=col,y=row)
      // Permitir pequeñas diferencias por redondeo/gamma/muestreo (tolerancia en bytes)
      int const tol = 10;
      EXPECT_NEAR(static_cast<int>(p.r), er, tol);
      EXPECT_NEAR(static_cast<int>(p.g), eg, tol);
      EXPECT_NEAR(static_cast<int>(p.b), eb, tol);
    }
  }
}

// Caso inválido: la imagen pasada a render_image_soa es más pequeña que la cámara
TEST(RenderSOA, RenderImageSOA_MismatchedImageSizeThrows) {
  Scene const escena;
  Config const cfg;
  // cámara 2x1
  render::Camera cam(cfg);
  cam.ancho_imagen = 2;
  cam.alto_imagen  = 1;

  // img demasiado pequeña: 1x1
  ImageSOA img(1, 1);

  EXPECT_ANY_THROW(render::render_image_soa(escena, cfg, cam, img));
}

// soa_calcular_fondo debe normalizar la dirección del rayo:
// un rayo con dirección (0,2,0) -> normaliza a (0,1,0) -> mismo fondo que (0,1,0)
TEST(RenderSOA, SoaCalcularFondo_NormalizesDirection) {
  Config cfg;
  cfg.background_dark_color  = render::vector{0.11, 0.22, 0.33};
  cfg.background_light_color = render::vector{0.88, 0.77, 0.66};

  render::ray r_unit;
  r_unit.direction = render::vector{0.0, 1.0, 0.0};

  render::ray r_scaled;
  r_scaled.direction = render::vector{0.0, 2.0, 0.0};  // same direction, not unit

  auto a = render::soa_calcular_fondo(r_unit, cfg);
  auto b = render::soa_calcular_fondo(r_scaled, cfg);

  EXPECT_DOUBLE_EQ(a.x, b.x);
  EXPECT_DOUBLE_EQ(a.y, b.y);
  EXPECT_DOUBLE_EQ(a.z, b.z);
}
