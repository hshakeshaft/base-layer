/* @doc hs_logging.h - a library providing a flexible interface for logging.

@note: this particular version of the library was deprecated as I found it too complex,
and limiting upon usage in a more real program. an equivalent is available in the
project root which has a better API design (I believe).

@note this library provides some default logging handlers, but expects you to provide
the actual handler code.

@note this logging library sets the default, global, logging level to `hsl_LogLevel_Info`

Usage:
Include this library in all files where required, the in **one** and only one **source**
file, do the following:

    #define HS_LOGGING_IMPLEMENTATION
    #include "hs_logging.h"

Author: Henry Shakeshaft <henry.shakeshaft@live.co.uk>

Contributors:
    - Henry Shakeshaft <henry.shakeshaft@live.co.uk>

Licence: See end of file for details

Notes:
    - The set of log handlers is assumed to be a dynamic array with initial capacity
    for 16 log handlers.
    - Upon exceeding the number of defined handlers, the memory will be reallocated
    and capacity increased (by a factor of 2).
    - If users require access to "custom data" for their log handler implementation,
    this currently requires users to access \[their own\] globally defined data
    (such as file handles)
    - All log handlers are triggered on all log events, i.e. if you have a handler
    configured to log to stdout, and another which logs to some file, a single call
    to [hsl_log] will trigger a message to be send to both stdout and the file (in
    whatever order the handlers were registered in)

Customisation/Flags:
    - HS_LOGGING_IMPLEMENTATION - define this to include the API definition.
    - HSL_ASSERT - define a custom assertion function, if not defined the library
    provides a definition, which will log a message, and call the standard library
    assert.
    - HS_LOGGING_STATIC - define this to force all functions to have static storage
    class, if not defined functions defined as `extern`.
    - HS_LOGGING_DO_NOT_ABORT_WHEN_GLOBAL_LOG_LEVEL_SET_TO_DEFAULT - define this
    if you do not wish the library to assert & potentially crash when the user attempts
    to set the default (global) log level to `hsl_LogLevel_Default`. If not defined,
    and either your definition of `HSL_ASSERT`, or the default library definition,
    are configured to crash the program, they will if the code path is triggered.
    - HS_LOGGING_NO_STDIO - define this to remove all reliance on `stdio.h` from
    the API.
        - NOTE(HS): removes call to `printf` in the default library implementation
        of `HSL_ASSERT`.
    - HS_LOGGING_NO_DEFAULT_LOGGERS - define this to not build with the default
    loggers.
        - NOTE(HS): implicitly the default loggers rely on `stdio.h`.
        - NOTE(HS): if `HS_LOGGING_NO_STDIO` is defined then this is implicit.
    - allocator overrides:
        - NOTE(HS): either define none, or define all, failure to do the later will
        raise a compiler error
        - HSL_MALLOC - override memory allcotion behavior, defaults to `malloc`
        - HSL_REALLOC - override memory reallocation behaviour, defaults to `realloc`
        - HSL_FREE - override memory freeing behaviour, defaults to `free`
    - HSL_MEMCPY: to add handlers into the set of handlers, this requires use of
    `memcpy`, to allow maximal divorce from the C standard library users are given
    interface to override this if requried.
    - HS_LOGGING_NO_SHORT_NAMES - define this to remove all `#define` aliases, where
    the prefix has been stripped. Use when namespace conflicts arise.

Changelog:
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

#ifndef HSLOG_DEF
    #ifdef HS_LOGGING_STATIC
        #define HSLOG_DEF static
    #else
        #define HSLOG_DEF extern
    #endif
#endif  /* HSLOG_DEF */

