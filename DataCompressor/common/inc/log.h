/* Logging macros (header only)
   Part of DataCompressor
   Andreas Unterweger, 2013-2015 */

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>

#define LOG(f, ...) {\
  if ((f) != NULL) \
    fprintf((f), __VA_ARGS__); \
  }

/* Output errors in red on the console */
#ifndef COLORED_ERROR_MESSAGES
  #define COLORED_ERROR_MESSAGES
#endif

#ifdef COLORED_ERROR_MESSAGES
  #ifdef __linux__ /* Linux */
    #define LOG_ERROR(f, ...) {\
      LOG((f), "\033[0;31m"); /* Switch to red color */ \
      LOG((f), __VA_ARGS__); /* Print message */ \
      LOG((f), "\033[0m"); /* Switch back to default color */ \
    }
  #elif defined(_WIN32) /* Windows */
    #pragma warning(push, 0) /* Disable warnings for Windows headers */
    #include <windows.h>
    #pragma warning(pop) /* Restore original warning level */
    #define LOG_ERROR(f, ...) {\
      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
      CONSOLE_SCREEN_BUFFER_INFO info; \
      GetConsoleScreenBufferInfo(hConsole, &info); /* Retrieve default console attributes */ \
      SetConsoleTextAttribute(hConsole, (info.wAttributes & ~0xF) | 4); /* Switch to red color */ \
      LOG((f), __VA_ARGS__); /* Print message */ \
      SetConsoleTextAttribute(hConsole, info.wAttributes); /* Switch back to default color */ \
    }
  #else /* Other OS (print with regular format) */
    #define LOG_ERROR(f, ...) LOG((f), __VA_ARGS__)
  #endif
#else /* No colored error messages */
  #define LOG_ERROR(f, ...) LOG((f), __VA_ARGS__)
#endif

#ifdef _DEBUG
  #define LOG_DEBUG(f, ...) LOG((f), __VA_ARGS__)
#else
  #define LOG_DEBUG(f, ...) {}
#endif

#endif