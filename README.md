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
