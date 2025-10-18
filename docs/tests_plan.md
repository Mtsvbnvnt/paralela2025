# Plan de Pruebas Detallado - Procesador de Exámenes PAES

## 1. Estrategia y Alcance de Pruebas
Este documento detalla el conjunto de pruebas implementadas para validar la correctitud, robustez y rendimiento del sistema. La estrategia cubre tres niveles: pruebas unitarias para el aislamiento de componentes, pruebas de integración para el flujo completo y pruebas de rendimiento para la validación de requerimientos no funcionales.

---
## 2. Pruebas Unitarias

> **Implementación**: `test_csv_parser.cpp`, `test_processor.cpp`
> **Framework**: Biblioteca estándar de C++ (`<cassert>`).
> **Ejecución**: Compilación y ejecución directa de los archivos de prueba.

### 2.1. Módulo: `csv_parser`

| ID de Prueba | Función Probada | Escenario de Prueba | Aserción de Verificación |
| :--- | :--- | :--- | :--- |
| **UT-CSV-01** | `split_csv` | Línea con campos encerrados en comillas dobles. | `assert(fields[0] == "field1")` confirma que las comillas son removidas. |
| **UT-CSV-02** | `split_csv` | Línea con campos sin comillas. | `assert(fields[0] == "field1")` confirma el correcto parsing. |
| **UT-CSV-03** | `leer_estudiantes` | (Mock) Lectura de estudiante con nota "6,5". | El `struct Estudiante` resultante debe tener `promedio_notas` cercano a `6.5`. |
| **UT-CSV-04** | `escribir_resultados`| (Mock) Escritura de un lote de resultados. | El archivo de salida debe contener una cabecera y el formato de línea debe coincidir exactamente con el especificado (comillas, separadores, precisión decimal). |

### 2.2. Módulo: `processor`

| ID de Prueba | Función Probada | Escenario de Prueba (Datos Mock) | Aserciones Clave de Verificación |
| :--- | :--- | :--- | :--- |
| **UT-PROC-01**| `procesar` | **Caso Perfecto**: 100 respuestas correctas. | `assert(r.buenas == 100)`, `assert(r.malas == 0)`, `assert(r.puntaje == 1000.0)`. |
| **UT-PROC-02**| `procesar` | **Caso Pésimo**: 100 respuestas incorrectas. | `assert(r.malas == 100)`, `assert(r.puntaje == 100.0)`. |
| **UT-PROC-03**| `procesar` | **Caso Nulo**: 100 respuestas omitidas. | `assert(r.omitidas == 100)`, `assert(r.puntaje == 100.0)`. |
| **UT-PROC-04**| `procesar` | **Caso Mixto**: 80 buenas, 16 malas, 4 omitidas. | `assert(r.buenas == 80)`, `assert(r.malas == 16)`, `assert(abs(r.puntaje - 784.0) < 0.001)`. |
| **UT-PROC-05**| `procesar` | **Cálculo de PES**: Estudiante con promedio 6.5. | `assert(abs(r.pes - 928.5714286) < 0.0001)`. |
| **UT-PROC-06**| `procesar` | **Datos Inconsistentes**: Código de estudiante o prueba sin correspondencia. | El registro de respuesta es ignorado silenciosamente, el tamaño del vector de resultados no aumenta para ese caso. |

---
## 3. Pruebas de Integración

> **Implementación**: `test_integration.cpp`
> **Metodología**: El script de prueba genera mediante programación un conjunto autocontenido y mínimo de archivos CSV de entrada (`estudiantes_test.csv`, `correctas_test.csv`, `paes_test.csv`). Luego invoca a las funciones de alto nivel (`leer_*`, `procesar`) y valida los resultados en memoria.

| ID de Prueba | Escenario de Prueba | Pasos de Ejecución | Aserción de Verificación |
| :--- | :--- | :--- | :--- |
| **IT-01** | Flujo End-to-End | 1. `create_test_files()` escribe archivos mock. <br> 2. Se leen los archivos y se cargan en los structs. <br> 3. Se llama a `procesar`. | `assert(resultados.size() == 1)`, `assert(r.puntaje == 1000.0)`. Confirma que todo el pipeline funciona para un caso simple y válido. |

---
## 4. Pruebas de Rendimiento

> **Implementación**: `performance_test.cpp`, `test_chunk_size.cpp`, `main.f90`
> **Metodología**: Scripts dedicados que ejecutan el procesador sobre el dataset completo (`data/cpyd`) para capturar y registrar métricas de tiempo y rendimiento en archivos de texto.

