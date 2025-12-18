#include <camera.hpp>
#include <config.hpp>
#include <cstddef>
#include <exception>
#include <fstream>
#include <image_par.hpp>
#include <iostream>
#include <memory>
#include <oneapi/tbb/global_control.h>
#include <render-par.hpp>
#include <scene.hpp>
#include <string>
#include <vector>

using std::string;
// Definición de valores óptimos según pruebas de rendimiento --> constexpr
constexpr int grain_size           = 1;         // grano tamaño óptimo encontrado
constexpr char const * partitioner = "simple";  // particionador óptimo encontrado
constexpr int max_threads          = 64;        // número máximo de hilos óptimo encontrado

int main(int argc, char * argv[]) {
  // Ahora el programa solo requiere los 3 archivos básicos
  std::vector<std::string> args(argv, argv + argc);
  if (argc < 4) {
    std::cerr << "Uso: " << args.at(0) << " <config_file> <scene_file> <output_image>\n";
    return 1;
  }

  std::string const & config_file = args.at(1);
  std::string const & scene_file  = args.at(2);
  std::string const & output_file = args.at(3);
  Config config;
  Scene scene;
  try {
    config.load_config(config_file);
    scene.load_scene(scene_file);
    render::Camera camara(config);
    ImageSOA img(camara.ancho_imagen, camara.alto_imagen);
    // Renderiza la imagen y obtiene los píxeles en formato AOS

    // Configuramos el número máximo de hilos si se especificó
    std::unique_ptr<oneapi::tbb::global_control> control;
    if (max_threads > 0) {
      control = std::make_unique<oneapi::tbb::global_control>(
          oneapi::tbb::global_control::max_allowed_parallelism,
          static_cast<std::size_t>(max_threads));
    }

    render::render_image_soa(scene, config, camara, img, partitioner, grain_size);
    // Escribe la imagen en formato PPM (P3)
    std::ofstream out(output_file);
    if (!out) {
      std::cerr << "Error al abrir el archivo de salida: " << output_file << '\n';
      return 1;
    }
    img.write_ppm_p3(out);
    out.close();
    std::cout << "Imagen guardada en " << output_file << '\n';
  } catch (std::exception const & e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
