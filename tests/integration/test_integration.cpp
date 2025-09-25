#include "csv_parser.hpp"
#include "processor.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <iomanip>

// Función helper para crear archivos de test temporales
void create_test_files() {
    // estudiantes_test.csv
    std::ofstream est_file("tests/integration/estudiantes_test.csv");
    est_file << "\"CÓDIGO\";\"GÉNERO\";\"FECHA DE NACIMIENTO\";\"NOMBRES\";\"APELLIDOS\";\"REGIÓN GEOGRÁFICA\";\"PROMEDIO DE NOTAS\"\n";
    est_file << "\"123\";\"M\";\"2000-01-01\";\"Juan\";\"Perez\";\"Norte\";\"6,5\"\n";
    est_file.close();

    // correctas_test.csv - 100 respuestas A
    std::ofstream corr_file("tests/integration/correctas_test.csv");
    corr_file << "\"PRUEBA\"";
    for(int i=1; i<=100; i++) corr_file << ";\"RESPUESTA " << std::setfill('0') << std::setw(3) << i << "\"";
    corr_file << "\n\"Matematicas\"";
    for(int i=0; i<100; i++) corr_file << ";\"A\"";
    corr_file << "\n";
    corr_file.close();

    // paes_test.csv - 100 respuestas A
    std::ofstream paes_file("tests/integration/paes_test.csv");
    paes_file << "\"ESTUDIANTE\";\"PRUEBA\"";
    for(int i=1; i<=100; i++) paes_file << ";\"PREGUNTA " << std::setfill('0') << std::setw(3) << i << "\"";
    paes_file << "\n\"123\";\"Matematicas\"";
    for(int i=0; i<100; i++) paes_file << ";\"A\"";
    paes_file << "\n";
    paes_file.close();
}

void test_full_integration() {
    create_test_files();

    auto estudiantes = leer_estudiantes("tests/integration/estudiantes_test.csv");
    auto correctas = leer_correctas("tests/integration/correctas_test.csv");

    std::ifstream paes_stream("tests/integration/paes_test.csv");
    std::string dummy;
    std::getline(paes_stream, dummy); // skip header

    std::vector<RespuestaEstudiante> respuestas;
    auto chunk = leer_chunk(paes_stream, 100);
    respuestas.insert(respuestas.end(), chunk.begin(), chunk.end());
    paes_stream.close();

    auto resultados = procesar(estudiantes, respuestas, correctas);

    assert(resultados.size() == 1);
    auto& r = resultados[0];
    std::cout << "buenas: " << r.buenas << ", malas: " << r.malas << ", omitidas: " << r.omitidas << ", puntaje: " << r.puntaje << std::endl;
    assert(r.buenas == 100);
    assert(r.malas == 0);
    assert(r.omitidas == 0);
    assert(r.puntaje == 1000.0); // 100 + (100/100)*900 = 1000

    std::cout << "test_full_integration aprobado" << std::endl;

    // Cleanup
    std::remove("tests/integration/estudiantes_test.csv");
    std::remove("tests/integration/correctas_test.csv");
    std::remove("tests/integration/paes_test.csv");
}

int main() {
    test_full_integration();
    std::cout << "Todas las pruebas de integración pasaron!" << std::endl;
    return 0;
}