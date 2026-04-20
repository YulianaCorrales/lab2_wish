# Wish Shell - Laboratorio de Sistemas Operativos

## Práctica No. 2: API de Procesos

---

## (a) Integrantes

| Nombre Completo | Correo Electrónico | Número de Documento |
|----------------|-------------------|---------------------|
| Yuliana Corrales Castaño | yuliana.corralesc@udea.edu | 39193015 |
| Hellen Jakeline Rubio Casas | hellen.rubio@udea.edu | 1001686406 | 

---

## (b) Documentación de Funciones

### Descripción General

Wish Shell es un intérprete de comandos (CLI) implementado en C que ejecuta comandos del sistema, gestiona procesos, soporta redirección de salida y comandos en paralelo. El shell cumple con la especificación del laboratorio, incluyendo modo interactivo y modo batch.

### Funciones de Gestión de Path

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `init_path()` | Inicializa el path de búsqueda con `/bin` | Ninguno | `void` |
| `free_path()` | Libera la memoria asignada a los directorios del path | Ninguno | `void` |
| `find_executable(command)` | Busca un ejecutable en los directorios del path | `command`: nombre del comando | `char*` (ruta completa) o `NULL` |

### Funciones de Tokenización

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `tokenize_line(line, args)` | Parsea una línea de entrada en tokens, reconociendo operadores `>` y `&` | `line`: línea a parsear, `args`: array para almacenar tokens | `int` (número de tokens) |

### Funciones de Comandos Built-in

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `builtin_exit(args, arg_count)` | Termina la ejecución del shell | `args`: argumentos, `arg_count`: número de argumentos | `int` |
| `builtin_cd(args, arg_count)` | Cambia el directorio de trabajo actual | `args`: argumentos (debe tener 1: el directorio), `arg_count`: número de argumentos | `int` |
| `builtin_path(args, arg_count)` | Modifica la ruta de búsqueda de ejecutables | `args`: lista de directorios, `arg_count`: número de argumentos | `int` |
| `execute_builtin(args, arg_count)` | Determina si un comando es built-in y lo ejecuta | `args`: argumentos, `arg_count`: número de argumentos | `int` |

### Funciones de Redirección

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `check_redirection(args, arg_count, redir_index, outfile)` | Verifica si hay redirección y la valida | `args`: argumentos, `arg_count`: número de argumentos, `redir_index`: índice del `>`, `outfile`: nombre del archivo | `int` |

### Funciones de Ejecución de Comandos Externos

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `execute_external(args, outfile)` | Ejecuta un comando externo con fork/exec | `args`: argumentos del comando, `outfile`: archivo para redirección (o `NULL`) | `void` |

### Funciones de Comandos Paralelos

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `contains_ampersand(args, arg_count)` | Verifica si hay operador `&` en la línea | `args`: argumentos, `arg_count`: número de argumentos | `int` |
| `split_commands(args, arg_count, cmds, cmd_lengths)` | Divide los comandos separados por `&` | `args`: argumentos, `arg_count`: número de argumentos, `cmds`: array de comandos, `cmd_lengths`: longitudes | `int` |
| `execute_parallel(args, arg_count)` | Ejecuta múltiples comandos en paralelo | `args`: argumentos, `arg_count`: número de argumentos | `void` |

### Funciones Auxiliares

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `print_error()` | Imprime el mensaje de error estándar | Ninguno | `void` |
| `free_args(args, count)` | Libera la memoria de los argumentos | `args`: array de argumentos, `count`: número de argumentos | `void` |
| `execute_command(args, arg_count)` | Función principal de despacho de comandos | `args`: argumentos, `arg_count`: número de argumentos | `void` |

### Función Principal

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `main(argc, argv)` | Punto de entrada del shell | `argc`: número de argumentos, `argv`: argumentos de línea de comandos | `int` |

---

## (c) Problemas Presentados y Soluciones

