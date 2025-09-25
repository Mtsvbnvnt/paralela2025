#include "csv_parser.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>

void test_split_csv() {
    // Test básico
    std::string line = "\"field1\";\"field2\";\"field3\"";
    auto fields = split_csv(line);
    assert(fields.size() == 3);
    assert(fields[0] == "field1");
    assert(fields[1] == "field2");
    assert(fields[2] == "field3");

    // Test con comillas internas (no debería haber, pero)
    line = "\"field\"\"with\"\"quotes\";\"normal\"";
    fields = split_csv(line);
    assert(fields.size() == 2);
    assert(fields[0] == "field\"\"with\"\"quotes");
    assert(fields[1] == "normal");

    // Test sin comillas
    line = "field1;field2;field3";
    fields = split_csv(line);
    assert(fields.size() == 3);
    assert(fields[0] == "field1");

    std::cout << "test_split_csv aprobado" << std::endl;
}

int main() {
    test_split_csv();
    std::cout << "Todas las pruebas unitarias para csv_parser pasaron!" << std::endl;
    return 0;
}