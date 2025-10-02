#include "csv_parser.hpp"
#include "processor.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cstdlib>
#include <thread>

// Performance test using original data in data/cpyd
void test_performance() {
    std::string estudiantes_file = "data/cpyd/estudiantes.csv";
    std::string paes_file = "data/cpyd/paes.csv";
    std::string correctas_file = "data/cpyd/correctas.csv";
    std::string resultados_file = "performance_resultados.csv"; // Temp output file

    // Count total responses (lines in paes.csv minus header)
    size_t total_responses = 0;
    {
        std::ifstream count_stream(paes_file);
        std::string line;
        std::getline(count_stream, line); // skip header
        while (std::getline(count_stream, line)) {
            total_responses++;
        }
        count_stream.close();
    }

    // System info
    unsigned int num_cores = std::thread::hardware_concurrency();

    auto overall_start = std::chrono::high_resolution_clock::now();

    // Time for loading estudiantes
    auto load_est_start = std::chrono::high_resolution_clock::now();
    auto estudiantes = leer_estudiantes(estudiantes_file);
    auto load_est_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> load_est_time = load_est_end - load_est_start;

    // Time for loading correctas
    auto load_corr_start = std::chrono::high_resolution_clock::now();
    auto correctas = leer_correctas(correctas_file);
    auto load_corr_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> load_corr_time = load_corr_end - load_corr_start;

    size_t total_resultados = 0;
    size_t processed_responses = 0;
    size_t num_chunks = 0;
    std::vector<double> chunk_times;

    try {
        std::ifstream paes_stream(paes_file);
        if (!paes_stream.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo: " + paes_file);
        }
        std::string dummy_header;
        std::getline(paes_stream, dummy_header);

        bool first_chunk = true;
        size_t chunk_size = 10000;
        while (true) {
            auto chunk_start = std::chrono::high_resolution_clock::now();
            auto respuestas_chunk = leer_chunk(paes_stream, chunk_size);
            if (respuestas_chunk.empty()) break;
            auto resultados = procesar(estudiantes, respuestas_chunk, correctas);
            escribir_resultados(resultados, resultados_file, first_chunk);
            auto chunk_end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> chunk_time = chunk_end - chunk_start;
            chunk_times.push_back(chunk_time.count());
            first_chunk = false;
            total_resultados += resultados.size();
            processed_responses += respuestas_chunk.size();
            num_chunks++;
            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = now - overall_start;
            double percentage = static_cast<double>(processed_responses) / total_responses * 100.0;
            std::cout << "\rProcessed " << processed_responses << " / " << total_responses << " responses (" << std::fixed << std::setprecision(2) << percentage << "%) - Elapsed time: " << elapsed.count() << "s" << std::flush;
        }
        paes_stream.close();
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error in performance test: " << e.what() << std::endl;
        return;
    }

    auto overall_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> overall_duration = overall_end - overall_start;

    // Calculate averages
    double avg_chunk_time = 0.0;
    if (!chunk_times.empty()) {
        for (auto t : chunk_times) avg_chunk_time += t;
        avg_chunk_time /= chunk_times.size();
    }

    // Write metrics to file
    std::ofstream metrics_file("performance_metrics.txt");
    metrics_file << "Performance Metrics (using data/cpyd):\n";
    metrics_file << "System Info:\n";
    metrics_file << "  Number of CPU cores: " << num_cores << "\n";
    metrics_file << "Data Info:\n";
    metrics_file << "  Total responses: " << total_responses << "\n";
    metrics_file << "  Number of students: " << estudiantes.size() << "\n";
    metrics_file << "  Number of chunks processed: " << num_chunks << "\n";
    metrics_file << "Timing (seconds):\n";
    metrics_file << "  Time to load estudiantes: " << load_est_time.count() << "\n";
    metrics_file << "  Time to load correctas: " << load_corr_time.count() << "\n";
    metrics_file << "  Total processing time (chunks): " << overall_duration.count() - load_est_time.count() - load_corr_time.count() << "\n";
    metrics_file << "  Overall time: " << overall_duration.count() << "\n";
    metrics_file << "  Average time per chunk: " << avg_chunk_time << "\n";
    metrics_file << "Results:\n";
    metrics_file << "  Total results processed: " << total_resultados << "\n";
    metrics_file << "  Processing rate (results/second): " << total_resultados / overall_duration.count() << "\n";
    metrics_file.close();

    // Cleanup temp output
    std::remove(resultados_file.c_str());

    std::cout << "Performance test completed, metrics written to performance_metrics.txt" << std::endl;
}

int main() {
    test_performance();
    return 0;
}