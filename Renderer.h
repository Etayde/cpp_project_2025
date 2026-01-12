#pragma once

#include "Console.h"
#include <iostream>
#include <string>

class Renderer {
private:
    inline static bool silentMode = false;  // Static inline flag - set once at Game construction

public:
    // Set rendering mode (called once by Game constructor)
    static void setSilentMode(bool silent) {
        silentMode = silent;
    }

    // Check if we should render
    static inline bool shouldRender() {
        return !silentMode;
    }

    // Rendering methods - wrap Console.h functions
    static inline void gotoxy(int x, int y) {
        if (shouldRender()) ::gotoxy(x, y);
    }

    static inline void clrscr() {
        if (shouldRender()) ::clrscr();
    }

    static inline void hideCursor() {
        if (shouldRender()) ::hideCursor();
    }

    static inline void showCursor() {
        if (shouldRender()) ::showCursor();
    }

    static inline void sleep_ms(int milliseconds) {
        if (shouldRender()) ::sleep_ms(milliseconds);
    }

    // Convenience methods for common patterns
    static inline void print(char c) {
        if (shouldRender()) std::cout << c;
    }

    static inline void print(const std::string& str) {
        if (shouldRender()) std::cout << str;
    }

    static inline void print(int value) {
        if (shouldRender()) std::cout << value;
    }

    static inline void flush() {
        if (shouldRender()) std::cout << std::flush;
    }

    // Combined operations for common patterns
    static inline void printAt(int x, int y, char c) {
        if (shouldRender()) {
            ::gotoxy(x, y);
            std::cout << c;
        }
    }

    static inline void printAt(int x, int y, const std::string& str) {
        if (shouldRender()) {
            ::gotoxy(x, y);
            std::cout << str;
        }
    }
};
