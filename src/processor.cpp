#include "processor.hpp"
#include <map>
#include <algorithm>
#include <iostream>
#include <omp.h>

std::vector<Resultado> procesar(
    const std::vector<Estudiante>& estudiantes, 
    const std::vector<RespuestaEstudiante>& respuestas, 
    const std::vector<RespuestaCorrecta>& correctas
) {
    std::map<std::string, RespuestaCorrecta> correctas_map;
    for (const auto& c : correctas) {
        correctas_map[c.prueba] = c;
    }

    std::vector<Resultado> resultados;

    // Paralelizar sobre respuestas
    #pragma omp parallel
    {
        std::vector<Resultado> local_resultados;
        #pragma omp for
        for (size_t i = 0; i < respuestas.size(); ++i) {
            const auto& resp = respuestas[i];
            auto it = correctas_map.find(resp.prueba);
            if (it == correctas_map.end()) {
                std::cout << "No encontrada correcta para " << resp.prueba << std::endl;
                continue;
            }
            const auto& corr = it->second;

            int buenas = 0, omitidas = 0, malas = 0;
            for (size_t j = 0; j < 100; ++j) {
                if (resp.respuestas[j].empty()) {
                    omitidas++;
                } else if (resp.respuestas[j][0] == corr.respuestas[j]) {
                    buenas++;
                } else {
                    malas++;
                }
            }

            double penalizacion = malas * 0.25;
            double aciertos = std::max(0.0, static_cast<double>(buenas) - penalizacion);
            double puntaje = 100 + (aciertos / 100.0) * 900;

            // Encontrar estudiante
            auto est_it = std::find_if(
                estudiantes.begin(), 
                estudiantes.end(), 
                [&](const Estudiante& e)
                { return e.codigo == resp.estudiante; }
            );
            if (est_it == estudiantes.end()) {
                continue;
            }
            double pes = (est_it->promedio_notas / 7.0) * 1000;

            Resultado r;
            r.codigo_estudiante = resp.estudiante;
            r.pes = pes;
            r.prueba = resp.prueba;
            r.buenas = buenas;
            r.omitidas = omitidas;
            r.malas = malas;
            r.puntaje = puntaje;

            local_resultados.push_back(r);
        }

        #pragma omp critical
        resultados.insert(resultados.end(), local_resultados.begin(), local_resultados.end());
    }

    return resultados;
}
