/* @doc hs_logging.h - a library which provides a flexible interface for logging.

@note The library provides the primatives for logging, but leaves the actualy design
of "handlers" to the user, i.e. if you want to log to the console, then you must
write a "console handler" function.

@note This library sets the default, hsl_DEFAULT_LOG_LEVEL value to `hsl_Log_Info`

@note the only true requirement on the C standard libarry in this implementation
is now `stdarg.h`

Usage:
Include this library in any, and all files you require its functionality. Then in
**one and only one source file**, do the following:

    #define HS_LOGGING_IMPLEMENTATION
    #include "hs_logging.h"

Author: Henry Shakeshaft <henry.shakeshaft@live.co.uk>

Contributors:
    - Henry Shakeshaft <henry.shakeshaft@live.co.uk>

Licence: See end of file for details

Customisation/Flags:
    - HS_LOGGING_IMPLEMENTATION - define this to generate the API implementation
    - HS_LOGGING_STATIC - define this to annotate all functions with "static" storage
    specifier
    - HS_LOGGING_NO_SHORT_NAMES - define this to force exclusion of the short API
    names (use in cases where there are namespace collisions)

Changelog:
    - 0.3   Greatly simplify interface, and update API
    - 0.2   Annotate log function with "printf-like" annotations for MSVC, GCC, and
    Clang compilers
                - this will introduce raise warnings at higher levels
    - 0.1   initial implementation of functions
                - add [hsl_log]
                - add [hsl_use_default_loggers]
                - add [hsl_set_default_log_level]
                - add [hsl_log_handler_add]
                - add [hsl_log_handler_set_level]
            add structure to represent the log handler [hsl_LogHandlerId]
            provide interface for users to define log handler functions: [hsl_LogHandlerFnPtr]
            add log levels: [hsl_LogLevel]
*/
#ifndef HS_LOGGING_H_
#define HS_LOGGING_H_
#include <stdarg.h>

#ifndef HSL_DEF
    #ifdef HS_LOGGING_STATIC
        #define HSL_DEF static
    #else
        #define HSL_DEF extern
    #endif
#endif

/* NOTE(HS): this is some "markup" to add printf annotations to logging functions,
which enables warnings to be thrown by the compiler (as it would for incorrect printf
usage) 
*/
#if defined(_MSC_VER)
    #define HSL__PRINTF_LIKE(N, M)
    #if _MSC_VER > 1400
        #define HSL__PRINTF_FMT(FMT) _Printf_format_string_ FMT
    #else
        #define HSL__PRINTF_FMT(FMT) __format_string FMT
    #endif /* FORMAT_STRING */
#elif defined(__clang__) || defined(__GNUC__)
    #define HSL__PRINTF_LIKE(N, M) __attribute__ ((format (printf, N, M)))
    #define HSL__PRINTF_FMT(FMT) FMT
#else
    #define HSL__PRINTF_LIKE(N, M)
    #define HSL__PRINTF_FMT(FMT) FMT
#endif

/* @doc The levels which messages can be logged at

@note `hsl_Log_Default` should only be used to construct new handlers to make them
log at the globally defined minimum level.
*/
typedef enum hsl_Log_Level
{
    /* @doc indicates that level is whatever the default level the library as a
    whole logs at */
    hsl_Log_Default = -1,

    /* @doc log at "TRACE" priority */
    hsl_Log_Trace   = 10,

    /* @doc log at "DEBUG" priority */
    hsl_Log_Debug   = 20,

    /* @doc log at "INFO" priority */
    hsl_Log_Info    = 30,

    /* @doc log at "WARN" priority */
    hsl_Log_Warn    = 40,

    /* @doc log at "ERROR" priority */
    hsl_Log_Error   = 50,

    /* @doc log at "FATAL" priority */
    hsl_Log_Fatal   = 60
} hsl_Log_Level;

/* @doc variable that stores the default logging level this lirbary logs at */
extern hsl_Log_Level hsl_DEFAULT_LOG_LEVEL;

