#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP

#include "model.hpp"
#include <vector>
#include <string>
#include <fstream>

std::vector<std::string> split_csv(const std::string& line);
std::vector<Estudiante> leer_estudiantes(const std::string& archivo);
std::vector<RespuestaCorrecta> leer_correctas(const std::string& archivo);
std::vector<RespuestaEstudiante> leer_respuestas(const std::string& archivo);
std::vector<RespuestaEstudiante> leer_chunk(std::ifstream& file, size_t max_lines);

void escribir_resultados(
    const std::vector<Resultado>& resultados, 
    const std::string& archivo, 
    bool write_header = true
);

#endif // CSV_PARSER_HPP
