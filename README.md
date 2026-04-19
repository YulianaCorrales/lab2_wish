# Wish Shell - Laboratorio de Sistemas Operativos

## Práctica No. 2: API de Procesos

---

## (a) Integrantes

| Nombre Completo | Correo Electrónico | Número de Documento |
|----------------|-------------------|---------------------|
| Yuliana Corrales Castaño | yuliana.corralesc@udea.edu | 39193015 |
| Hellen Rubio| [correo2@universidad.edu] | [Documento 2] |

---

## (b) Documentación de Funciones

### Descripción General

Wish Shell es un intérprete de comandos (CLI) implementado en C que ejecuta comandos del sistema, gestiona procesos, soporta redirección de salida y comandos en paralelo. El shell cumple con la especificación del laboratorio, incluyendo modo interactivo y modo batch.

---

### Funciones de Gestión de Path

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `init_path()` | Inicializa el path de búsqueda con `/bin` | Ninguno | `void` |
| `free_path()` | Libera la memoria asignada a los directorios del path | Ninguno | `void` |
| `find_executable(command)` | Busca un ejecutable en los directorios del path | `command`: nombre del comando | `char*` (ruta completa) o `NULL` |

---

### Funciones de Tokenización

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `tokenize_line(line, args)` | Parsea una línea de entrada en tokens, reconociendo operadores `>` y `&` | `line`: línea a parsear, `args`: array para almacenar tokens | `int` (número de tokens) |

**Comportamiento:**
- Elimina caracteres de control (`\n`, `\r`, `\t`, espacios)
- Reconoce `>` y `&` como tokens separados incluso sin espacios
- Realiza trim (elimina espacios alrededor de cada token)

---

### Funciones de Comandos Built-in

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `builtin_exit(args, arg_count)` | Termina la ejecución del shell | `args`: argumentos, `arg_count`: número de argumentos | `int` |
| `builtin_cd(args, arg_count)` | Cambia el directorio de trabajo actual | `args`: argumentos (debe tener 1: el directorio), `arg_count`: número de argumentos | `int` |
| `builtin_path(args, arg_count)` | Modifica la ruta de búsqueda de ejecutables | `args`: lista de directorios, `arg_count`: número de argumentos | `int` |
| `execute_builtin(args, arg_count)` | Determina si un comando es built-in y lo ejecuta | `args`: argumentos, `arg_count`: número de argumentos | `int` (1 si fue built-in, 0 si no) |

**Validaciones:**
- `exit`: solo acepta 0 argumentos adicionales
- `cd`: exactamente 1 argumento (el directorio destino)
- `path`: 0 o más argumentos (sobrescribe la ruta anterior)

---

### Funciones de Redirección

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `check_redirection(args, arg_count, redir_index, outfile)` | Verifica si hay redirección y la valida | `args`: argumentos, `arg_count`: número de argumentos, `redir_index`: índice del `>`, `outfile`: nombre del archivo | `int` (0: sin redir, 1: redir válida, -1: error) |

**Reglas de validación:**
- Solo un operador `>` por comando
- Debe haber exactamente un archivo después de `>`
- No puede haber más tokens después del archivo

---

### Funciones de Ejecución de Comandos Externos

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `execute_external(args, outfile)` | Ejecuta un comando externo con fork/exec | `args`: argumentos del comando, `outfile`: archivo para redirección (o NULL) | `void` |

**Flujo de ejecución:**
1. Busca el ejecutable con `find_executable()`
2. Crea un proceso hijo con `fork()`
3. Si hay redirección, redirige stdout y stderr con `dup2()`
4. Ejecuta el programa con `execv()`
5. El padre espera con `waitpid()`

---