| ID de Prueba | Objetivo de la Prueba | Metodología | Criterio de Éxito / Resultado Esperado (basado en `performance_metrics_*.txt`) |
| :--- | :--- | :--- | :--- |
| **PT-01** | **Benchmark Optimizado** | Ejecutar `performance_test.cpp` con la implementación final. | Tasa de procesamiento: **~3109 res/s**. Tiempo total: **~365s**. |
| **PT-02** | **Benchmark Comparativo (Búsqueda Lineal)**| Ejecutar con una versión modificada del procesador que usa `std::vector::find`. | Tasa de procesamiento: **~132 res/s**. Tiempo total: **~8574s**. Se valida la ganancia de ~23x por usar `unordered_map`. |
| **PT-03** | **Benchmark Comparativo (Fortran)** | Ejecutar el programa `main.f90` compilado como implementación de referencia secuencial. | Tasa de procesamiento: **~555 res/s**. Tiempo total: **~3267s**. Se valida la ganancia de ~5.6x de la solución C++ paralela. |
| **PT-04** | **Análisis de `chunk_size`** | Ejecutar `test_chunk_size.cpp`, que itera sobre varios tamaños de lote. | Se genera un CSV con los resultados, que al ser graficado (`ProcessingRate_ChunkSize_Plot.png`), muestra que `10000` es un punto cercano al óptimo antes de que la curva de rendimiento se aplane. |

---
## 5. Implementación de Referencia en Fortran

> **Ubicación**: `tests/fortran_test/main.f90`  
> **Propósito**: Validación cruzada y benchmark de referencia secuencial  
> **Compilación**: `gfortran -o main main.f90`

### 5.1. Objetivo y Justificación

La implementación en Fortran sirve como **implementación de referencia secuencial** para validar la correctitud de los algoritmos y proporcionar una línea base de rendimiento. Esta implementación replica exactamente la misma lógica de cálculo que la versión C++, pero de manera secuencial y sin optimizaciones avanzadas.

### 5.2. Características de la Implementación Fortran

| Aspecto | Implementación | Justificación |
| :--- | :--- | :--- |
| **Algoritmo** | Búsqueda lineal para estudiantes y respuestas correctas | Implementación más directa y simple para validación |
| **Paralelismo** | Ninguno (completamente secuencial) | Línea base para medir ganancia del paralelismo |
| **Estructuras de datos** | Arrays estáticos con tipos derivados Fortran | Simplicidad y compatibilidad con Fortran 90+ |
| **Procesamiento** | Chunking idéntico (10,000 registros) | Consistencia en metodología de pruebas |
| **Precisión** | Aritmética de punto flotante simple | Suficiente para validación de correctitud |

### 5.3. Validación Cruzada

La implementación Fortran permite verificar que:

1. **Algoritmos de cálculo son correctos**: Mismos resultados numéricos para los mismos datos de entrada
2. **Manejo de archivos CSV es consistente**: Parsing y escritura producen formatos compatibles
3. **Lógica de negocio es equivalente**: Cálculo de PES, puntajes y penalizaciones son idénticos

### 5.4. Métricas de Rendimiento Fortran

Basado en `performance_metrics_fortran.txt`:

| Métrica | Valor | Observaciones |
| :--- | :--- | :--- |
| **Tiempo total** | 3267.39 segundos | ~54.5 minutos para dataset completo |
| **Tasa de procesamiento** | 554.97 respuestas/segundo | Línea base secuencial |
| **Respuestas procesadas** | 1,764,582 | Dataset completo PAES |
| **Estudiantes únicos** | 294,097 | Validación de integridad de datos |
| **Chunks procesados** | 177 | Consistencia con implementación C++ |

### 5.5. Comparativa de Rendimiento

| Implementación | Tasa (res/s) | Tiempo Total | Speedup vs Fortran |
| :--- | :--- | :--- | :--- |
| **Fortran Secuencial** | 555 | 3267s | 1.0x (baseline) |
| **C++ Búsqueda Lineal** | 132 | 8575s | 0.24x |
| **C++ Optimizado (8 hilos)** | 3109 | 365s | **5.6x** |

### 5.6. Casos de Uso de la Implementación Fortran

1. **Desarrollo**: Validación rápida de algoritmos durante implementación
2. **Testing**: Verificación cruzada de resultados en casos edge
3. **Benchmarking**: Línea base para medir mejoras de optimización
4. **Documentación**: Referencia académica de implementación "naive" pero correcta
5. **Portabilidad**: Alternativa en sistemas donde C++/OpenMP no está disponible

### 5.7. Limitaciones Conocidas

- **Sin paralelismo**: No aprovecha múltiples cores disponibles
- **Búsqueda O(n)**: Sin optimizaciones de estructuras de datos
- **Memoria**: Carga chunks completos en memoria (similar a C++)
- **Precision**: Usa aritmética simple precision (suficiente para el caso de uso)