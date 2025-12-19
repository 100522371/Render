#include <camera.hpp>
#include <config.hpp>
#include <exception>
#include <fstream>
#include <image_par.hpp>
#include <iostream>
#include <render-par.hpp>
#include <scene.hpp>
#include <string>
#include <vector>

// Programa principal para renderizar una escena a partir de un archivo de configuración y un archivo de escena.
int main(int argc, char * argv[]) {
  std::vector<std::string> args(argv, argv + argc);
  if (argc != 4) {
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
