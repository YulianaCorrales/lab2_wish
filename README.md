# Wish Shell - Laboratorio de Sistemas Operativos

## Práctica No. 2: API de Procesos

---

## (a) Integrantes

| Nombre Completo | Correo Electrónico | Número de Documento |
|----------------|-------------------|---------------------|
| Yuliana Corrales Castaño | yuliana.corralesc@udea.edu | 39193015 |
| Hellen Rubio | hellen.rubio@udea.edu | 1001686406 |

---

## (b) Documentación de Funciones

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `init_path()` | Inicializa el path de búsqueda con `/bin` | Ninguno | `void` |
| `free_path()` | Libera la memoria asignada a los directorios del path | Ninguno | `void` |
| `find_executable(command)` | Busca un ejecutable en los directorios del path | `commit`: nombre del comando | `char*` o `NULL` |
| `tokenize_line(line, args)` | Parsea una línea en tokens, reconociendo `>` y `&` | `line`: línea a parsear, `args`: array de tokens | `int` |
| `builtin_exit(args, arg_count)` | Termina la ejecución del shell | `args`: argumentos, `arg_count`: número | `int` |
| `builtin_cd(args, arg_count)` | Cambia el directorio de trabajo actual | `args`: argumentos, `arg_count`: número | `int` |
| `builtin_path(args, arg_count)` | Modifica la ruta de búsqueda | `args`: lista de directorios, `arg_count`: número | `int` |
| `builtin_route(args, arg_count)` | Comando alternativo para modificar el path | `args`: lista de directorios, `arg_count`: número | `int` |
| `execute_builtin(args, arg_count)` | Determina si un comando es built-in y lo ejecuta | `args`: argumentos, `arg_count`: número | `int` |
| `check_redirection(args, arg_count, redir_index, outfile)` | Verifica si hay redirección válida | `args`, `arg_count`, `redir_index`, `outfile` | `int` |
| `execute_external(args, outfile)` | Ejecuta un comando externo con fork/exec | `args`: argumentos, `outfile`: archivo o `NULL` | `void` |
| `contains_ampersand(args, arg_count)` | Verifica si hay operador `&` en la línea | `args`: argumentos, `arg_count`: número | `int` |
| `split_commands(args, arg_count, cmds, cmd_lengths)` | Divide comandos separados por `&` | `args`, `arg_count`, `cmds`, `cmd_lengths` | `int` |
| `execute_parallel(args, arg_count)` | Ejecuta múltiples comandos en paralelo | `args`: argumentos, `arg_count`: número | `void` |
| `print_error()` | Imprime el mensaje de error estándar | Ninguno | `void` |
| `free_args(args, count)` | Libera la memoria de los argumentos | `args`: array, `count`: número | `void` |
| `execute_command(args, arg_count)` | Función principal de despacho de comandos | `args`: argumentos, `arg_count`: número | `void` |
| `main(argc, argv)` | Punto de entrada del shell | `argc`: número, `argv`: argumentos | `int` |

---

## (c) Problemas Presentados y Soluciones

| # | Problema | Descripción | Solución |
|---|----------|-------------|----------|
| 1 | Tokenización de operadores sin espacios | El shell debía reconocer `>` y `&` sin espacios (ej: `ls>file`) | Tokenizador manual que detecta caracteres especiales |
| 2 | Redirección de stderr | La especificación pedía redirigir stdout y stderr al archivo | Uso de `dup2()` para redirigir ambos file descriptors |
| 3 | Comandos paralelos con fallos intermedios | Comandos ya lanzados quedaban como huérfanos si uno fallaba | Validación en dos fases: verificar todos antes de forkear |
| 4 | Caracteres ocultos en tokens | `cd /tmp` fallaba por saltos de línea ocultos | Mejora en `tokenize_line()` con trim de tokens |
| 5 | Path vacío | Al ejecutar `path` sin argumentos, el path debe quedar vacío | `free_path()` limpia el array y establece `path_count = 0` |
| 6 | Comando `route` no reconocido | El comando `route` no estaba en `execute_builtin` | Se agregó `builtin_route` y su llamada en `execute_builtin` |

---

## (d) Pruebas Realizadas

### Prueba 4: Ejecución de Comandos Externos y PATH