| # | Problema | Descripción | Solución |
|---|----------|-------------|----------|
| 1 | Tokenización de operadores sin espacios | El shell debía reconocer `>` y `&` como operadores incluso cuando no hay espacios (ej: `ls>file` o `ls&ls`) | Se implementó un tokenizador manual que detecta estos caracteres durante el escaneo de la línea, creando tokens separados para ellos independientemente de los espacios |
| 2 | Redirección de stderr | La especificación requería redirigir tanto stdout como stderr al archivo, a diferencia del comportamiento estándar de Unix | En el proceso hijo, después de abrir el archivo, se usó `dup2()` para redirigir ambos file descriptors |
| 3 | Comandos paralelos con fallos intermedios | Si un comando en una secuencia paralela fallaba (ejecutable no encontrado), los comandos ya lanzados quedaban como procesos huérfanos | Se implementó una validación en dos fases: primero se verifican TODOS los ejecutables, y solo si todos existen se procede a forkear |
| 4 | Caracteres ocultos en el token `/tmp` | El comando `cd /tmp` fallaba porque el token llegaba con caracteres no visibles (salto de línea, retorno de carro) | Se mejoró la función `tokenize_line()` para eliminar TODOS los caracteres de control y espacios al final de la línea, y se agregó trim a cada token |
| 5 | Path vacío | Cuando el usuario ejecuta `path` sin argumentos, el path debe quedar vacío y ningún comando externo debe funcionar | `builtin_path()` llama a `free_path()` que limpia el array y establece `path_count = 0` |

---

## (d) Pruebas Realizadas

| # | Prueba | Comandos | Resultado Esperado |
|---|--------|----------|-------------------|
| 1 | Modo Interactivo Básico | `./wish` → `ls -la` → `pwd` → `echo "Hola"` → `exit` | Todos los comandos se ejecutan correctamente |
| 2 | Comando `cd` | `cd /tmp` → `pwd` → `cd /nonexistent` → `cd` | Cambia directorio correctamente, maneja errores |
| 3 | Comando `path` | `path` → `ls` → `path /bin` → `ls` → `path /usr/bin /bin` → `ls` | Path vacío impide ejecución, path con directorios permite ejecución |
| 4 | Redirección con `>` | `ls -la > salida.txt` → `cat salida.txt` → `echo "Prueba" > test.txt` → `ls /nonexistent > error.txt` | Ambos stdout y stderr se redirigen al archivo |
| 5 | Comandos Paralelos con `&` | `ls & echo "Hola" & pwd` → `sleep 3 & echo "Inmediato"` | Los comandos se ejecutan simultáneamente |
| 6 | Modo Batch | `./wish comandos.txt` con archivo que contiene `ls -la`, `pwd`, `echo "Batch"`, `exit` | Se ejecutan los comandos sin mostrar prompt |
| 7 | Manejo de Errores | `./wish arg1 arg2 arg3` → `./wish inexistente.txt` → `comando_inexistente` → `ls > > file` | Todos los errores muestran el mismo mensaje |

### Script Automatizado de Pruebas

```bash
#!/bin/bash
# run_tests.sh

echo "=== Prueba 1: Comandos básicos ==="
./wish -c "ls -la"

echo "=== Prueba 2: Built-in cd ==="
./wish -c "cd /tmp && pwd"

echo "=== Prueba 3: Redirección ==="
./wish -c "ls > test.out && cat test.out"

echo "=== Prueba 4: Comandos paralelos ==="
./wish -c "ls & pwd & echo ok"

echo "=== Prueba 5: Modo batch ==="
./wish batch.txt

echo "=== Todas las pruebas completadas ==="

(e) Enlace al Video
Ítem	Descripción
Enlace	🔗 Link al video de sustentación (10 minutos)
Contenido	Demostración de compilación, ejecución en modo interactivo, prueba de built-in commands (exit, cd, path), demostración de redirección con >, demostración de comandos paralelos con &, ejecución en modo batch, explicación del manejo de errores, revisión del código fuente
(f) Manifiesto de Transparencia - Uso de IA Generativa

Declaración de Integridad
Todo el código fue escrito, revisado y comprendido por los integrantes. La IA se utilizó como herramienta de apoyo para sugerencias, debugging y documentación, pero la implementación final y la comprensión completa del código son 100% de los autores.

