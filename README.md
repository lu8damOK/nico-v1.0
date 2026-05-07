# 🧠 Nico v1.0.1 - Intérprete Educativo de Scripting en Español

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows%20%7C%20Raspberry%20Pi-green)](#)
[![Education](https://img.shields.io/badge/Prop%C3%B3sito-Educativo-orange)](#)

Un intérprete ligero, modular y **100% multiplataforma** para un lenguaje de scripting estructurado en español, diseñado con fines educativos, automatización ligera y aprendizaje de arquitectura de lenguajes.

> 🤖 **Nota:** Este proyecto fue desarrollado con asistencia de IA (Qwen - Alibaba Cloud) bajo dirección humana. El código fuente es abierto, educativo y libre para modificar, distribuir y estudiar.

---

## 📜 Créditos y Atribución

| Rol | Responsable |
|-----|-------------|
| 👨‍💻 Diseño, Arquitectura y Supervisión | **Diego Alejandro Majluff** |
| 🤖 Implementación, Debugging y Optimización | **Qwen (Alibaba Cloud)** |
| 📚 Filosofía | Legibilidad, sintaxis en español nativo, cero dependencias críticas, arquitectura modular |

---

## ✨ Características Principales (v1.0.1)

### 🔹 Tipos de Datos
| Tipo | Descripción | Notas |
|------|-------------|-------|
| `ENTERA` | Entero con signo | 32 bits |
| `DECIMAL` | Punto flotante | `double` precisión estándar C |
| `SIN SIGNO` | Entero grande sin signo | 32 bits |
| `CARACTER` | ASCII simple | 1 byte |
| `TEXTO` | Cadena estática | Longitud fija (`MAX_TEXTO_LEN`) |
| `TEXTO EXTENSO` | Cadena dinámica (heap) | ⚠️ **Solo scope global**, crecimiento limitado por RAM |

### 🔹 Estructuras y Control
- ✅ Condicionales: `SI/SINO`, `SEGÚN CASO`
- ✅ Bucles: `MIENTRAS...HACER`, `PARA...HASTA...PASO...HACER`, `REALIZAR...MIENTRAS`
- ✅ Operadores lógicos con **cortocircuito**: `Y` (AND), `O` (OR)
- ✅ Funciones con retorno, parámetros y **scopes locales anidados**
- ✅ Recursión segura controlada por `profundidad_eval`

### 🔹 E/S y Sistema
- ✅ Entrada/Salida: `LEER()`, `ESCRIBIR()`, `MOSTRAR()`, `SALTO`
- ✅ Archivos: `ABRIRARCHIVO`, `LEERLINEA`, `ESCRIBIRARCHIVO`, `CERRARARCHIVO`
- ✅ Terminal: `LIMPIARPANTALLA`, `CURSOR(x,y)`, `ANCHOTERMINAL()`, `ALTOTERMINAL()`
- ✅ Input no-bloqueante: `TECLAMANTENIDA()` con buffer inteligente ✅
- ✅ **Nuevo en v1.0.1:** Flag `-e` para evaluación rápida (`./nico -e "2+2"`)
- ✅ GPIO nativo Raspberry Pi con fallback seguro en Windows/Linux
- ✅ Input no-bloqueante: `TECLAMANTENIDA()` y **`LEERTECLA($var)`** con decodificación de flechas/ESC ✅

### 🔹 Biblioteca Matemática y Bitwise
- ✅ Funciones nativas: `ABS()`, `POTENCIA()`, `RAIZ()`, `MODULO()`, `MAXIMO()`, `MINIMO()`
- ✅ Trigonométricas: `SENO()`, `COSENO()`, `TANGENTE()`, `ARCOSENO()`, `ARCOCOSENO()`, `ARCOTANGENTE()`
- ✅ Logarítmicas/Exponenciales: `LOGNATURAL()`, `LOGBASE10()`, `LOGBASE2()`, `EXPONENCIAL()`, `DOSALAX()`
- ✅ 🧠 **Machine Learning educativo:** `SIGMOIDE(x)` nativa
- ✅ Redondeo: `REDONDEAR(ARRIBA/ABAJO/ENTERO, valor)`, `QUITARDECIMAL()`
- ✅ Bitwise: `BITY()`, `BITO()`, `BITXOR()`, `ROTARIZQUIERDA()`, `ROTARDERECHA()`, `LEERBIT()`, `ACTIVARBIT()`, `DESACTIVARBIT()`

---

## ⚠️ Nota sobre Precedencia y Paréntesis

En Nico v1.0.1, las expresiones aritméticas se evalúan **estrictamente de izquierda a derecha**, sin precedencia matemática nativa entre operadores.

| Expresión | Evaluación en Nico | Resultado |
|-----------|-------------------|-----------|
| `2 + 3 * 4` | `(2 + 3) * 4` | `20` |
| `10 - 2 ^ 3 + 5` | `((10 - 2) ^ 3) + 5` | `517` |
| `2 + (3 * 4)` | `2 + 12` (paréntesis fuerzan orden) | `14` ✅ |

**Solución recomendada:** Usar **paréntesis explícitos** para garantizar el orden matemático esperado:
```nico
// ✅ Claridad y control total
CALCULAR EN $a = 2 + (3 * 4)          // → 14
CALCULAR EN $b = (10 - 2) * 3         // → 24
CALCULAR EN $c = RAIZ((3 ^ 2) + (4 ^ 2))  // → 5 (Pitágoras)

// ⚠️ Sin paréntesis, se evalúa izquierda a derecha
CALCULAR EN $d = 2 + 3 * 4            // → 20 (no 14)

💡 Esta regla de parsing es intencional en v1.0.1 para mantener transparencia y simplicidad. 
En futuras versiones se planifica un parser con precedencia nativa como feature avanzada.

## 📊 Límites del Lenguaje (v1.0.1)

Profundidad de recursión: 1024 niveles. Controlado por profundidad_eval

Longitud de TEXTO: Fijo por #define. Buffer estático en stack.

TEXTO EXTENSO: Dinámico (RAM). ⚠️ Solo global, no válido en funciones.

Cantidad de TEXTO EXTENSO: Definido en nico.h. Gestión por heap manual.

Dimensiones de listas/matrices: 1D o 2D estáticas. Índices 0..MAX-1.

Longitud máxima de línea: MAX_LINEA chars. Parseo seguro con limpiar_string()

## 📁 Estructura del Proyecto

nico-v1.0.1/
├── nico.h                  # Cabecera central (estructuras, límites, prototipos)
├── main.c                  # Punto de entrada, REPL, flag -e, carga de archivos
├── leertecla.c             # 🆕 Comando nativo LEERTECLA: input no-bloqueante con decodificación ANSI
├── expressions.c           # Evaluador de expresiones, lógica Y/O, funciones built-in
├── funciones.c             # Gestión de funciones, scopes, parámetros, retorno
├── io.c                    # E/S estándar, teclado no-bloqueante, terminal
├── flow.c                  # Control de flujo: bucles, saltos, resolución de etiquetas
├── validador.c             # Validación estática pre-ejecución
├── declaraciones.c         # Parsing de variables, constantes, listas y matrices
├── etiquetas.c             # Registro y gestión de ETIQUETAS y SUBPROGRAMAS
├── cadenas.c               # Motor de manipulación de cadenas y funciones de texto nativas
├── variables.c             # Gestión centralizada de variables globales/locales y tipos
├── nico_bits.c / .h        # Operaciones bitwise y rotación de bits
├── nico_gpio.c / .h        # GPIO nativo Raspberry Pi
├── nico_gpio_stub.c        # Stub seguro para Windows/Linux sin hardware
├── win_compat.h            # Capa de compatibilidad para compilación en Windows
├── compile.sh              # Script de compilación Linux/Raspberry Pi
├── compile_windows.bat     # Script de compilación Windows (MinGW-w64)
├── LICENSE                 # Licencia MIT
├── README.md               # Documentación principal
├── CHANGELOG.md            # Historial de versiones y fixes
├── .gitignore              # Reglas de exclusión para Git
└── ejemplos/
    ├── 01_hola_mundo.nico
    ├── 02_condicionales_y_bucles.nico
    ├── 03_funciones_y_scopes.nico
    ├── 04_operadores_logicos.nico
    ├── 05_archivos_y_texto.nico
    ├── 06_listas_y_matematica.nico
    └── TestPerceptronSimple.nico  # 🧠 Ejemplo de ML con SIGMOIDE() nativa

## ⚙️ Compilación e Instalación
🐧 Linux / Raspberry Pi

# 1. Instalar dependencias
sudo apt update && sudo apt install build-essential gcc -y  # Debian/Ubuntu

# 2. Compilar y ejecutar
chmod +x compile.sh
./compile.sh
./nico ejemplos/01_hola_mundo.nico

# 💡 Raspberry Pi + GPIO:
sudo apt install libgpiod-dev  # Opcional para hardware real

# 🪟 Windows 10/11 (MinGW-w64)

# 1. Agregar C:\mingw64\bin al PATH
# 2. Abrir CMD/PowerShell en la carpeta y ejecutar:
compile_windows.bat
nico.exe ejemplos\01_hola_mundo.nico

⚠️ Antivirus: Windows Defender puede marcar nico.exe como sospechoso al ser un ejecutable nuevo. 
Agregá la carpeta a exclusiones o esperá unos segundos.

## 🚀 Uso Básico
# Modo directo
./nico programa.nico           # Linux/RPi
nico.exe programa.nico         # Windows

# Evaluación rápida (nuevo en v1.0.1)
./nico -e "SIGMOIDE(0)"        # → 0.5
./nico -e "POTENCIA(2, 8)"     # → 256

# Modo interactivo (REPL)
./nico
>>> USAR programa.nico
>>> CORRER
>>> SALIR

## 📦 Ejemplos Incluidos

01_hola_mundo.nico: Estructura base, tipos, ESCRIBIR, LEER
02_condicionales_y_bucles.nico: SI/SINO, MIENTRAS, PARA, REALIZAR
03_funciones_y_scopes.nico: Declaración, parámetros, RETORNAR, ámbito local
04_operadores_logicos.nico: Y/O con cortocircuito, condiciones compuestas
05_archivos_y_texto.nico: ABRIRARCHIVO, lectura/escritura, funciones de cadena
06_listas_y_matematica.nico: Indexación [], iteración, funciones matemáticas
TestPerceptronSimple.nico: 🧠 NUEVO: Perceptrón que aprende puerta AND con SIGMOIDE() nativa

💡 Todos los ejemplos están en UTF-8 sin BOM. Se recomienda VS Code, Notepad++ o nano.

## 🛠️ Solución de Problemas Comunes

1 - gcc: command not found: Compilador no instalado/PATH. sudo apt install build-essential o MinGW-w64.

2 - undefined reference to sin/cos: Falta -lm. Verificar que el script de compilación incluya -lm

3 - multiple definition of 'main': Archivos test_*.c incluidos. Excluirlos antes de compilar.

4 - Caracteres basura (áéíóúÃ): Codificación incorrecta. Guardar en UTF-8 sin BOM. En Windows: chcp 65001.

5 - ñ o acentos se cortan: Uso de CARACTER en lugar de TEXTO. Usar VARIABLE TEXTO + LEER() para input humano.

6 - Permission denied: compile.sh: Sin permisos de ejecución. chmod +x compile.sh

7 - LEERTECLA no detecta teclas: Terminal en modo "cocido" (buffered).  leertecla.c configura raw mode automáticamente; si falla, verificar permisos de terminal. 

## 📄 Licencia
Distribuido bajo Licencia MIT. Uso libre para estudio, modificación y distribución. 
Se agradece mencionar a los autores en trabajos derivados.

Copyright (c) 2026 Diego Alejandro Majluff.
Asistencia en implementación: Qwen (Alibaba Cloud).

## 🐛 ¿Encontraste un bug o tenés una idea?
Abrí un Issue en GitHub incluyendo:

   1 - Sistema operativo y versión exacta.
   2 - Comando de compilación usado.
   3 - Salida completa del error (stdout y stderr).
   4 - Fragmento del .nico que falla + pasos para reproducir.

## 🙌 ¡Gracias por usar Nico v1.0.1!

    "Un lenguaje en español, por y para aprender." 🇦🇷✨

Si este proyecto te sirvió, compartilo, forkéalo o mejorá un ejemplo. La educación abierta crece cuando todos aportamos.
Hecho con ❤️, paciencia y muchas iteraciones de debugging.
