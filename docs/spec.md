# Especificaciones Técnicas Detalladas - Procesador de Exámenes PAES
**Versión 1.0**

## 1. Introducción y Alcance
Este documento proporciona una especificación técnica completa del sistema de procesamiento de exámenes PAES. Su propósito es servir como referencia para desarrolladores, testers y arquitectos, detallando la arquitectura del software, los formatos de datos, los algoritmos de procesamiento y los requerimientos de rendimiento.

El sistema está diseñado para procesar archivos CSV de gran tamaño que contienen respuestas de exámenes, calcular puntajes y generar un reporte consolidado de manera eficiente.

---
## 2. Arquitectura del Sistema
El software sigue un diseño modular para separar responsabilidades.

| Módulo | Archivos | Responsabilidad Principal |
| :--- | :--- | :--- |
| **Punto de Entrada** | `main.cpp` | Orquestación del flujo principal, manejo de argumentos de línea de comandos (CLI), gestión del procesamiento por lotes (chunks) y manejo de excepciones a alto nivel. |
| **Lógica de Negocio** | `processor.cpp`, `processor.hpp` | Implementación del algoritmo de cálculo de puntajes, paralelización de cómputo con OpenMP y uso de estructuras de datos optimizadas para búsquedas rápidas. |
| **Parser CSV** | `csv_parser.cpp`, `csv_parser.hpp` | Abstracción de todas las operaciones de lectura y escritura de archivos CSV, incluyendo el manejo de formatos, delimitadores y conversión de datos. |
| **Modelo de Datos** | `model.hpp` | Definición de todas las estructuras de datos (structs) que representan las entidades del dominio (Estudiante, Resultado, etc.). |

---
## 3. Modelo y Formatos de Datos

### 3.1. Estructuras de Datos Internas (`model.hpp`)
* `struct Estudiante`: Almacena los datos de un estudiante.
    * `promedio_notas` (double): Se almacena como número de punto flotante para cálculos.
* `struct RespuestaCorrecta`: Almacena la pauta de una prueba.
    * `respuestas` (vector<char>): Vector de caracteres para un acceso eficiente por índice.
* `struct RespuestaEstudiante`: Representa las respuestas de un estudiante a una prueba.
    * `respuestas` (vector<string>): Vector de strings para manejar respuestas vacías (omitidas).
* `struct Resultado`: Representa una línea del archivo de salida.

### 3.2. Formato de Archivos de Entrada
El sistema procesa tres archivos CSV con codificación UTF-8 y separador punto y coma (`;`).

#### 3.2.1. `estudiantes.csv`
| Campo | Tipo de Dato | Ejemplo | Notas |
| :--- | :--- | :--- | :--- |
| CÓDIGO | `string` | `"A00000001"` | Identificador único del estudiante. |
| GÉNERO | `string` | `"M"` | |
| FECHA DE NACIMIENTO | `string` | `"1998-05-12"` | Formato AAAA-MM-DD. |
| NOMBRES | `string` | `"JUAN PABLO"` | |
| APELLIDOS | `string` | `"PÉREZ GONZÁLEZ"` | |
| REGIÓN GEOGRÁFICA | `string` | `"REGIÓN METROPOLITANA"` | |
| PROMEDIO DE NOTAS | `string` | `"6,5"` | **Crítico**: Usa coma (`,`) como separador decimal. El parser debe manejar esta conversión a `double`. |

#### 3.2.2. `correctas.csv`
| Campo | Tipo de Dato | Ejemplo | Notas |
| :--- | :--- | :--- | :--- |
| PRUEBA | `string` | `"Matemáticas"` | Nombre de la prueba (clave). |
| RESPUESTA 001..100 | `string` | `"A"` | Letra de la respuesta correcta. |

#### 3.2.3. `paes.csv`
| Campo | Tipo de Dato | Ejemplo | Notas |
| :--- | :--- | :--- | :--- |
| ESTUDIANTE | `string` | `"A00000001"` | Clave foránea a `estudiantes.csv`. |
| PRUEBA | `string` | `"Matemáticas"` | Clave foránea a `correctas.csv`. |
| PREGUNTA 001..100 | `string` | `"A"`, `""` | `""` (string vacío) representa una respuesta omitida. |

### 3.3. Formato de Archivo de Salida (`resultados.csv`)
| Campo | Tipo de Dato | Ejemplo | Formato/Precisión |
| :--- | :--- | :--- | :--- |
| CÓDIGO ESTUDIANTE| `string` | `"A00000001"` | Encerrado en comillas dobles. |
| PES | `double` | `"928.5714286"` | Precisión de **7 decimales**. |
| PRUEBA | `string` | `"Matemáticas"` | Encerrado en comillas dobles. |
| BUENAS | `int` | `"80"` | Encerrado en comillas dobles. |
| OMITIDAS | `int` | `"4"` | Encerrado en comillas dobles. |
| MALAS | `int` | `"16"` | Encerrado en comillas dobles. |
| PUNTAJE | `double` | `"784.00"` | Precisión de **2 decimales**. |