#ifndef HSL_ASSERT
    #include <assert.h>

    #ifndef HS_LOGGING_NO_STDIO
    #include <stdio.h>
    #endif  /* HS_LOGGING_NO_STDIO */

    #define HSL__DO_STRINGIFY(S) #S
    #define HSL__STRINGIFY(S) HSL__DO_STRINGIFY(S)

    #ifndef HS_LOGGING_NO_STDIO
    #define HSL_ASSERT(COND, MSG)                                           \
        do {                                                                \
            if (!(COND)) {                                                  \
                printf(                                                     \
                    "%s:%i - invalid condition encountered: %s - %s\n",     \
                    __FILE__, __LINE__,                                     \
                    HSL__STRINGIFY(COND), MSG                               \
                );                                                          \
            }                                                               \
            assert(0 && (MSG));                                             \
        } while (0)
    #else
    #define HSL_ASSERT(COND, MSG)                                           \
        do {                                                                \
            assert((COND) && (MSG));                                        \
        } while (0)
    #endif  /* HS_LOGGING_NO_STDIO */
#endif  /* HSL_ASSERT */

#if !defined(HSL_MALLOC) || !defined(HSL_REALLOC) || !defined(HSL_FREE)
#include <stdlib.h>
#define HSL_MALLOC(SIZE) malloc(SIZE)
#define HSL_REALLOC(PTR, SIZE) realloc(PTR, SIZE)
#define HSL_FREE(PTR) free(PTR)

#elif \
    defined(HSL_MALLOC) && (!defined(HSL_REALLOC) || !defined(HSL_FREE)) \
    defined(HSL_REALLOC) && (!defined(HSL_MALLOC) || !defined(HSL_FREE)) \
    defined(HSL_FREE) && (!defined(HSL_MALLOC) || !defined(HSL_REALLOC))
#error "you cannot provide partial override of default library allcoator functions"
#endif  /* !defined(HSL_MALLOC) || !defined(HSL_REALLOC) || !defined(HSL_FREE) */


#ifndef HSL_MEMCPY
#include <string.h>
#define HSL_MEMCPY(DEST, SRC, SIZE) memcmp(DEST, SRC, SIZE)
#endif  /* HSL_MEMCPY */

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


/* @doc enum representing the level at which a message should be logged

@note the [hsl_LogLevel_Default] enum should only be used in construction of new
handlers to make them log at whatever the global default level is.
*/
typedef enum
{
    /* @doc use this to indicate the global default level should be used by this
    logger to log at

    @note if you attempt to set the global log level to this it will raise an assertion
    unless you define the library assertion macro away, if the program does not crash
    after asserting, then the library **will not update** the global level
    */
    hsl_LogLevel_Default = -1,

    /* @doc indicate the message should be logged at "TRACE" priority */
    hsl_LogLevel_Trace   = 10,

    /* @doc indicate the message should be logged at "DEBUG" priority */
    hsl_LogLevel_Debug   = 20,

    /* @doc indicate the message should be logged at "INFO" priority */
    hsl_LogLevel_Info    = 30,

    /* @doc indicate the message should be logged at "WARN" priority */
    hsl_LogLevel_Warn    = 40,

    /* @doc indicate the message should be logged at "ERROR" priority */
    hsl_LogLevel_Error   = 50,

    /* @doc indicate the message should be logged at "FATAL" priority

    @note should be used to indicate when an irrecoverable error was encountered
    */
    hsl_LogLevel_Fatal   = 60
} hsl_LogLevel;

/* @doc a pointer to a function which will handle the actual logging of you message.
For example, a handler which prints a message in the following format to `stdout`

```
[$LEVEL] $FILE:$LINE_NO - $FMT
```
*/
typedef void (hsl_LogHandlerFnPtr)(
    hsl_LogLevel level,
    char *file, int line,
    char *fmt, va_list args
);

/* @doc A "handle" to a log handler implementation

@note primarily used to wrap an integer, and provide some extra type safety for
the API

@note if `id` is set to `-1`, this means the handle was invalid
*/
typedef struct { int id; } hsl_LogHandlerHandle;


/* @doc converts a [hsl_LogLevel] enumerant to a string representation */
HSLOG_DEF char *hsl_log_level_to_string(hsl_LogLevel level);

/* @doc initialises a list of log handlers, and then populates it with the default
library implementations

@note the default handler implementations include:
- a console logger, which prints to stdout
*/
HSLOG_DEF int hsl_use_default_loggers(void);


