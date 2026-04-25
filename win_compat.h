/*
 * Nico v1.0 - Intérprete Educativo de Scripting en Español
 * @file:         win_compat.h
 * @author:       Diego Alejandro Majluff (Diseño, Arquitectura y Supervisión)
 * @ai_assist:    Qwen (Alibaba Cloud) - Implementación, Debugging y Optimización
 * @license:      MIT / Personal Use (ver LICENSE)
 * @description:  Capa de compatibilidad Windows: macros, inclusión de <windows.h> 
 *                y definiciones POSIX simuladas.
 */
#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

#ifdef _WIN32
#include <windows.h>
#include <string.h>

#define sleep(sec) Sleep((DWORD)((sec) * 1000))

static inline int nico_usleep_win(unsigned int usec) {
    if (usec > 0) {
        DWORD ms = (DWORD)(usec / 1000 + (usec % 1000 > 0 ? 1 : 0));
        Sleep(ms);
    }
    return 0;
}
#define usleep(usec) nico_usleep_win(usec)
#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)

#endif 
#endif
