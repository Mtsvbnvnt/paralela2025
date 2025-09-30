# Proyecto Paralela - Procesamiento de Exámenes PAES

## Descripción

Este proyecto implementa un programa en C++ para la asignatura de "Computación Paralela y Distribuida". El programa procesa datos de estudiantes y sus respuestas a los exámenes PAES (Prueba de Acceso a la Educación Superior) de un país ficticio. Utiliza OpenMP para paralelizar los cálculos de puntajes, optimizando el rendimiento en sistemas multi-núcleo.

### Contexto del Problema

El programa calcula:

- **PES (Puntaje de Educación Secundaria)**: `(promedio_notas / 7.0) * 1000`
- **Puntaje de Pruebas**: `100 + (aciertos / 100) * 900`, donde `aciertos = max(0, buenas - penalización)`, y `penalización = malas * 0.25`

Las pruebas incluyen: Matemáticas, Lenguaje y Literatura, Ciencias Sociales, Biología, Física y Química, cada una con 100 preguntas.

## Características

- **Paralelismo con OpenMP**: Procesamiento paralelo de respuestas para mejorar el rendimiento.
- **Manejo de Archivos CSV**: Lectura y escritura de archivos CSV con separador `;` y comillas dobles.
- **Chunking de Datos**: Procesamiento en lotes para manejar grandes volúmenes de datos sin problemas de memoria.
- **Validación de Entrada**: Verificación de formatos y existencia de archivos.
- **Salida Estructurada**: Resultados en CSV con precisión decimal específica.

## Requisitos del Sistema

- **Compilador**: C++17 compatible con OpenMP (ej. g++ 8+ o MSVC con OpenMP).
- **CMake**: Versión 3.10 o superior.
- **Sistema Operativo**: Windows, Linux o macOS.
- **Memoria**: Recomendado al menos 4GB RAM para datasets grandes.

## Instalación

1. Clona el repositorio:

   ```bash
   git clone <url-del-repositorio>
   cd proyecto_paralela
   ```

2. Crea el directorio de build:

   ```bash
   mkdir build
   cd build
   ```

3. Configura con CMake:

   ```bash
   cmake ..
   ```

4. Compila el proyecto:

   ```bash
   make  # En Linux/macOS
   # o en Windows con Ninja: ninja
   ```

   El ejecutable `main.exe` (o `main` en Unix) se generará en el directorio `build`.

## Uso

### Sintaxis de Comando

```bash
./main -e <archivo_estudiantes> -p <archivo_paes> -c <archivo_correctas> -r <archivo_resultados>
```

### Parámetros

- `-e`: Archivo CSV de estudiantes (requerido).
- `-p`: Archivo CSV de respuestas PAES (requerido).
- `-c`: Archivo CSV de respuestas correctas (requerido).
- `-r`: Archivo de salida para resultados (requerido).

### Ejemplo de Uso

```bash
./main -e data/cpyd/estudiantes.csv -p data/cpyd/paes.csv -c data/cpyd/correctas.csv -r data/cpyd/resultados.csv
```

### Archivos de Ejemplo

- `data/cpyd/`: Archivos reales con datos completos.

### Formatos de Archivos

#### Archivo de Estudiantes (`estudiantes.csv`)

- Separador: `;`
- Campos: CÓDIGO, GÉNERO, FECHA DE NACIMIENTO, NOMBRES, APELLIDOS, REGIÓN GEOGRÁFICA, PROMEDIO DE NOTAS
- Decimales: Usan `,` (ej. `6,5`)

#### Archivo de Respuestas Correctas (`correctas.csv`)

- Separador: `;`
- Campos: PRUEBA, RESPUESTA 001, ..., RESPUESTA 100
- Pruebas: LENGUAJE Y LITERATURA, MATEMÁTICAS, etc.

#### Archivo de Respuestas PAES (`paes.csv`)

- Separador: `;`
- Campos: ESTUDIANTE, PRUEBA, PREGUNTA 001, ..., PREGUNTA 100
- Respuestas: A, B, C, D, E o vacío para omitidas.

#### Archivo de Resultados (`resultados.csv`)

- **Separador: `;`**
- Campos: CÓDIGO ESTUDIANTE, PES, PRUEBA, BUENAS, OMITIDAS, MALAS, PUNTAJE
- Todos los campos entre comillas dobles.
- PES: 7 decimales, PUNTAJE: 2 decimales.

## Estructura del Proyecto

```text
proyecto_paralela/
├── CMakeLists.txt          # Configuración de CMake
├── README.md               # Este archivo
├── src/                    # Código fuente
│   ├── main.cpp            # Punto de entrada
│   ├── csv_parser.hpp      # Declaraciones del parser CSV
│   ├── csv_parser.cpp      # Implementación del parser CSV
│   ├── model.hpp           # Estructuras de datos
│   ├── processor.hpp       # Declaraciones del procesador
│   └── processor.cpp       # Implementación del procesador
├── data/                   # Datos de entrada y salida
│   └── cpyd/               # Archivos reales
│       ├── estudiantes.csv
│       ├── paes.csv
│       ├── correctas.csv
│       └── resultados.csv
├── tests/                  # Pruebas
│   ├── integration/
│   └── unit/
├── docs/                   # Documentación
│   ├── spec.md
│   ├── tests_plan.md
│   └── diagrams/
└── documentos/             # Documentos del proyecto
    ├── contexto.txt
    └── Trabajo Paralelo.pdf
```