/* @doc set the default global level to log messages at

@note if you attempt to set the global default to be [hsl_LogLevel_Default], **and**
do not define `HS_LOGGING_DO_NOT_ABORT_WHEN_GLOBAL_LOG_LEVEL_SET_TO_DEFAULT`, then
this will run an assertion. If the assertion does not trigger program exit, the
default global level is left unchanged.

@param level the level at which all handlers log at default
*/
HSLOG_DEF void hsl_set_default_log_level(hsl_LogLevel level);


/* @doc add a handler function to the list of log handlers, which logs at a specified
level

@note set `level` to [hsl_LogLevel_Default] to use the global, default log level

@param handler_fn pointer to a function which actually does the logging
@param level the minimum level at which messages passed to the handler will log at
@param o_handler a pointer to a handle which will be set, so the user can update
the log handler later

@returns `1` when a handler is successfully inserted into the list of handlers,
`0` if there was some error
*/
HSLOG_DEF int hsl_log_handler_add(
    hsl_LogHandlerFnPtr *handler_fn,
    hsl_LogLevel level,
    hsl_LogHandlerHandle *o_handler
);

/* @doc sets the level of a configured handler to whatever was provided

@param handler the handle to a logger, which is to have its log level updated
@param level the new minimum level at which handler messages will be logged at

@returns 1 if the handler was found, and updated successfully, 0 if the logger was
not found
*/
HSLOG_DEF int hsl_log_handler_set_level(hsl_LogHandlerHandle handler, hsl_LogLevel level);

/* @doc log a message, which will be logged by all loggers registerd in the system

@param level the level at which the message should be logged at
@param file the file the log event was dispatched from (__FILE__)
@param line the line on which the log event was dispatched from (__LINE__)
@param fmt a format string to print
*/
HSLOG_DEF void hsl_log(
    hsl_LogLevel level,
    char *file, int line,
    char * HSL__PRINTF_FMT(fmt),
    ...) HSL__PRINTF_LIKE(4, 5);


/* names where the `hsl` prefix is stripped - undef this if you don't want this */
#ifndef HS_LOGGING_NO_SHORT_NAMES
    #define LogLevel            hsl_LogLevel
    #define LogLevel_Default    hsl_LogLevel_Default
    #define LogLevel_Trace      hsl_LogLevel_Trace
    #define LogLevel_Debug      hsl_LogLevel_Debug
    #define LogLevel_Info       hsl_LogLevel_Info
    #define LogLevel_Warn       hsl_LogLevel_Warn
    #define LogLevel_Error      hsl_LogLevel_Error
    #define LogLevel_Fatal      hsl_LogLevel_Fatal

    #define LogHandlerFnPtr     hsl_LogHandlerFnPtr
    #define LogHandlerHandle    hsl_LogHandlerHandle

    #define log_level_to_string     hsl_log_level_to_string
    #define use_default_loggers     hsl_use_default_loggers
    #define set_default_log_level   hsl_set_default_log_level
    #define log_handler_add         hsl_log_handler_add
    #define log_handler_set_level   hsl_log_handler_set_level
    #define log                     hsl_log
#endif  /* HS_LOGGING_NO_SHORT_NAMES */


/*==============================================================================
                                Library Implementation
==============================================================================*/
#ifdef HS_LOGGING_IMPLEMENTATION

#ifndef HS_LOGGING_NO_STDIO
#include <stdio.h>
#endif  /* HS_LOGGING_NO_STDIO */

/* internal representation of a "log handler"/logger

NOTE(HS): the `id` here maps to [hsl_LogHandlerHandle::id]
*/
typedef struct
{
    hsl_LogHandlerFnPtr *handler_fn;
    hsl_LogLevel level;
    int id;
} hsl__LogHandler;

/* internal representation of the set of log handlers that are currently registered
to the system
*/
typedef struct
{
    hsl__LogHandler *elems;
    int count;
    int capacity;
} hsl__LogHandlerList;