### Funciones de Comandos Paralelos

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `contains_ampersand(args, arg_count)` | Verifica si hay operador `&` en la línea | `args`: argumentos, `arg_count`: número de argumentos | `int` (1 si hay `&`, 0 si no) |
| `split_commands(args, arg_count, cmds, cmd_lengths)` | Divide los comandos separados por `&` | `args`: argumentos, `arg_count`: número de argumentos, `cmds`: array de comandos, `cmd_lengths`: longitudes | `int` (número de comandos) |
| `execute_parallel(args, arg_count)` | Ejecuta múltiples comandos en paralelo | `args`: argumentos, `arg_count`: número de argumentos | `void` |

**Flujo de ejecución paralela:**
1. Divide la línea en comandos individuales
2. Valida todos los comandos y rutas ANTES de forkear
3. Crea un proceso hijo para cada comando
4. Espera a que todos terminen con `waitpid()`

---

### Funciones Auxiliares

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `print_error()` | Imprime el mensaje de error estándar | Ninguno | `void` |
| `free_args(args, count)` | Libera la memoria de los argumentos | `args`: array de argumentos, `count`: número de argumentos | `void` |
| `execute_command(args, arg_count)` | Función principal de despacho de comandos | `args`: argumentos, `arg_count`: número de argumentos | `void` |

---

### Función Principal

| Función | Descripción | Parámetros | Retorno |
|---------|-------------|------------|---------|
| `main(argc, argv)` | Punto de entrada del shell | `argc`: número de argumentos, `argv`: argumentos de línea de comandos | `int` |

**Modos de operación:**
- **Modo interactivo**: `./wish` - muestra prompt `wish> `
- **Modo batch**: `./wish archivo.txt` - lee comandos desde archivo

---

## (c) Problemas Presentados y Soluciones

### Problema 1: Tokenización de operadores sin espacios

**Descripción:**  
El shell debía reconocer `>` y `&` como operadores incluso cuando no hay espacios (ej: `ls>file` o `ls&ls`).

**Solución:**  
Se implementó un tokenizador manual que detecta estos caracteres durante el escaneo de la línea, creando tokens separados para ellos independientemente de los espacios.