## Arquitectura del Código

### Módulos Principales

- **model.hpp**: Define structs para `Estudiante`, `RespuestaCorrecta`, `RespuestaEstudiante` y `Resultado`.
- **csv_parser.cpp**: Maneja lectura/escritura de CSV con soporte para comillas y separadores personalizados.
- **processor.cpp**: Contiene la lógica de cálculo de puntajes con paralelismo OpenMP.
- **main.cpp**: Orquesta la ejecución, maneja argumentos y coordina el chunking.

### Optimizaciones

- **Chunking**: Procesa respuestas en lotes de 10,000 para controlar uso de memoria.
- **Paralelismo**: Usa `#pragma omp parallel for` para distribuir cálculos en múltiples hilos.
- **Búsqueda Eficiente**: Se utiliza un `std::map` para una búsqueda rápida de las pautas de respuestas correctas. La búsqueda de estudiantes se realiza mediante una búsqueda lineal.

## Contribución

1. Fork el proyecto.
2. Crea una rama para tu feature: `git checkout -b feature/nueva-funcionalidad`.
3. Commit tus cambios: `git commit -am 'Agrega nueva funcionalidad'`.
4. Push a la rama: `git push origin feature/nueva-funcionalidad`.
5. Abre un Pull Request.

### Guías de Desarrollo

- Sigue el estilo de código existente.
- Agrega comentarios en funciones complejas.
- Prueba cambios con los archivos de muestra antes de archivos reales.
- Actualiza este README si agregas nuevas funcionalidades.

## Pruebas

El proyecto incluye una suite completa de pruebas unitarias e integración para asegurar la calidad del código.

### Compilación de Tests

```bash
cd build
cmake --build . --target test_csv_parser
cmake --build . --target test_processor
cmake --build . --target test_integration
```

### Ejecución de Tests

```bash
# Test unitario del parser CSV
./test_csv_parser.exe

# Test unitario del procesador
./test_processor.exe

# Test de integración completa
./test_integration.exe
```

### Descripción de Tests

#### Tests Unitarios

**test_csv_parser**
- Prueba la función `split_csv` con diferentes casos:
  - Líneas simples sin comillas
  - Campos con comillas dobles
  - Campos vacíos
  - Separadores dentro de comillas
- Verifica el parsing correcto de CSV con separador `;`

**test_processor**
- Prueba la lógica de cálculo de puntajes:
  - Cálculo de aciertos, errores y omitidas
  - Penalización por respuestas malas (0.25 por error)
  - Cálculo del puntaje final: `100 + (aciertos / 100) * 900`
  - Cálculo del PES: `(promedio_notas / 7.0) * 1000`
- Usa datos mock para validar la lógica de procesamiento paralelo

#### Tests de Integración

**test_integration**
- Prueba el flujo completo del programa:
  - Creación de archivos CSV temporales (estudiantes, correctas, respuestas PAES)
  - Lectura y procesamiento de datos
  - Escritura de resultados
  - Verificación de valores calculados correctamente
- Simula 100 preguntas con respuestas perfectas para validar cálculos
- Limpieza automática de archivos temporales

### Resultados Esperados

Todos los tests deben pasar exitosamente mostrando mensajes como:

```text
test_split_csv aprobado
Todas las pruebas unitarias para csv_parser pasaron!

test_procesar aprobado
Todas las pruebas unitarias para processor pasaron!

buenas: 100, malas: 0, omitidas: 0, puntaje: 1000
test_full_integration aprobado
Todas las pruebas de integración pasaron!
```

### Cobertura de Tests

- **Parsing CSV**: Manejo de comillas, separadores, campos vacíos
- **Lógica de Negocio**: Cálculos de puntajes, penalizaciones, PES
- **Flujo Completo**: Lectura → Procesamiento → Escritura
- **Manejo de Errores**: Archivos inexistentes, formatos inválidos
- **Paralelismo**: Validación de resultados consistentes con OpenMP

## Licencia

Este proyecto es para fines educativos. Consulta con el docente para distribución.

## Autores

- Patricio Abarca
- Rodrigo Tapia
- Matias Villarroel

## Notas Adicionales

- El programa muestra errores en stderr y nombres de autores en stdout.
- Para datasets muy grandes, ajusta `chunk_size` en `main.cpp`.
- Compatible con UTF-8, pero evita caracteres especiales en paths.

Para más detalles, consulta `docs/spec.md` y `docs/tests_plan.md`.