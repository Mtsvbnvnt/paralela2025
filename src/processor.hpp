#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "model.hpp"
#include <vector>

std::vector<Resultado> procesar(const std::vector<Estudiante>& estudiantes, const std::vector<RespuestaEstudiante>& respuestas, const std::vector<RespuestaCorrecta>& correctas);

#endif // PROCESSOR_HPP
