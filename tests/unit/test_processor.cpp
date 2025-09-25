#include "model.hpp"
#include "processor.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

void test_procesar() {
    // Crear datos mock
    std::vector<Estudiante> estudiantes = {
        {"123", "M", "2000-01-01", "Juan", "Perez", "Norte", 6.5}
    };

    std::vector<RespuestaCorrecta> correctas = {
        {"Matematicas", std::vector<char>(100, 'A')} // Todas correctas A
    };

    std::vector<RespuestaEstudiante> respuestas = {
        {"123", "Matematicas", std::vector<std::string>(100, "A")} // Todas A, buenas=100
    };

    auto resultados = procesar(estudiantes, respuestas, correctas);

    assert(resultados.size() == 1);
    auto& r = resultados[0];
    assert(r.codigo_estudiante == "123");
    assert(r.prueba == "Matematicas");
    assert(r.buenas == 100);
    assert(r.omitidas == 0);
    assert(r.malas == 0);
    assert(r.puntaje == 1000.0); // 100 + 100/100 * 900 = 1000
    assert(std::abs(r.pes - 928.5714286) < 0.0001); // (6.5/7)*1000 ≈ 928.5714286

    std::cout << "test_procesar aprobado" << std::endl;
}

int main() {
    test_procesar();
    std::cout << "Todas las pruebas unitarias para processor pasaron!" << std::endl;
    return 0;
}