/* @doc A strcture representing a handler for logging */
typedef struct hsl_Log_Handler hsl_Log_Handler;

/* @doc function pointer for a logging handler function

@note this is what users must define an implementation of
@note this should never be called directly

@param ctx userdata, which is supplied from the handler - can be used to represent
arbitrary data the user requires to use in the handler function
@param level the level at which the message is to be logged at
@param file path to the file from where the log function was called
@param line the line number from where the log function was called
@param fmt the "printf-like" format specifier
@param args a variadic argument list which will be used in formatting
*/
typedef void (hsl_Log_Handler_FnPtr)(
    void *ctx, hsl_Log_Level level,
    char *file, int line, char *fmt, va_list args);

/*
@member ctx user defined paramter, which will be passed to the log handler. Use this
to store data required for the function of the logger.
@member min_level the minimum level below which the handler will not trigger
@member handler_fn a function pointer, to a user defined logging function
*/
struct hsl_Log_Handler
{
    void *ctx;
    hsl_Log_Level min_level;
    hsl_Log_Handler_FnPtr *handler_fn;
};


/* @doc convert a log level into a string representation

@note strings returned from here are static litereals, defined in the implementation

@param
*/
HSL_DEF char *hsl_log_level_to_string(hsl_Log_Level level);

/* @doc create a new log handler

@param o_handler the handler which will be written to
@param min_level the minimum level at which the handler will log messages at
@param handler_fn the function this handler should use to log messages with
@param ctx (optional) a pointer to any arbitrary user defined data, which the handler
function requires access to
*/
HSL_DEF int hsl_log_handler_create(
    hsl_Log_Handler *o_handler, hsl_Log_Level min_level,
    hsl_Log_Handler_FnPtr handler_fn, void *ctx);

/* @doc set the library default log level

@param new_default the new default level for messages that the library will log at
*/
HSL_DEF void hsl_log_level_set_default(hsl_Log_Level new_default);

/* @doc get the library default log level */
HSL_DEF hsl_Log_Level hsl_log_level_get_default(void);

/* @doc log a message, using a specified handler

@param handler the handler to be used to log this message
@param level the level at which to log this message
@param file path to the file from where this function was called (use __FILE__)
@param line the line from which this function was called (use __LINE__)
@param fmt the "printf-like" format string which contains the message to print
*/
HSL_DEF void hsl_log(
    hsl_Log_Handler *handler, hsl_Log_Level level,
    char *file, int line, char *HSL__PRINTF_FMT(fmt), ...) HSL__PRINTF_LIKE(5, 6);


#ifndef HS_LOGGING_NO_SHORT_NAMES
    #define Log_Level hsl_Log_Level
        #define Log_Default hsl_Log_Default
        #define Log_Trace   hsl_Log_Trace
        #define Log_Debug   hsl_Log_Debug
        #define Log_Info    hsl_Log_Info
        #define Log_Warn    hsl_Log_Warn
        #define Log_Error   hsl_Log_Error
        #define Log_Fatal   hsl_Log_Fatal
    
    #define log_level_to_string hsl_log_level_to_string

    #define log_level_set_default hsl_log_level_set_default
    #define log_level_get_default hsl_log_level_get_default

    #define Log_Handler         hsl_Log_Handler
    #define log_handler_create  hsl_log_handler_create
    #define log                 hsl_log
#endif  /* HS_LOGGING_NO_SHORT_NAMES */


/* NOTE(HS): An example program which shows how to setup a handler, using custom
context */
#if HS_LOGGING_EXAMPLE == 1
#include <stdio.h>
#include <stdlib.h>

#define HS_LOGGING_IMPLEMENTATION

struct fmt_buffer {
    char *buffer;
    int len;
};

void example_handler(
    void *ctx, hsl_Log_Level level, char *file, int line,
    char *fmt, va_list args)
{
    struct fmt_buffer *buffer;
    char *level_str;
    int offset;
    buffer = (struct fmt_buffer*) ctx;
    level_str = hsl_log_level_to_string(level);
    offset = sprintf(buffer->buffer, "[EXAMPLE::%s] %s:%i - ", level_str, file, line);
    vsprintf(&buffer->buffer[offset], fmt, args);
    printf("%s\n", buffer->buffer);
}

