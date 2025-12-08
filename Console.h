/*
 * Console.h - Cross-Platform Console Functions
 * Works on Windows, Linux, and macOS
 */
#pragma once
#include <iostream>

// Auto-detect platform
#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_UNIX)
    #if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__)
        #define PLATFORM_WINDOWS
    #else
        #define PLATFORM_UNIX
    #endif
#endif

// Platform-specific includes
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    #include <conio.h>
#else
    #include <unistd.h>
    #include <termios.h>
    #include <fcntl.h>
    #include <cstdlib>
    #include <sys/select.h>
    #include <cstdio>
#endif

// Unix terminal management
#ifdef PLATFORM_UNIX
namespace ConsoleInternal {
    static struct termios orig_termios;
    static bool terminal_initialized = false;
    static bool terminal_saved = false;
}

inline void restore_terminal() {
    if (ConsoleInternal::terminal_initialized && ConsoleInternal::terminal_saved) {
        tcsetattr(STDIN_FILENO, TCSANOW, &ConsoleInternal::orig_termios);
        // Reset non-blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
        ConsoleInternal::terminal_initialized = false;
    }
}

inline void init_terminal() {
    if (!ConsoleInternal::terminal_initialized) {
        // Save original terminal settings
        if (tcgetattr(STDIN_FILENO, &ConsoleInternal::orig_termios) == 0) {
            ConsoleInternal::terminal_saved = true;
            atexit(restore_terminal);
        }
        
        struct termios new_termios = ConsoleInternal::orig_termios;
        // Disable canonical mode (line buffering) and echo
        new_termios.c_lflag &= ~(ICANON | ECHO);
        // Set minimum characters to read
        new_termios.c_cc[VMIN] = 0;
        new_termios.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
        
        // Set non-blocking mode
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        
        ConsoleInternal::terminal_initialized = true;
    }
}
#endif

// Clear screen
inline void clrscr() {
#ifdef PLATFORM_WINDOWS
    system("cls");
#else
    std::cout << "\033[2J\033[H";
    std::cout.flush();
#endif
}

// Hide cursor
inline void hideCursor() {
#ifdef PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\033[?25l";
    std::cout.flush();
#endif
}

// Show cursor
inline void showCursor() {
#ifdef PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
    std::cout << "\033[?25h";
    std::cout.flush();
#endif
}

// Move cursor to position
inline void gotoxy(int x, int y) {
#ifdef PLATFORM_WINDOWS
    std::cout.flush();
    HANDLE hConsoleOutput;
    COORD dwCursorPosition;
    dwCursorPosition.X = x;
    dwCursorPosition.Y = y;
    hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hConsoleOutput, dwCursorPosition);
#else
    std::cout.flush();
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
#endif
}

// Check if key pressed (non-blocking)
inline bool check_kbhit() {
#ifdef PLATFORM_WINDOWS
    return _kbhit() != 0;
#else
    // Use select() for reliable non-blocking check
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    
    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) > 0;
#endif
}

// Get single character (blocking)
inline int get_single_char() {
#ifdef PLATFORM_WINDOWS
    return _getch();
#else
    // Temporarily switch to blocking mode for reliable read
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    
    int c = getchar();
    
    // Restore non-blocking mode
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    
    return c;
#endif
}

// Get character if available (non-blocking, returns -1 if none)
inline int get_char_nonblocking() {
#ifdef PLATFORM_WINDOWS
    if (_kbhit()) {
        return _getch();
    }
    return -1;
#else
    if (check_kbhit()) {
        return getchar();
    }
    return -1;
#endif
}

// Sleep for milliseconds
inline void sleep_ms(int milliseconds) {
#ifdef PLATFORM_WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// Initialize console
inline void init_console() {
#ifdef PLATFORM_UNIX
    init_terminal();
#endif
}

// Cleanup console
inline void cleanup_console() {
#ifdef PLATFORM_UNIX
    restore_terminal();
#endif
}

// Text colors
enum class Color {
    Black, Blue, Green, Aqua, Red, Purple, Yellow, White,
    Gray, LightBlue, LightGreen, LightAqua, LightRed, LightPurple, LightYellow, BrightWhite
};

inline void set_color(Color color) {
#ifdef PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, static_cast<int>(color));
#else
    static const char* colors[] = {
        "\033[30m", "\033[34m", "\033[32m", "\033[36m",
        "\033[31m", "\033[35m", "\033[33m", "\033[37m",
        "\033[90m", "\033[94m", "\033[92m", "\033[96m",
        "\033[91m", "\033[95m", "\033[93m", "\033[97m"
    };
    std::cout << colors[static_cast<int>(color)];
#endif
}

inline void reset_color() {
    set_color(Color::White);
}