| ID | Prueba | Comando | Resultado Esperado |
|----|--------|---------|-------------------|
| 4.1 | PATH inicial | `ls` | Funciona (`/bin` está en path) |
| 4.2 | PATH cambiado | `route /tmp` → `ls` | Error (`ls` no está en `/tmp`) |
| 4.3 | PATH restaurado | `route /bin` → `ls` | Funciona nuevamente |

### Prueba 5: Redirección con `>`

| ID | Prueba | Comando | Resultado Esperado |
|----|--------|---------|-------------------|
| 5.1 | Redirección básica | `ls > output.txt` → `cat output.txt` | Archivo contiene salida de `ls` |
| 5.2 | Sobrescritura | `ls > output.txt` (dos veces) | El archivo se sobrescribe |
| 5.3 | stderr redirigido | `ls /inexistente > output.txt` | Error dentro del archivo |
| 5.4 | Múltiples `>` | `ls > f1 > f2` | Error |
| 5.5 | Sin archivo | `ls >` | Error |
| 5.6 | Tokens extra | `ls > out extra` | Error |

### Prueba 6: Comandos Paralelos con `&`

| ID | Prueba | Comando | Resultado Esperado |
|----|--------|---------|-------------------|
| 6.1 | Dos comandos | `ls & pwd` | Ambos se ejecutan, orden variable |
| 6.2 | Tres comandos | `ls & pwd & whoami` | Todos terminan antes del prompt |
| 6.3 | Con argumentos | `ls -la /tmp & echo hola & pwd` | Todos funcionan |
| 6.4 | Vacío antes de `&` | `& ls` | Error |
| 6.5 | Vacío después de `&` | `ls &` | Error |
| 6.6 | Múltiples `&` | `ls & & pwd` | Error |

### Prueba 7: Redirección + Paralelismo

| ID | Prueba | Comando | Resultado Esperado |
|----|--------|---------|-------------------|
| 7.1 | Redirección en paralelo | `ls > f1.txt & pwd > f2.txt` | Ambos archivos creados |
| 7.2 | Redirección inválida en paralelo | `ls > f1 > f2 & pwd` | Error |

### Prueba 8: Modo Batch

| ID | Prueba | Resultado Esperado |
|----|--------|-------------------|
| 8.1 | Líneas vacías | Se ignoran, ejecuta comandos |
| 8.2 | Built-in `cd` | Cambia directorio correctamente |
| 8.3 | Redirección | Archivo se crea con la salida |
| 8.4 | Comandos paralelos | Se ejecutan en paralelo |
| 8.5 | Error y continuación | Imprime error y sigue |

### Prueba 9: Mensajes de Error

| ID | Prueba | Comando | Resultado Esperado |
|----|--------|---------|-------------------|
| 9.1 | Comando inexistente | `comando_invalido` | `An error has occurred` |
| 9.2 | Continuar tras error | `comando_invalido` → `ls` | El segundo funciona |

### Prueba 10: Fin de Archivo (EOF)

| ID | Prueba | Comando | Resultado Esperado |
|----|--------|---------|-------------------|
| 10.1 | EOF en interactivo | `Ctrl+D` | Sale con `exit(0)` |

---

## (e) Enlace al Video

| Ítem | Descripción |
|------|-------------|
| Enlace | 🔗 [Link al video de sustentación (10 minutos)](https://youtu.be/[ID_DEL_VIDEO]) |
| Contenido | Compilación, modo interactivo, built-ins (`exit`, `cd`, `path`, `route`), redirección, comandos paralelos, modo batch, manejo de errores, EOF |

---

## (f) Manifiesto de Transparencia - Uso de IA Generativa

### Declaración de Integridad

Todo el código fue escrito, revisado y comprendido por los integrantes. La IA se utilizó como herramienta de apoyo para sugerencias, debugging y documentación, pero la implementación final y la comprensión completa del código son 100% de los autores.

---

## Estructura del Repositorio

| Archivo/Directorio | Descripción |
|--------------------|-------------|
| `README.md` | Este archivo |
| `wish.c` | Código fuente del shell |
| `Makefile` | Archivo de compilación |

---

## Compilación y Ejecución

| Acción | Comando |
|--------|---------|
| Compilar | `gcc -Wall -o wish wish.c` |
| Modo interactivo | `./wish` |
| Modo batch | `./wish comandos.txt` |

**Fecha de entrega:** 19/04/2026

**Curso:** Laboratorio de Sistemas Operativos

**Universidad:** Universidad de Antioquia