int main(void) {
    hsl_Log_Handler handler;
    struct fmt_buffer buffer;

    buffer.len = 4096;
    buffer.buffer = malloc(sizeof(*buffer.buffer) * buffer.len + 1);

    hsl_log_handler_create(&handler, hsl_Log_Info, example_handler, &buffer);

    hsl_log(&handler, Log_Trace, __FILE__, __LINE__, "should not log");
    hsl_log(
        &handler, hsl_Log_Info, __FILE__, __LINE__,
        "test format: %s | %i", "Hello, World!", 42);

    return 0;
}
#endif  /* HS_LOGGING_EXAMPLE */


/*==============================================================================
                            Library Implementation
==============================================================================*/

#ifdef HS_LOGGING_IMPLEMENTATION

hsl_Log_Level hsl_DEFAULT_LOG_LEVEL = hsl_Log_Info;


HSL_DEF void hsl_log_level_set_default(hsl_Log_Level new_default)
{
    if (new_default == hsl_Log_Default) return;
    switch (new_default)
    {
        case hsl_Log_Trace:
        case hsl_Log_Debug:
        case hsl_Log_Info:
        case hsl_Log_Warn:
        case hsl_Log_Error:
        case hsl_Log_Fatal:
        {} break;

        /* TODO(HS): "unreachable" macro */
        default: {
            * (int*) 0 = 0;
        }
    }
    hsl_DEFAULT_LOG_LEVEL = new_default;
}

HSL_DEF hsl_Log_Level hsl_log_level_get_default(void)
{
    return hsl_DEFAULT_LOG_LEVEL;
}


HSL_DEF char *hsl_log_level_to_string(hsl_Log_Level level)
{
    char *str;
    switch (level)
    {
        case hsl_Log_Default: {
            hsl_log_level_to_string(hsl_log_level_get_default());
        } break;

        case hsl_Log_Trace: { str = "TRACE"; } break;
        case hsl_Log_Debug: { str = "DEBUG"; } break;
        case hsl_Log_Info:  { str = "INFO";  } break;
        case hsl_Log_Warn:  { str = "WARN";  } break;
        case hsl_Log_Error: { str = "ERROR"; } break;
        case hsl_Log_Fatal: { str = "FATAL"; } break;

        /* TODO(HS): unreachable macro */
        default: {
            * (int*) 0 = 0;
        }
    }
    return str;
}

HSL_DEF int hsl_log_handler_create(
    hsl_Log_Handler *o_handler, hsl_Log_Level min_level,
    hsl_Log_Handler_FnPtr handler_fn, void *ctx)
{
    int success;
    success = 0;

    o_handler->ctx = ctx;
    o_handler->handler_fn = handler_fn;

    if (min_level == hsl_Log_Default)
    {
        min_level = hsl_log_level_get_default();
    }
    o_handler->min_level = min_level;

    success = 1;
    return success;
}

HSL_DEF void hsl_log(
    hsl_Log_Handler *handler, hsl_Log_Level level,
    char *file, int line, char *fmt, ...)
{
    if (level >= handler->min_level)
    {
        va_list args;
        va_start(args, fmt);
        handler->handler_fn(handler->ctx, level, file, line, fmt, args);
        va_end(args);
    }
}

#endif  /* HS_LOGGING_IMPLEMENTATION */

#endif  /* HS_LOGGING_H_ */
/*
================================================================================
This software is distributed under a dual licence model.

Choose whichever either suits your preferences, or the requirements of your 
project.
--------------------------------------------------------------------------------
Alternative A: MIT License
--------------------------

Copyright (c) 2026 Henry Shakeshaft

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
--------------------------------------------------------------------------------
Alternative B: Public Domain (The Unlicence)
--------------------------------------------
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <https://unlicense.org>
*/
