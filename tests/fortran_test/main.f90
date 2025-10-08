program main
    implicit none
    character(len=256) :: estudiantes_file, paes_file, correctas_file, resultados_file
    integer :: i, num_args, ios
    real :: overall_start, load_est_start, load_est_end, load_corr_start, load_corr_end, proc_start, proc_end, overall_end
    real :: load_est_time, load_corr_time, proc_time, overall_time
    integer :: total_responses, num_students, num_chunks, total_results, chunk_size, total_lines, total_chunks
    real :: avg_chunk_time, proc_rate
    character(len=10) :: flag
    character(len=1000) :: dummy
    character(len=50) :: progress_str

    ! Derived types
    type :: Estudiante
        character(len=50) :: codigo
        character(len=10) :: genero
        character(len=20) :: fecha_nac
        character(len=100) :: nombres
        character(len=100) :: apellidos
        character(len=50) :: region
        real :: promedio_notas
    end type Estudiante

    type :: RespuestaCorrecta
        character(len=50) :: prueba
        character(len=1) :: respuestas(100)
    end type RespuestaCorrecta

    type :: RespuestaEstudiante
        character(len=50) :: estudiante
        character(len=50) :: prueba
        character(len=1) :: respuestas(100)
    end type RespuestaEstudiante

    type :: Resultado
        character(len=50) :: codigo_estudiante
        real :: pes
        character(len=50) :: prueba
        integer :: buenas, omitidas, malas
        real :: puntaje
    end type Resultado

    ! Arrays (dynamic sizing)
    type(Estudiante), allocatable :: estudiantes(:)
    type(RespuestaCorrecta), allocatable :: correctas(:)
    type(RespuestaEstudiante), allocatable :: respuestas_chunk(:)
    type(Resultado), allocatable :: resultados(:)

    num_args = command_argument_count()
    if (num_args < 8) then
        write(*,*) "Usage: ./main -e <estudiantes> -p <paes> -c <correctas> -r <resultados>"
        stop
    end if
    do i = 1, num_args, 2
        call get_command_argument(i, flag)
        if (trim(flag) == '-e') then
            call get_command_argument(i+1, estudiantes_file)
        else if (trim(flag) == '-p') then
            call get_command_argument(i+1, paes_file)
        else if (trim(flag) == '-c') then
            call get_command_argument(i+1, correctas_file)
        else if (trim(flag) == '-r') then
            call get_command_argument(i+1, resultados_file)
        else
            write(*,*) "Invalid argument: ", trim(flag)
            stop
        end if
    end do

    call cpu_time(overall_start)

    ! Load estudiantes
    call cpu_time(load_est_start)
    call load_estudiantes(estudiantes_file, estudiantes, num_students)
    call cpu_time(load_est_end)
    load_est_time = load_est_end - load_est_start

    ! Load correctas
    call cpu_time(load_corr_start)
    call load_correctas(correctas_file, correctas)
    call cpu_time(load_corr_end)
    load_corr_time = load_corr_end - load_corr_start

    ! Process in chunks
    open(unit=10, file=paes_file, status='old')
    read(10, *)  ! Skip header
    ! Count total lines
    total_lines = 0
    do
        read(10, '(A)', iostat=ios) dummy
        if (ios /= 0) exit
        total_lines = total_lines + 1
    end do
    rewind(10)
    read(10, *)  ! Skip header again
    chunk_size = 10000
    total_chunks = (total_lines + chunk_size - 1) / chunk_size
    num_chunks = 0
    proc_time = 0.0
    total_results = 0
    total_responses = 0
    do
        call load_chunk(10, chunk_size, respuestas_chunk)
        if (size(respuestas_chunk) == 0) exit
        num_chunks = num_chunks + 1
        total_responses = total_responses + size(respuestas_chunk)
        call cpu_time(proc_start)
        call procesar(estudiantes, respuestas_chunk, correctas, resultados)
        call cpu_time(proc_end)
        proc_time = proc_time + (proc_end - proc_start)
        call write_resultados(resultados, resultados_file, num_chunks == 1)
        total_results = total_results + size(resultados)
        ! Show progress
        write(progress_str, '(A,I0,A,I0,A,I0,A)') 'Processing chunk ', num_chunks, ' of ', total_chunks, ' - ', int(100.0 * num_chunks / total_chunks), '%'
        progress_str = trim(progress_str) // repeat(' ', 50 - len_trim(progress_str))
        write(*, '(A1,A)', advance='no') char(13), progress_str
    end do
    close(10)
    write(*,*)  ! New line after progress

    call cpu_time(overall_end)
    overall_time = overall_end - overall_start
    avg_chunk_time = proc_time / num_chunks
    proc_rate = total_results / proc_time

    ! Write metrics
    open(unit=20, file='performance_metrics_fortran.txt', status='replace')
    write(20, *) "Performance Metrics (using data/cpyd):"
    write(20, *) "System Info:"
    write(20, *) "  Number of CPU cores: 1 (sequential)"
    write(20, *) "Data Info:"
    write(20, '(A, I0)') "  Total responses: ", total_responses
    write(20, '(A, I0)') "  Number of students: ", num_students
    write(20, '(A, I0)') "  Number of chunks processed: ", num_chunks
    write(20, *) "Timing (seconds):"
    write(20, '(A, F0.5)') "  Time to load estudiantes: ", load_est_time
    write(20, '(A, F0.5)') "  Time to load correctas: ", load_corr_time
    write(20, '(A, F0.5)') "  Total processing time (chunks): ", proc_time
    write(20, '(A, F0.5)') "  Overall time: ", overall_time
    write(20, '(A, F0.5)') "  Average time per chunk: ", avg_chunk_time
    write(20, *) "Results:"
    write(20, '(A, I0)') "  Total results processed: ", total_results
    write(20, '(A, F0.5)') "  Processing rate (results/second): ", proc_rate
    close(20)

    write(*,*) "Patricio Abarca"
    write(*,*) "Rodrigo Tapia"
    write(*,*) "Matias Villarroel"