```c
if (line[i] == '>' || line[i] == '&') {
    char op[2];
    op[0] = line[i];
    op[1] = '\0';
    args[arg_count++] = strdup(op);
    i++;
}
Problema 2: Redirección de stderr
Descripción:
La especificación requería redirigir tanto stdout como stderr al archivo, a diferencia del comportamiento estándar de Unix.

Solución:
En el proceso hijo, después de abrir el archivo, se usó dup2() para redirigir ambos file descriptors:

c
dup2(fd, STDOUT_FILENO);
dup2(fd, STDERR_FILENO);
Problema 3: Comandos paralelos con fallos intermedios
Descripción:
Si un comando en una secuencia paralela fallaba (ejecutable no encontrado), los comandos ya lanzados quedaban como procesos huérfanos.

Solución:
Se implementó una validación en dos fases: primero se verifican TODOS los ejecutables, y solo si todos existen se procede a forkear.

c
// Primera fase: validación
for (int c = 0; c < cmd_count; c++) {
    full_paths[c] = find_executable(cmds[c][0]);
    if (full_paths[c] == NULL) {
        valid = 0;
        break;
    }
}
// Segunda fase: ejecución solo si valid = 1
Problema 4: Caracteres ocultos en el token /tmp
Descripción:
El comando cd /tmp fallaba porque el token llegaba con caracteres no visibles (salto de línea, retorno de carro).

Solución:
Se mejoró la función tokenize_line() para eliminar TODOS los caracteres de control y espacios al final de la línea, y se agregó trim a cada token.

c
// Eliminar caracteres al final de la línea
while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' || 
       line[len - 1] == '\t' || line[len - 1] == ' ')) {
    line[len - 1] = '\0';
    len--;
}

// Trim del token
char *trimmed = token;
while (*trimmed && isspace(*trimmed)) trimmed++;
Problema 5: Path vacío
Descripción:
Cuando el usuario ejecuta path sin argumentos, el path debe quedar vacío y ningún comando externo debe funcionar.

Solución:
builtin_path() llama a free_path() que limpia el array y establece path_count = 0.

(d) Pruebas Realizadas
Prueba 1: Modo Interactivo Básico
Comandos:

bash
./wish
wish> ls -la
wish> pwd
wish> echo "Hola mundo"
wish> exit
Resultado esperado: Todos los comandos se ejecutan correctamente

Prueba 2: Comando cd
Comandos:

bash
wish> pwd
/home/user/lab2_wish
wish> cd /tmp
wish> pwd
/tmp
wish> cd /nonexistent
An error has occurred
wish> cd
An error has occurred
Resultado esperado: Cambia directorio correctamente, maneja errores

Prueba 3: Comando path
Comandos:

bash
wish> path
wish> ls
An error has occurred
wish> path /bin
wish> ls
[listado de archivos]
wish> path /usr/bin /bin
wish> ls
[listado de archivos]
Resultado esperado: Path vacío impide ejecución, path con directorios permite ejecución

Prueba 4: Redirección con >
Comandos:

bash
wish> ls -la > salida.txt
wish> cat salida.txt
[contenido del ls]
wish> echo "Prueba" > test.txt
wish> cat test.txt
Prueba
wish> ls /nonexistent > error.txt
wish> cat error.txt
ls: cannot access '/nonexistent'...
Resultado esperado: Ambos stdout y stderr se redirigen al archivo

Prueba 5: Comandos Paralelos con &
Comandos:

bash
wish> ls & echo "Hola" & pwd
[las salidas aparecen en orden indeterminado]
wish> sleep 3 & echo "Inmediato"
Inmediato
[después de 3 segundos, el prompt aparece]
Resultado esperado: Los comandos se ejecutan simultáneamente

Prueba 6: Modo Batch
Archivo comandos.txt:

text
ls -la
pwd
echo "Batch mode"
exit
Ejecución:

bash
./wish comandos.txt
Resultado esperado: Se ejecutan los comandos sin mostrar prompt

Prueba 7: Manejo de Errores
Comandos:

bash
./wish arg1 arg2 arg3
An error has occurred

./wish archivo_inexistente.txt
An error has occurred

wish> comando_inexistente
An error has occurred

wish> ls > > file
An error has occurred
Resultado esperado:  Todos los errores muestran el mismo mensaje

Script Automatizado de Pruebas
bash
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
🔗 Link al video de sustentación (10 minutos)

El video incluye:

Demostración de compilación

Ejecución en modo interactivo

Prueba de todos los built-in commands (exit, cd, path)

Demostración de redirección con >

Demostración de comandos paralelos con &

Ejecución en modo batch

Explicación del manejo de errores

Revisión del código fuente

(f) Manifiesto de Transparencia - Uso de IA Generativa

Detalle de uso de IA Generativa
ChatGPT (OpenAI):

Generación de documentación: Se utilizó para estructurar el README y generar las tablas de documentación de funciones.

Sugerencias para el tokenizador: Se recibió asistencia para manejar correctamente los operadores sin espacios.

Debugging: Se utilizó para diagnosticar el problema de cd /tmp y se implementó la solución de trim de tokens.

Validación en paralelo: Se recibió orientación sobre cómo implementar la validación en dos fases para comandos paralelos.

Declaración de Integridad:
Todo el código fue escrito, revisado y comprendido por los integrantes. La IA se utilizó como herramienta de apoyo para sugerencias, debugging y documentación, pero la implementación final y la comprensión completa del código son 100% de los autores.

Estructura del Repositorio
text
lab2_wish/
├── README.md          # Este archivo
├── wish.c             # Código fuente del shell
├── Makefile           # Archivo de compilación
├── pruebas/
│   ├── test_batch.txt
│   ├── test_redirection.txt
│   ├── test_parallel.txt
│   ├── test_path.txt
│   └── test_cd.txt
└── run_tests.sh       # Script de pruebas automatizadas

Compilación y Ejecución
Compilar
bash
make
Ejecutar en modo interactivo
bash
./wish
Ejecutar en modo batch
bash
./wish comandos.txt
Limpiar archivos generados
bash
make clean
