#include "csv_parser.hpp"
#include "processor.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string estudiantes_file, paes_file, correctas_file, resultados_file;

    for (int i = 1; i < argc; i += 2) {
        if (std::string(argv[i]) == "-e") estudiantes_file = argv[i + 1];
        else if (std::string(argv[i]) == "-p") paes_file = argv[i + 1];
        else if (std::string(argv[i]) == "-c") correctas_file = argv[i + 1];
        else if (std::string(argv[i]) == "-r") resultados_file = argv[i + 1];
        else {
            std::cerr << "Argumento inválido: " << argv[i] << std::endl;
            return 1;
        }
    }

    if (estudiantes_file.empty() || paes_file.empty() || correctas_file.empty() || resultados_file.empty()) {
        std::cerr << "Faltan argumentos requeridos: -e, -p, -c, -r" << std::endl;
        return 1;
    }

    try {
        auto estudiantes = leer_estudiantes(estudiantes_file);
        auto correctas = leer_correctas(correctas_file);

        std::ifstream paes_stream(paes_file);
        if (!paes_stream.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo: " + paes_file);
        }
        std::string dummy_header;
        std::getline(paes_stream, dummy_header);

        bool first_chunk = true;
        size_t chunk_size = 10000;
        size_t total_resultados = 0;
        while (true) {
            auto respuestas_chunk = leer_chunk(paes_stream, chunk_size);
            if (respuestas_chunk.empty()) break;
            auto resultados = procesar(estudiantes, respuestas_chunk, correctas);
            escribir_resultados(resultados, resultados_file, first_chunk);
            first_chunk = false;
            total_resultados += resultados.size();
        }
        paes_stream.close();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Patricio Abarca" << std::endl;
    std::cout << "Rodrigo Tapia" << std::endl;
    std::cout << "Matias Villarroel" << std::endl;
    return 0;
}