contains

    subroutine load_estudiantes(file, est, n)
        character(len=*), intent(in) :: file
        type(Estudiante), allocatable, intent(out) :: est(:)
        integer, intent(out) :: n
        character(len=1000) :: line
        integer :: unit, ios, count
        unit = 11
        open(unit=unit, file=file, status='old')
        read(unit, *)  ! Skip header
        count = 0
        do
            read(unit, '(A)', iostat=ios) line
            if (ios /= 0) exit
            count = count + 1
        end do
        rewind(unit)
        read(unit, *)  ! Skip header
        allocate(est(count))
        n = count
        do i = 1, count
            read(unit, '(A)') line
            call parse_estudiante(line, est(i))
        end do
        close(unit)
    end subroutine

    subroutine parse_estudiante(line, e)
        character(len=*), intent(in) :: line
        type(Estudiante), intent(out) :: e
        character(len=100) :: fields(7)
        call split_csv(line, fields, 7)
        e%codigo = trim(fields(1))
        e%genero = trim(fields(2))
        e%fecha_nac = trim(fields(3))
        e%nombres = trim(fields(4))
        e%apellidos = trim(fields(5))
        e%region = trim(fields(6))
        read(fields(7), *) e%promedio_notas  ! Assume , replaced to .
    end subroutine

    subroutine load_correctas(file, corr)
        character(len=*), intent(in) :: file
        type(RespuestaCorrecta), allocatable, intent(out) :: corr(:)
        character(len=1000) :: line
        integer :: unit, ios, count
        unit = 12
        open(unit=unit, file=file, status='old')
        read(unit, *)  ! Skip header
        count = 0
        do
            read(unit, '(A)', iostat=ios) line
            if (ios /= 0) exit
            count = count + 1
        end do
        rewind(unit)
        read(unit, *)  ! Skip header
        allocate(corr(count))
        do i = 1, count
            read(unit, '(A)') line
            call parse_correcta(line, corr(i))
        end do
        close(unit)
    end subroutine

    subroutine parse_correcta(line, c)
        character(len=*), intent(in) :: line
        type(RespuestaCorrecta), intent(out) :: c
        character(len=100) :: fields(101)
        integer :: j  ! Add local j
        call split_csv(line, fields, 101)
        c%prueba = trim(fields(1))
        do j = 1, 100  ! Change i to j
            c%respuestas(j) = fields(j+1)(1:1)
        end do
    end subroutine

    subroutine load_chunk(unit, max_lines, resp)
        integer, intent(in) :: unit, max_lines
        type(RespuestaEstudiante), allocatable, intent(out) :: resp(:)
        character(len=1000) :: line
        character(len=1000), allocatable :: lines(:)
        integer :: count, ios
        allocate(lines(max_lines))
        count = 0
        do i = 1, max_lines
            read(unit, '(A)', iostat=ios) line
            if (ios /= 0) exit
            count = count + 1
            lines(count) = line
        end do
        allocate(resp(count))
        do i = 1, count
            call parse_respuesta(lines(i), resp(i))
        end do
        deallocate(lines)
    end subroutine

    subroutine parse_respuesta(line, r)
        character(len=*), intent(in) :: line
        type(RespuestaEstudiante), intent(out) :: r
        character(len=100) :: fields(102)
        integer :: j  ! Add local j
        call split_csv(line, fields, 102)
        r%estudiante = trim(fields(1))
        r%prueba = trim(fields(2))
        do j = 1, 100  ! Change i to j
            r%respuestas(j) = fields(j+2)(1:1)
        end do
    end subroutine

    subroutine procesar(est, resp, corr, res)
        type(Estudiante), intent(in) :: est(:)
        type(RespuestaEstudiante), intent(in) :: resp(:)
        type(RespuestaCorrecta), intent(in) :: corr(:)
        type(Resultado), allocatable, intent(out) :: res(:)
        integer :: i, j, buenas, omitidas, malas, idx_corr, idx_est
        real :: penalizacion, aciertos, puntaje, pes
        allocate(res(size(resp)))
        do i = 1, size(resp)
            ! Linear search for correctas
            idx_corr = -1
            do j = 1, size(corr)
                if (trim(resp(i)%prueba) == trim(corr(j)%prueba)) then
                    idx_corr = j
                    exit
                end if
            end do
            if (idx_corr == -1) cycle
            ! Linear search for estudiante
            idx_est = -1
            do j = 1, size(est)
                if (trim(resp(i)%estudiante) == trim(est(j)%codigo)) then
                    idx_est = j
                    exit
                end if
            end do
            if (idx_est == -1) cycle
            ! Calculate scores
            buenas = 0
            omitidas = 0
            malas = 0
            do j = 1, 100
                if (len_trim(resp(i)%respuestas(j)) == 0) then
                    omitidas = omitidas + 1
                else if (resp(i)%respuestas(j) == corr(idx_corr)%respuestas(j)) then
                    buenas = buenas + 1
                else
                    malas = malas + 1
                end if
            end do
            penalizacion = malas * 0.25
            aciertos = max(0.0, real(buenas) - penalizacion)
            puntaje = 100.0 + (aciertos / 100.0) * 900.0
            pes = (est(idx_est)%promedio_notas / 7.0) * 1000.0
            res(i)%codigo_estudiante = resp(i)%estudiante
            res(i)%pes = pes
            res(i)%prueba = resp(i)%prueba
            res(i)%buenas = buenas
            res(i)%omitidas = omitidas
            res(i)%malas = malas
            res(i)%puntaje = puntaje
        end do
    end subroutine

    subroutine write_resultados(res, file, write_header)
        type(Resultado), intent(in) :: res(:)
        character(len=*), intent(in) :: file
        logical, intent(in) :: write_header
        integer :: unit
        unit = 13
        if (write_header) then
            open(unit=unit, file=file, status='replace')
            write(unit, *) '"CÓDIGO ESTUDIANTE","PES","PRUEBA","BUENAS","OMITIDAS","MALAS","PUNTAJE"'
        else
            open(unit=unit, file=file, status='old', position='append')
        end if
        do i = 1, size(res)
            write(unit, '(A,A,A,F10.5,A,A,A,I0,A,I0,A,I0,A,F7.2,A)') &
                '"', trim(res(i)%codigo_estudiante), '","', res(i)%pes, '","', &
                trim(res(i)%prueba), '","', res(i)%buenas, '","', res(i)%omitidas, &
                '","', res(i)%malas, '","', res(i)%puntaje, '"'
        end do
        close(unit)
    end subroutine

    subroutine split_csv(line, fields, n)
        character(len=*), intent(in) :: line
        character(len=*), intent(out) :: fields(:)
        integer, intent(in) :: n
        integer :: start, pos, k
        start = 1
        k = 1
        do while (k <= n .and. start <= len_trim(line))
            pos = index(line(start:), ';')
            if (pos == 0) pos = len_trim(line) - start + 2
            fields(k) = line(start:start+pos-2)
            if (fields(k)(1:1) == '"' .and. fields(k)(len_trim(fields(k)):len_trim(fields(k))) == '"') then
                fields(k) = fields(k)(2:len_trim(fields(k))-1)
            end if
            start = start + pos
            k = k + 1
        end do
    end subroutine

end program main