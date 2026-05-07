# 📜 Changelog - Nico

Todos los cambios notables en este proyecto serán documentados en este archivo.
El formato se basa en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/).

## [1.0.1] - 2026-05-07
### ✅ Agregado
- Flag `-e` para evaluación rápida de expresiones desde terminal (`./nico -e "2+2"`).
- Función nativa `SIGMOIDE(x)` para educación en Machine Learning básico.
- Ejemplo educativo: `ejemplos/TestPerceptronSimple.nico` (Perceptrón aprendiendo puerta AND).
- Buffer inteligente y estable para `TECLAMANTENIDA()` (input no-bloqueante cross-platform).
- Restauración automática de eco y configuración de terminal post-ejecución.

### 🔧 Corregido
- Fuga de contador estático en evaluador de expresiones con recursión inline.
- Desalineación de scope/contexto post-`RETORNAR` en funciones anidadas.
- Terminal quedaba en estado "sin eco" al salir abruptamente (Linux/Windows).
- Caracteres UTF-8 (`ñ`, `á`, `é`) cortados en `VARIABLE CARACTER` → solucionado mediante `VARIABLE TEXTO` + `LEER()`.
- Múltiples warnings de compilador (`-Wunused-variable`, `-Wsequence-point`) limpiados.

### 🔄 Mejorado
- Optimización del loop principal de parsing para mayor estabilidad en modo REPL.
- Capa `win_compat.h` refinada para compilación limpia en MinGW-w64 (Windows 10/11).
- Gestión de `TEXTO EXTENSO` validada y documentada (solo scope global, heap dinámico).
- Validador estructural pre-ejecución más robusto contra parámetros malformados.

### 📚 Documentación
- 📝 **Nota crítica de precedencia:** Nico v1.0.1 evalúa expresiones **estrictamente de izquierda a derecha**. Se documentó el uso obligatorio de paréntesis explícitos para orden matemático estándar (ej: `2 + (3 * 4)`).
- Límites del lenguaje (recursión, buffers, scopes) expuestos en modo interactivo y README.
- Créditos de asistencia IA integrados en encabezados de código y metadata del proyecto.

### 🖥️ Compatibilidad Verificada
- ✅ Linux (GCC, x86_64 / ARM64)
- ✅ Windows 10/11 (MinGW-w64, x86_64)
- ✅ Raspberry Pi OS (ARMv7/ARMv8, GPIO opcional)

### 👥 Créditos
- **Diseño, Arquitectura y Supervisión:** Diego Alejandro Majluff
- **Implementación, Debugging y Optimización:** Qwen (Alibaba Cloud)
- **Licencia:** MIT / Uso Educativo

> 💡 **Nota:** Este release prioriza estabilidad, claridad pedagógica y compatibilidad multiplataforma. La precedencia matemática nativa se planifica como feature principal para `v1.1.0`.
