#ifndef MODEL_HPP
#define MODEL_HPP

#include <string>
#include <vector>

struct Estudiante {
    std::string codigo;
    std::string genero;
    std::string fecha_nac;
    std::string nombres;
    std::string apellidos;
    std::string region;
    double promedio_notas;
};

struct RespuestaCorrecta {
    std::string prueba;
    std::vector<char> respuestas; // 100 respuestas correctas
};

struct RespuestaEstudiante {
    std::string estudiante;
    std::string prueba;
    std::vector<std::string> respuestas; // 100 respuestas, puede ser "" para omitida
};

struct Resultado {
    std::string codigo_estudiante;
    double pes;
    std::string prueba;
    int buenas;
    int omitidas;
    int malas;
    double puntaje;
};

#endif // MODEL_HPP
