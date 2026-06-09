#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#define HS_LOGGING_IMPLEMENTATION
#include "../hs_logging.h"

void stdout_handler_fn(void *ctx, hsl_Log_Level level, char *file, int line, char *fmt, va_list args)
{
    #define BUFFER_SIZE 4096
    char buffer[BUFFER_SIZE];
    int offset;
    char *level_str;
    (void) ctx;
    level_str = hsl_log_level_to_string(level);
    offset = sprintf(buffer, "[STDOUT::%s] %s:%i - ", level_str, file, line);
    offset += vsprintf(&buffer[offset], fmt, args);
    fprintf(stdout, "%s\n", buffer);
}

void stderr_handler_fn(void *ctx, hsl_Log_Level level, char *file, int line, char *fmt, va_list args)
{
    #define BUFFER_SIZE 4096
    char buffer[BUFFER_SIZE];
    int offset;
    char *level_str;
    (void) ctx;
    level_str = hsl_log_level_to_string(level);
    offset = sprintf(buffer, "[STDERR::%s] %s:%i - ", level_str, file, line);
    offset += vsprintf(&buffer[offset], fmt, args);
    fprintf(stderr, "%s\n", buffer);
}


int main(int argc, char **argv) {
    int i;
    hsl_Log_Handler stdout_handler;
    hsl_Log_Handler stderr_handler;

    assert(
        hsl_log_handler_create(&stdout_handler, hsl_Log_Info, stdout_handler_fn, NULL) &&
        "failed to create log handler [stdout]");

    assert(
        hsl_log_handler_create(&stderr_handler, hsl_Log_Error, stderr_handler_fn, NULL) &&
        "failed to create log handler [stderr]");


    { /* NOTE(HS): log to stdout handler */
        hsl_log(&stdout_handler, hsl_Log_Trace, __FILE__, __LINE__, "test trace");
        hsl_log(&stdout_handler, hsl_Log_Debug, __FILE__, __LINE__, "test debug");
            /* NOTE(HS): should only trigger stdout from here on */
        hsl_log(&stdout_handler, hsl_Log_Info,  __FILE__, __LINE__, "test info");
        hsl_log(&stdout_handler, hsl_Log_Warn,  __FILE__, __LINE__, "test warning");
        hsl_log(&stdout_handler, hsl_Log_Error, __FILE__, __LINE__, "test error");
        hsl_log(&stdout_handler, hsl_Log_Fatal, __FILE__, __LINE__, "test fatal");
    }

    { /* NOTE(HS): log to stderr handler */
        hsl_log(&stderr_handler, hsl_Log_Trace, __FILE__, __LINE__, "test trace");
        hsl_log(&stderr_handler, hsl_Log_Debug, __FILE__, __LINE__, "test debug");
        hsl_log(&stderr_handler, hsl_Log_Info,  __FILE__, __LINE__, "test info");
        hsl_log(&stderr_handler, hsl_Log_Warn,  __FILE__, __LINE__, "test warning");
        /* NOTE(HS): should only trigger stderr from here on */
        hsl_log(&stderr_handler, hsl_Log_Error, __FILE__, __LINE__, "test error");
        hsl_log(&stderr_handler, hsl_Log_Fatal, __FILE__, __LINE__, "test fatal");
    }

    for (i = 0; i < argc; i++) {
        hsl_log(
            &stdout_handler, hsl_Log_Info, __FILE__, __LINE__,
            "argv[%i] = \"%s\"", i, argv[i]);

        /* NOTE(HS): should not trigger logging */
        hsl_log(
            &stderr_handler, hsl_Log_Info, __FILE__, __LINE__,
            "argv[%i] = \"%s\"", i, argv[i]);
    }

    return 0;
}