/* list of available log handlers */
static hsl__LogHandlerList hsl__log_handler_list;

/* the next id value for a log handler */
static int hsl__next_log_handler_id = 1;

/* the default log level */
hsl_LogLevel hsl__default_log_level = hsl_LogLevel_Info; 

/* initialise the list of log handlers */
static int hsl__log_handlers_init(void)
{
    if (hsl__log_handler_list.elems)
    {
        return 1;
    }

    hsl__log_handler_list.capacity = 16;
    hsl__log_handler_list.count = 0;
    hsl__log_handler_list.elems = HSL_MALLOC(
        sizeof(*hsl__log_handler_list.elems) * hsl__log_handler_list.capacity
    );

    if (!hsl__log_handler_list.elems)
    {
        return 0;
    }

    return 1;
}

/* do the actual logging, which involves:
    1. looping through the list of available handlers
    2. if the supplied level is >= level of the handler the call the handler function
*/
static void hsl__do_log(hsl_LogLevel level, char *file, int line, char *fmt, va_list args)
{
    int i;
    hsl__LogHandler *handler;
    for (i = 0; i < hsl__log_handler_list.count; ++i)
    {
        handler = &hsl__log_handler_list.elems[i];
        if (level >= handler->level)
        {
            handler->handler_fn(level, file, line, fmt, args);
        }
    }
}

#if defined(HS_LOGGING_NO_DEFAULT_LOGGERS) || defined(HS_LOGGING_NO_STDIO)
#else
/* a default implementation of a log handler, which simply prints the message to
`stdout`
*/
static void hsl__default_log_handler_console_fn(
    hsl_LogLevel level,
    char *file, int line,
    char *fmt, va_list args
)
{
    char *level_str;
    level_str = hsl_log_level_to_string(level);
    fprintf(stdout, "[%s] %s:%i - ", level_str, file, line);
    vfprintf(stdout, fmt, args);
    fprintf(stdout, "\n");
}
#endif  /* defined(HS_LOGGING_NO_DEFAULT_LOGGERS) || defined(HS_LOGGING_NO_STDIO) */


HSLOG_DEF char *hsl_log_level_to_string(hsl_LogLevel level)
{
    char *str;
    switch (level)
    {
        case hsl_LogLevel_Default: { str = hsl_log_level_to_string(hsl__default_log_level); } break;

        case hsl_LogLevel_Trace: { str = "TRACE"; } break;
        case hsl_LogLevel_Debug: { str = "DEBUG"; } break;
        case hsl_LogLevel_Info:  { str = "INFO";  } break;
        case hsl_LogLevel_Warn:  { str = "WARN";  } break;
        case hsl_LogLevel_Error: { str = "ERROR"; } break;
        case hsl_LogLevel_Fatal: { str = "FATAL"; } break;

        default: { str = NULL; * (int*) 0 = 0; /* TOOD: "unreachable" */ }
    }
    return str;
}

HSLOG_DEF int hsl_use_default_loggers(void)
{
    hsl_LogHandlerHandle console_handler;

    if (!hsl__log_handlers_init()) { return 0; }

#if defined(HS_LOGGING_NO_DEFAULT_LOGGERS) || defined(HS_LOGGING_NO_STDIO)
    (void) console_handler;
#else
    if (!hsl_log_handler_add(hsl__default_log_handler_console_fn, -1, &console_handler))
    {
        return 0;
    }
#endif  /* defined(HS_LOGGING_NO_DEFAULT_LOGGERS) || defined(HS_LOGGING_NO_STDIO) */

    return 1;
}

