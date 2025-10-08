#include "csv_parser.hpp"
#include "processor.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <cstdlib> // for std::remove if needed

// Test to justify chunk size using a fragment of data (first 100k responses)
void test_chunk_sizes() {
    std::string estudiantes_file = "data/cpyd/estudiantes.csv";
    std::string paes_file = "data/cpyd/paes.csv";
    std::string correctas_file = "data/cpyd/correctas.csv";
    std::string temp_paes_file = "temp_subset_paes.csv"; // Temp file for subset
    std::string resultados_file = "chunk_test_resultados.csv"; // Temp output file

    // Load estudiantes and correctas once
    auto estudiantes = leer_estudiantes(estudiantes_file);
    auto correctas = leer_correctas(correctas_file);

    // Create a subset of paes.csv: first 100000 responses
    size_t subset_size = 100000;
    {
        std::ifstream paes_stream(paes_file);
        std::ofstream temp_stream(temp_paes_file);
        std::string line;
        std::getline(paes_stream, line); // header
        temp_stream << line << "\n";
        size_t count = 0;
        while (count < subset_size && std::getline(paes_stream, line)) {
            temp_stream << line << "\n";
            count++;
        }
        paes_stream.close();
        temp_stream.close();
    }

    // Chunk sizes to test
    std::vector<size_t> chunk_sizes = {5000, 10000, 20000, 50000};

    // Open CSV for metrics
    // Columns explanation:
    // ChunkSize: The size of each chunk (number of responses processed per chunk)
    // TotalTime: Total time in seconds to process all data with this chunk size
    // NumChunks: Number of chunks processed
    // AvgChunkTime: Average time in seconds per chunk
    // TotalResults: Total number of results processed (should be the same for all chunk sizes)
    // ProcessingRate: Number of results processed per second (TotalResults / TotalTime)
    // TimePerData: Average time in seconds to process one data point (TotalTime / TotalResults)
    std::ofstream csv_file("chunk_size_metrics.csv");
    csv_file << "ChunkSize,TotalTime,NumChunks,AvgChunkTime,TotalResults,ProcessingRate,TimePerData\n";

    for (size_t chunk_size : chunk_sizes) {
        std::cout << "Testing chunk size: " << chunk_size << std::endl;

        auto test_start = std::chrono::high_resolution_clock::now();

        size_t total_resultados = 0;
        size_t processed_responses = 0;
        size_t num_chunks = 0;
        std::vector<double> chunk_times;

        std::ifstream paes_stream(temp_paes_file);
        std::string dummy_header;
        std::getline(paes_stream, dummy_header);

        bool first_chunk = true;
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
            std::cout << "  Processed " << processed_responses << " / " << subset_size << " responses" << std::endl;
        }
        paes_stream.close();

        auto test_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> total_time = test_end - test_start;

        double avg_chunk_time = 0.0;
        if (!chunk_times.empty()) {
            for (auto t : chunk_times) avg_chunk_time += t;
            avg_chunk_time /= chunk_times.size();
        }

        double processing_rate = total_resultados / total_time.count();
        double time_per_data = total_time.count() / total_resultados;

        // Write to CSV
        csv_file << chunk_size << "," << total_time.count() << "," << num_chunks << "," << avg_chunk_time << "," << total_resultados << "," << processing_rate << "," << time_per_data << "\n";

        // Cleanup temp results for this chunk size
        std::remove(resultados_file.c_str());
    }

    csv_file.close();

    // Cleanup temp subset
    std::remove(temp_paes_file.c_str());

    std::cout << "Chunk size test completed, metrics written to chunk_size_metrics.csv" << std::endl;
}

int main() {
    test_chunk_sizes();
    return 0;
}