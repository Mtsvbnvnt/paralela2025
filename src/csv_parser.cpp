#include "csv_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <algorithm>

std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ';')) {
        // Quitar comillas externas si existen
        if (field.size() >= 2 && field[0] == '"' && field.back() == '"') {
            field = field.substr(1, field.size() - 2);
        }
        fields.push_back(field);
    }
    return fields;
}

std::vector<Estudiante> leer_estudiantes(const std::string& archivo) {
    std::vector<Estudiante> estudiantes;
    std::ifstream file(archivo);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + archivo);
    }
    std::string line;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        auto fields = split_csv(line);
        if (fields.size() != 7) continue;
        Estudiante e;
        e.codigo = fields[0];
        e.genero = fields[1];
        e.fecha_nac = fields[2];
        e.nombres = fields[3];
        e.apellidos = fields[4];
        e.region = fields[5];
        std::string prom = fields[6];
        std::replace(prom.begin(), prom.end(), ',', '.');
        e.promedio_notas = std::stod(prom);
        estudiantes.push_back(e);
    }
    return estudiantes;
}

std::vector<RespuestaCorrecta> leer_correctas(const std::string& archivo) {
    std::vector<RespuestaCorrecta> correctas;
    std::ifstream file(archivo);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + archivo);
    }
    std::string line;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        auto fields = split_csv(line);
        if (fields.size() != 101) continue;
        RespuestaCorrecta rc;
        rc.prueba = fields[0];
        for (int i = 1; i <= 100; ++i) {
            rc.respuestas.push_back(fields[i][0]);
        }
        correctas.push_back(rc);
    }
    return correctas;
}

std::vector<RespuestaEstudiante> leer_chunk(std::ifstream& file, size_t max_lines) {
    std::vector<RespuestaEstudiante> respuestas;
    size_t count = 0;
    std::string line;
    while (count < max_lines && std::getline(file, line)) {
        auto fields = split_csv(line);
        if (fields.size() != 102) continue;
        RespuestaEstudiante re;
        re.estudiante = fields[0];
        re.prueba = fields[1];
        for (int i = 2; i < 102; ++i) {
            re.respuestas.push_back(fields[i]);
        }
        respuestas.push_back(re);
        count++;
    }
    return respuestas;
}

void escribir_resultados(const std::vector<Resultado>& resultados, const std::string& archivo, bool write_header) {
    std::ofstream file(archivo, write_header ? std::ios::out : std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + archivo);
    }
    if (write_header) {
        file << "\"CÓDIGO ESTUDIANTE\",\"PES\",\"PRUEBA\",\"BUENAS\",\"OMITIDAS\",\"MALAS\",\"PUNTAJE\"\n";
    }
    file << std::fixed;
    for (const auto& r : resultados) {
        file << "\"" << r.codigo_estudiante << "\","
             << "\"" << std::setprecision(7) << r.pes << "\","
             << "\"" << r.prueba << "\","
             << "\"" << r.buenas << "\","
             << "\"" << r.omitidas << "\","
             << "\"" << r.malas << "\","
             << "\"" << std::setprecision(2) << r.puntaje << "\"\n";
    }
}
