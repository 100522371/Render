#include <camera.hpp>
#include <config.hpp>
#include <exception>
#include <fstream>
#include <image_soa.hpp>
#include <iostream>
#include <render-soa.hpp>
#include <scene.hpp>
#include <string>
#include <vector>

int main(int argc, char * argv[]) {
  // Aceptamos hasta 6 argumentos (nombre del programa + 5 args)
  // 4: config, scene, output
  // 5: partitioner(si no se pone, sera el determinado)
  // 6: grain_size(si no se pone, sera el determinado)
  std::vector<std::string> args(argv, argv + argc);
  if (argc < 4 or argc > 6) {
    std::cerr << "Uso: " << args.at(0)
              << " <config_file> <scene_file> <output_image> [partitioner] [grain_size]\n";
    return 1;
  }
  std::string const & config_file = args.at(1);
  std::string const & scene_file  = args.at(2);
  std::string const & output_file = args.at(3);
  // valores por defecto
  std::string partitioner = "auto";  // Valor por defecto
  int grain_size          = 0;       // Valor por defecto (0 deja que TBB decida)
  // Si hay al menos 5 argumentos, leemos el particionador
  if (argc >= 5) {
    partitioner = args.at(4);
    std::cout << "Usando particionador: " << partitioner << "\n";
  }
  // Si hay 6 argumentos, leemos el tamaño de grano
  if (argc == 6) {
    try {
      grain_size = std::stoi(args.at(5));
      std::cout << "Usando tamaño de grano: " << grain_size << "\n";
    } catch (...) {
      std::cerr << "Error: El tamaño de grano debe ser un número entero.\n";
      return 1;
    }
  }

  Config config;
  Scene scene;
  try {
    config.load_config(config_file);
    scene.load_scene(scene_file);
    render::Camera camara(config);
    ImageSOA img(camara.ancho_imagen, camara.alto_imagen);
    // Renderiza la imagen y obtiene los píxeles en formato AOS
    render::render_image_soa(scene, config, camara, img);
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