---
## 4. Lógica de Procesamiento y Algoritmos

### 4.1. Orquestación (`main.cpp`)
1.  **Parsing de CLI**: Se leen los argumentos `-e`, `-p`, `-c`, `-r`. Si falta alguno, el programa termina con un mensaje de error en `stderr` y código de salida 1.
2.  **Carga Inicial**: Se cargan en memoria la totalidad de los archivos `estudiantes.csv` y `correctas.csv`.
3.  **Procesamiento por Lotes (Chunking)**:
    * Se abre un `ifstream` para `paes.csv`.
    * Se entra en un bucle `while(true)` que lee el archivo en lotes de `chunk_size` líneas (valor fijo: 10,000).
    * La función `leer_chunk` se encarga de esta lectura parcial. Si `leer_chunk` devuelve un vector vacío, el bucle termina.
    * Cada lote (`vector<RespuestaEstudiante>`) se envía a la función `procesar`.
    * El resultado de `procesar` se escribe en el archivo de salida usando `escribir_resultados`. Un flag `first_chunk` asegura que la cabecera del CSV se escriba solo una vez.
4.  **Manejo de Excepciones**: Todo el proceso está envuelto en un bloque `try-catch` para capturar `std::runtime_error` (ej. archivos no encontrados) y reportarlos a `stderr`.

### 4.2. Algoritmo de Cálculo (`processor.cpp`)
La función `procesar` recibe los datos de estudiantes, las pautas y un lote de respuestas.
1.  **Optimización de Búsqueda**:
    * Los datos de estudiantes y las pautas correctas se transforman en `std::unordered_map` para un acceso promedio en tiempo O(1).
        * `std::unordered_map<std::string, Estudiante> estudiantes_map;` (Clave: código de estudiante)
        * `std::unordered_map<std::string, RespuestaCorrecta> correctas_map;` (Clave: nombre de la prueba)
2.  **Paralelización**:
    * El bucle principal que itera sobre el lote de respuestas de estudiantes se paraleliza con una directiva OpenMP: `#pragma omp parallel for`.
    * **Reducción**: Se utiliza una estrategia de reducción local. Cada hilo acumula sus propios resultados en un `std::vector<Resultado> local_resultados`. Al final de la región paralela, estos vectores locales se fusionan en el vector principal `resultados` bajo una sección `#pragma omp critical` para evitar condiciones de carrera.
3.  **Lógica de Puntuación (por cada respuesta)**:
    * Se busca la pauta de la prueba en `correctas_map`. Si no se encuentra, la respuesta es ignorada.
    * Se itera de la pregunta 1 a la 100:
        * Si la respuesta es `""`, se incrementa `omitidas`.
        * Si la respuesta coincide con la pauta, se incrementa `buenas`.
        * En caso contrario, se incrementa `malas`.
    * Se aplican las fórmulas de puntuación:
        * `penalización = malas * 0.25`
        * `aciertos = std::max(0.0, (double)buenas - penalización)`
        * `puntaje = 100 + (aciertos / 100.0) * 900`
    * Se busca al estudiante en `estudiantes_map`. Si no se encuentra, el resultado es ignorado.
    * Se calcula el PES: `pes = (promedio_notas / 7.0) * 1000`
    * Se ensambla y guarda el `struct Resultado`.

---
## 5. Requerimientos No Funcionales

* **Rendimiento**: El sistema debe procesar el dataset de `1,764,582` respuestas en aproximadamente **365 segundos** en una máquina de 8 núcleos, logrando una tasa de procesamiento superior a **3,100 resultados/segundo**.
* **Eficiencia de Memoria**: El uso de un `chunk_size` de `10,000` está justificado por pruebas empíricas (`test_chunk_size.cpp`) que demuestran ser un punto de equilibrio óptimo entre el overhead de I/O y el consumo de RAM.
* **Escalabilidad**: El rendimiento debe escalar con el número de núcleos disponibles, gracias al uso de OpenMP.
* **Benchmarking**: El rendimiento de la solución debe ser comparado y superar a las implementaciones de referencia (Fortran secuencial y C++ con búsqueda lineal), como se detalla en los archivos `performance_metrics_*.txt`.
* **Compatibilidad**: Requiere un compilador C++17 (ej. g++ 8+) con soporte OpenMP y CMake (v3.10+).