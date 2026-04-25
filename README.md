# 🧠 Nico v1.0 - Intérprete Educativo de Scripting en Español estilo pseudocódigo.

Un intérprete ligero, modular y multiplataforma para un lenguaje de scripting estructurado en español, diseñado con fines educativos, automatización ligera y aprendizaje de arquitectura de lenguajes.

## 📜 Créditos y Atribución
- **👨‍💻 Diseño, Arquitectura y Supervisión:** Diego Alejandro Majluff
- **🤖 Implementación, Debugging y Optimización:** Qwen (Alibaba Cloud)
- **📚 Filosofía:** Código legible, sintaxis en español nativo, sin dependencias externas críticas, arquitectura modular y listo para extender.

> Este proyecto fue desarrollado íntegramente con asistencia de IA bajo dirección humana. El código fuente es abierto, educativo y libre para modificar, distribuir y estudiar.

## ✨ Características Principales
- ✅ Tipos de datos: `ENTERA`, `DECIMAL`, `CARACTER`, `TEXTO`, `SIN SIGNO`
- ✅ Estructuras de control: `SI/SINO`, `MIENTRAS...HACER`, `PARA...HASTA...PASO...HACER`, `REALIZAR...MIENTRAS`, `SEGUN CASO`
- ✅ Operadores lógicos con cortocircuito: `Y` (AND), `O` (OR)
- ✅ Funciones con retorno y scopes locales anidados
- ✅ Listas y Matrices (globales y locales)
- ✅ E/S de archivos y funciones de texto/cadenas
- ✅ Biblioteca matemática y bitwise completa
- ✅ Soporte nativo GPIO (Raspberry Pi) con fallback seguro en Windows/Linux
- ✅ Validador estructural estático + ejecución runtime robusta

## 📁 Estructura del Proyecto
```text
nico-v1.0/
├── nico.h                  # Cabecera central (estructuras, límites, prototipos)
├── main.c                  # Punto de entrada, carga de archivos, bucle principal, REPL
├── funciones.c             # Gestión de funciones, scopes locales, parámetros, retorno
├── expressions.c           # Evaluación recursiva, lógica Y/O, funciones built-in
├── io.c                    # Entrada/salida estándar y manejo de archivos
├── flow.c                  # Control de flujo, saltos, resolución de etiquetas
├── validador.c             # Validación estática pre-ejecución de bloques
├── declaraciones.c         # Parsing de variables, constantes, listas y matrices
├── etiquetas.c             # Registro y gestión de ETIQUETAS y SUBPROGRAMAS
├── cadenas.c               # Motor de manipulación de cadenas y funciones de texto nativas
├── variables.c             # Gestión centralizada de variables globales/locales y tipos
├── nico_bits.c             # Operaciones bitwise y rotación de bits (BITY, BITO, etc.)
├── nico_bits.h             # Prototipos y constantes para módulo bitwise
├── nico_gpio.c             # Implementación GPIO nativa (Raspberry Pi)
├── nico_gpio_stub.c        # Stub seguro para Windows/Linux sin hardware
├── nico_gpio.h             # Cabecera de abstracción GPIO y tracking de pines
├── win_compat.h            # Capa de compatibilidad para compilación en Windows
├── compile.sh              # Script de compilación para Linux/Raspberry Pi
├── compile_windows.bat     # Script de compilación para Windows (MinGW-w64)
├── LICENSE                 # Licencia MIT
├── README.md               # Documentación principal
├── .gitignore              # Reglas de exclusión para Git
└── ejemplos/               # Scripts de prueba y documentación viva

⚙️ Compilación e Instalación

🐧 Linux / Raspberry Pi
# 1. Instalar dependencias básicas
sudo apt update && sudo apt install build-essential gcc -y  # Debian/Ubuntu
# sudo pacman -S base-devel gcc                             # Arch
# sudo dnf install gcc make                                 # Fedora

# 2. Dar permisos de ejecución al script
chmod +x compile.sh

# 3. Compilar y ejecutar
./compile.sh
./nico ejemplos/01_hola_mundo.nico

💡 Si usás Raspberry Pi y necesitás GPIO real, asegurate de que tu entorno 
tenga libgpiod-dev instalado. El script ya gestiona automáticamente 
nico_gpio_stub.c si no se detecta la librería.

🪟 Windows 10/11

C:\mingw64\bin a la variable PATH del sistema.
Abrir CMD o PowerShell en la carpeta nico-v1.0.
Ejecutar:
compile_windows.bat

⚠️ Nota sobre Antivirus: Windows Defender u otros AV pueden marcar temporalmente
nico.exe como sospechoso por ser un ejecutable nuevo de consola. 
Si se elimina automáticamente, agregá la carpeta del proyecto a las exclusiones 
o esperá 2-3 segundos antes de ejecutar.

🚀 Uso Básico
# Modo directo (ejecutar un script)
./nico programa.nico        # Linux/RPi
nico.exe programa.nico      # Windows

# Modo interactivo (REPL)
./nico
>>> USAR programa.nico
>>> CORRER
>>> SALIR

🛠️ Solución de Problemas Comunes
gcc: command not found
Causa: Compilador no instalado o no en PATH.
Solución: Instalar build-essential (Linux) o MinGW-w64 (Windows).

undefined reference to sin/cos
Causa: Falta la librería matemática en el enlace.
Solución: Asegurar que el comando de compilación incluya el flag -lm.

multiple definition of 'main'
Causa: Archivos de prueba (test_*.c) incluidos en la compilación.
Solución: Eliminar o excluir cualquier archivo con main() extra antes de 
compilar.

Caracteres basura (áéíóú)
Causa: Archivo .nico guardado con codificación incorrecta o con BOM.
Solución: Guardar en UTF-8 sin BOM y ejecutar chcp 65001 en CMD (Windows).

Error de línea con \r\n
Causa: Saltos de línea de Windows no interpretados correctamente.
Solución: limpiar_string() ya los maneja. Si persiste, ejecutar dos2unix 
archivo.nico.

Permission denied: compile.sh
Causa: El script no tiene permisos de ejecución.
Solución: Ejecutar chmod +x compile.sh antes de usarlo.

📦 Ejemplos Incluidos
La carpeta ejemplos/ contiene 6 scripts más otros Test de bonus progresivos para aprender y validar la instalación:

    01_hola_mundo.nico → Estructura base, tipos, ESCRIBIR, LEER
    02_condicionales_y_bucles.nico → SI/SINO, MIENTRAS, PARA, REALIZAR
    03_funciones_y_scopes.nico → Declaración, parámetros, RETORNAR, ámbito local
    04_operadores_logicos.nico → Y/O con cortocircuito, condiciones compuestas
    05_archivos_y_texto.nico → ABRIRARCHIVO, lectura/escritura, funciones de cadena
    06_listas_y_matematica.nico → Indexación [], iteración, funciones matemáticas

💡 Todos los ejemplos están codificados en UTF-8 sin BOM. Se recomienda abrirlos con VS Code, Notepad++ o nano.

📄 Licencia
Este proyecto se distribuye bajo licencia MIT. Podés usarlo, modificarlo y 
distribuirlo libremente. Se agradece mencionar a los autores originales en 
proyectos derivados. Sobre todo el excelente trabajo realizado por la IA Qwen.

🐛 ¿Encontraste un bug o tenés una idea?
Abrí un Issue en GitHub incluyendo:

    Sistema operativo y versión.
    Comando de compilación exacto usado.
    Salida completa del error (stdout y stderr).
    Fragmento del .nico que falla (si aplica).

🙌 ¡¡Gracias por usar Nico v1.0!!
