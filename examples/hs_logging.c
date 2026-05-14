/* hs_logging.c - example of how to use the `hs_logging.h` library

Build Flags:
    - HS_LOGGING_EXAMPLE__INCLUDE_DEFAULT_LOGGERS: build this example with the default
    loggers implementation.
*/
#define HS_LOGGING_IMPLEMENTATION
#include "../hs_logging.h"

#include <assert.h>
#include <time.h>
#include <stdio.h>

/* an example of a custom log handler function 

NOTE(HS): with how the example program is constructed 
*/
void my_custom_log_handler(hsl_LogLevel level, char *file, int line, char *fmt, va_list args);

int main(int argc, char **argv) {
    int i;
    LogHandlerHandle custom_handler;

    /* NOTE(HS): you don't need to do this, but it's present to demonstrate that
    you can mix log handlers as you require
    */
#ifdef HS_LOGGING_EXAMPLE__INCLUDE_DEFAULT_LOGGERS
    assert(use_default_loggers() && "failed to initialise default lib loggers");
#endif

    set_default_log_level(hsl_LogLevel_Trace);

    /* NOTE(HS): tests that you can set the set the global logging level to "Default"

    when compiled without `HS_LOGGING_DO_NOT_ABORT_WHEN_GLOBAL_LOG_LEVEL_SET_TO_DEFAULT`
    then the program should crash here on an assertion
    */
#if 0
    set_default_log_level(hsl_LogLevel_Default);
#endif

    assert(log_handler_add(my_custom_log_handler, LogLevel_Default, &custom_handler) 
        && "failed to register custom log handler");

    assert(log_handler_set_level(custom_handler, LogLevel_Warn) 
        && "could not find handler with matching id");

    log(LogLevel_Trace, __FILE__, __LINE__, "trace message");
    log(LogLevel_Debug, __FILE__, __LINE__, "debug message");
    log(LogLevel_Info,  __FILE__, __LINE__, "info message");
    log(LogLevel_Warn,  __FILE__, __LINE__, "warn message");
    log(LogLevel_Error, __FILE__, __LINE__, "error message");
    log(LogLevel_Fatal, __FILE__, __LINE__, "fatal message");

    hsl_log(hsl_LogLevel_Info, __FILE__, __LINE__, "number of args passed to %s: %i", argv[0], argc - 1);
    for (i = 1; i < argc; ++i) {
        hsl_log(hsl_LogLevel_Info, __FILE__, __LINE__, "argv[%i] = \"%s\"", i, argv[i]);
    }

    return 0;
}


void my_custom_log_handler(hsl_LogLevel level, char *file, int line, char *fmt, va_list args) {
    #define STREAM stdout
    #define TIME_BUFFER_LEN 20
    char *level_str;
    char time_buffer[TIME_BUFFER_LEN];
    time_t now;
    struct tm *tm_val;

    level_str = log_level_to_string(level);

    now = time(NULL);
    tm_val = localtime(&now);

    strftime(time_buffer, TIME_BUFFER_LEN, "%Y-%m-%d %H:%M:%S", tm_val);

    fprintf(STREAM, "[custom::%s] %s %s:%i - ", level_str, time_buffer, file, line);
    vfprintf(STREAM, fmt, args);
    fprintf(STREAM, "\n");
}
