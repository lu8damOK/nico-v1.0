/*
 * Nico v1.0.1 - Capa de compatibilidad Windows
 * @file:         win_compat.h
 * @description:  Stub de funciones POSIX para MinGW-w64
 */
#ifndef WIN_COMPAT_H
#define WIN_COMPAT_H

#ifdef _WIN32
#include <windows.h>
#include <string.h>
#include <io.h>

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

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

#if defined(__MINGW64__) && defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0600
    #include <ws2tcpip.h>
    #define NicoUseWSAPoll 1
#else
    #define POLLIN 1
    struct pollfd {
        int fd;
        short events;
        short revents;
    };
    static inline int poll(struct pollfd *fds, unsigned int nfds, int timeout) {
        (void)timeout; (void)nfds;
        if (fds[0].fd == STDIN_FILENO) {
            fds[0].revents = _kbhit() ? POLLIN : 0;
            return fds[0].revents ? 1 : 0;
        }
        return 0;
    }
    #define NicoUseWSAPoll 0
#endif

struct termios { int dummy; };
#define TCSANOW 0
#define TCSADRAIN 0
#define ICANON 0
#define ECHO 0
#define VMIN 0
#define VTIME 0
static inline int tcgetattr(int fd, struct termios *t) { (void)fd; (void)t; return 0; }
static inline int tcsetattr(int fd, int opt, const struct termios *t) { (void)fd; (void)opt; (void)t; return 0; }
static inline int tcdrain(int fd) { (void)fd; return 0; }

#endif /* _WIN32 */
#endif /* WIN_COMPAT_H */