HSLOG_DEF int hsl_log_handler_add(
    hsl_LogHandlerFnPtr handler_fn,
    int level,
    hsl_LogHandlerHandle *o_handler_id
)
{
    int result;
    int next_handler_id;
    int next_handler_offset;
    hsl__LogHandler handler;
    void *next_handler_addr;

    if (!hsl__log_handler_list.elems)
    {
        int init_success;
        init_success = hsl__log_handlers_init();
        if (!init_success) { return 0; }
    }

    result = 0;

    /* dynamic array impl */
    if (hsl__log_handler_list.count + 1 >= hsl__log_handler_list.capacity)
    {
        int new_capacity;
        void *new_elems;
        new_capacity = hsl__log_handler_list.capacity * 2;
        new_elems = HSL_REALLOC(hsl__log_handler_list.elems, new_capacity);
        if (!new_elems) { return 0; }
        hsl__log_handler_list.elems = (hsl__LogHandler*) new_elems;
        hsl__log_handler_list.capacity = new_capacity;
    }

    next_handler_offset = hsl__log_handler_list.count;
    next_handler_id = hsl__next_log_handler_id;

    handler.handler_fn = handler_fn;
    handler.level = level;
    handler.id = next_handler_id;

    next_handler_addr = &(hsl__log_handler_list.elems[next_handler_offset]);
    /* TODO: check if memcpy succeeds */
    memcpy(next_handler_addr, (void*) &handler, sizeof(handler));

    hsl__log_handler_list.count += 1;

    hsl__next_log_handler_id += 1;
    o_handler_id->id = next_handler_id;

    result = (next_handler_id > 0) ? 1 : 0;
    return result;
}

HSLOG_DEF void hsl_set_default_log_level(hsl_LogLevel level)
{
    if (level == hsl_LogLevel_Default)
    {
#ifndef HS_LOGGING_DO_NOT_ABORT_WHEN_GLOBAL_LOG_LEVEL_SET_TO_DEFAULT
        HSL_ASSERT(level != hsl_LogLevel_Default, "cannot");
#endif
        return;
    }
    hsl__default_log_level = level;
}

HSLOG_DEF int hsl_log_handler_set_level(hsl_LogHandlerHandle handler_id, hsl_LogLevel level)
{
    int i;
    for (i = 0; i < hsl__log_handler_list.count; ++i)
    {
        hsl__LogHandler *handler;
        handler = &(hsl__log_handler_list.elems[i]);
        if (handler->id == handler_id.id)
        {
            handler->level = level;
            return 1;
        }
    }
    return 0;
}

HSLOG_DEF void hsl_log(hsl_LogLevel level, char *file, int line, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    hsl__do_log(level, file, line, fmt, args);
    va_end(args);
}

#endif  /* HS_LOGGING_IMPLEMENTATION */

/*
# Todos/Design Consideratoins

Some notes on API design, thoughts for improvements, and tasks to do at later date

## TODO: Remove dynamic capacity array features of handler list
- there is no reason to make the log handler list dynamic
- users of the API will know ahead of time how many log handlers are required
- instead API should return an error when attempting to insert a new log handler
when you exceed the max capacity
- requires addition of new "capacity" macro (HSL_HANDLER_CAPACITY)
    - if not defined set to 2 as default
    - this is I think the minimum for a user to get started as it includes space
    for the default (console) logger, and for (one of) their own handler(s)

## Design: Add ability to log with specified handler only
- I can forsee it being useful to allow for logging where only specified handlers
are used to log particular messages.
- would allow for users to create several handlers, e.g. create one per "system"
in your application (e.g. gameplay vs engine systems for a videogame)
- may require tagging of handlers as "global" i.e. ones where all log events are
sent to these regardless of whether the handle matches the provided one

## Design: Provide `ctx` var during handler instantiation
- it may be worth providing some "userdata" to the log handlers
- this could be used to store a file handle for logs to be written to
- improves some integration effort as users aren't then forced into using global
data as they now are
- userdata should be an optional parameter (i.e. nullable)
- userdata will be a `void*` and all memory associated with said pointer must be
managed by the API user
    - it is then up to the user to manipulate the void pointer in the handler function
    as required

## Maybe: Parallelise dispatch of events to handlers
- currently all log events are dispatched, sequentially, to each handler registered
with the library
- if users have many log handlers, then you may want to run the "global" handlers
in a multithreaded env
*/

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
