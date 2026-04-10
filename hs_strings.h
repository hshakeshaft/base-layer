/* @doc hs_strings.h - A library aiming to improve working with strings, whilst
remaining backwards compatable with the LibC functions (and interoperable with other
_C_ APIs that accept NULL-terminated strings).

Usage:

Include this library in all files where required, the in **one** and only one **source**
file, do the following:

    #define HS_STRINGS_IMPLEMENTATION
    #include "hs_strings.h"

Author: Henry Shakeshaft <henry.shakeshaft@live.co.uk>

Contributors:
    - Henry Shakeshaft <henry.shakeshaft@live.co.uk>

Licence: See end of file for details

Customisation/Flags:
    - HS_STRINGS_IMPLEMENTATION - define this to include the API definition
    - HSS_STATIC - controls whether functions have static or external linkage during
    compilation

Memory Allocation:
  This library provides memory allocation functionality, however this is done differently
to many _C_/_C++_ libraries. The normal way allocation interceptions are handled
is to provide a series of `#define`s for `malloc`, `calloc`, `realloc`, and `free`
from the C standard. This library instead provides an "interface" object which is
passed to functions where allocation is required.

The interface is defined as:

```c
typedef struct hss_IAllocator {
    void *(*alloc)(size_t, void*);
    void *(*realloc)(void*, size_t, size_t, void*);
    void (*free)(void*, size_t, void*);
    void *ctx;
} hss_IAllocator;
```

Where the first 3 members are function pointers, equivalent (loosely) in intended
function to `malloc`, `realloc`, and `free` from the C standard respectively.
In other words you are free to define your own functions, then create a struct
wrapping these, and pass this to the necessary library functions.

The final member is a user defined context variable passed to all interface functions.

e.g. Assume you want to use this library with some arena allocator, the way you
would integrate this may look like something like:

```c
arena_alloc(Arena*, size_t);   // do allocation using arena

void *arena_ialloc_fn(size_t size, void *ctx) {
    return arena_alloc((Arena*) ctx, size);
}

void *arena_irealloc_fn(void *ptr, size_t old_size, size_t new_size, void *ctx) {
    return NULL;
}

void *arena_ifree_fn(void *ptr, size_t size, void *ctx) {
    // do nothing
}

// ...

int main(void) {
    Arena arena;
    hss_IAllocator allocator;
    hss_String str;

    arena_init(&arena, 64);

    allocator.malloc = arena_ialloc_fn;
    allocator.realloc = arena_irealloc_fn;
    allocator.free = arena_ifree_fn;
    allocator.ctx = (void*) &arena;

    // function will allocate 16 bytes for a string using the defined arena
    hss_string_new(&allocator, &str, 16);
}
```

Changelog:
*/
#ifndef HS_STRINGS_H_
#define HS_STRINGS_H_
#include <stddef.h>

/*==============================================================================
                                Complier/OS Switches
        Some utilities required such that the library can switch functionality
    based on certain characteristics of the environment (e.g. OS, or compiler).
==============================================================================*/

#if defined(_WIN32) || defined(_WIN64)
    #ifndef HSS_OS_WINDOWS
        #define HSS_OS_WINDOWS 1
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #ifndef HSS_OS_MACOS
        #define HSS_OS_MACOS 1
    #endif
#elif defined(__unix__)
    #ifndef HSS_OS_UNIX
        #define HSS_OS_UNIX 1
    #endif

    #if defined(__linux__)
        #ifndef HSS_OS_LINUX
            #define HSS_OS_LINUX 1
        #endif
    #else
        #error "unsupported unix operating system"
    #endif
#else
    #error "unsupported operating system"
#endif

#if defined(_MSC_VER)
    #ifndef HSS_COMPILER_MSVC
        #define HSS_COMPILER_MSVC 1
    #endif
#elif defined(__clang__)
    #ifndef HSS_COMPILER_CLANG
        #define HSS_COMPILER_CLANG 1
    #endif
#elif defined(__GNUC__)
    #ifndef HSS_COMPILER_GCC
        #define HSS_COMPILER_GCC 1
    #endif
#else
    #error "unsupported compiler detected"
#endif


/*==============================================================================
                            Function Scope/Exporting
        Controls whether functions have static or extern definitions, and
    also whether functions are exported in a DLL.
==============================================================================*/

#ifndef HSS_DEF
    #ifdef HSS_STATIC
        #define HSS_DEF static
    #else
        #define HSS_DEF extern
    #endif  /* HSS_STATIC */
#endif  /* HSS_DEF */


/* @doc get the version of the API

@note the API version is returned as an integer, which is created by combining the
underlying `major`.`minor`.`patch` numbers in memory.
*/
HSS_DEF const char *hss_api_version(void);


/*==============================================================================
                                Allocator
==============================================================================*/

/* @doc pointer to a function that allocates a block of memory

@param{size} amount of bytes to allocate
@param{ctx} pointer to some user defined data which is part of the interface
*/
typedef void *(hss_IAllocator_Alloc_Fn)(size_t size, void *ctx);

/* @doc pointer to a function that reallocates a block of memory

@param{ptr} amount of bytes to allocate
@param{old_size} the number of bytes allocated for ptr
@param{new_size} the new size that ptr should be reallocated to be
@param{ctx} pointer to some user defined data which is part of the interface
*/
typedef void *(hss_IAllocator_ReAlloc_Fn)(
    void *ptr, size_t old_size, size_t new_size, void *ctx);

/* @doc pointer to a function that frees a block of memory

@param{ptr} pointer to block of memory to free
@param{size} the size of the block of memory pointer to
@param{ctx} pointer to user defined data which is part of the interface
*/
typedef void (hss_IAllocator_Free_Fn)(void *ptr, size_t size, void *ctx);

/* @doc memory allocation interface */
typedef struct hss_IAllocator
{
    hss_IAllocator_Alloc_Fn *alloc;
    hss_IAllocator_ReAlloc_Fn *realloc;
    hss_IAllocator_Free_Fn *free;
    void *ctx;
} hss_IAllocator;

/* @doc default allocator interface provided by this API - this acts as a wrapper
around the Lib C malloc, realloc, and free
*/
extern hss_IAllocator hss_default_allocator;


/*==============================================================================
                                    API Begins
==============================================================================*/

/* @doc the maximum length a string can be in this API */
#define HSS__STRING_MAX_LEN 2147483647

/* @doc returns the length of a string, returns -1 of there was an error

@note returns -1 when str is NULL
@note returns -1 when str exceeds [HSS__STRING_MAX_LEN], i.e. i32::max (2,147,483,647)

@param{str} pointer to a string to take the length of
*/
HSS_DEF int hss_string_len(const char *str);



/*==============================================================================
                                    Implemenataion
==============================================================================*/

#ifdef HS_STRINGS_IMPLEMENTATION

#define HS_STRINGS_API_VERSION_MAJOR "0"
#define HS_STRINGS_API_VERSION_MINOR "1"
#define HS_STRINGS_API_VERSION_PATCH "\0"

HSS_DEF const char *hss_api_version(void)
{
    char *patch;
    patch = HS_STRINGS_API_VERSION_PATCH;
    if (*patch)
    {
        return HS_STRINGS_API_VERSION_MAJOR "." HS_STRINGS_API_VERSION_MINOR "" HS_STRINGS_API_VERSION_PATCH;
    }
    else
    {
        return HS_STRINGS_API_VERSION_MAJOR "." HS_STRINGS_API_VERSION_MINOR;
    }
}


/* ===== IAllocator - Lib C wrapper ===== */

static void *hss__iallocator_libc_alloc(size_t size, void *ctx)
{
    extern void *malloc(size_t);
    void *ptr;
    (void) ctx;
    ptr = malloc(size);
    return ptr;
}

static void *hss__iallocator_libc_realloc(
    void *ptr, size_t old_size, size_t new_size, void *ctx)
{
    extern void *realloc(void*, size_t);
    void *new_ptr;
    (void) old_size; (void) ctx;
    new_ptr = realloc(ptr, new_size);
    return new_ptr;
}

static void hss__iallocator_libc_free(void *ptr, size_t size, void *ctx)
{
    extern void free(void*);
    (void) size; (void) ctx;
    free(ptr);
}

hss_IAllocator hss_default_allocator = {
    hss__iallocator_libc_alloc,
    hss__iallocator_libc_realloc,
    hss__iallocator_libc_free,
    NULL
};


#ifdef HSS_OS_WINDOWS
    #include <BaseTsd.h>
    typedef SSIZE_T hss_isize;
#else
    #include <sys/types.h>
    typedef ssize_t hss_isize;
#endif


HSS_DEF int hss_string_len(const char *str)
{
    int result;
    hss_isize len;

    result = -1;
    len = 0;

    if (!str) { return result; }

    while (str[len++])
    {
        if (len > HSS__STRING_MAX_LEN)
        {
            return result;
        }
    }

    len -= 1;
    result = (int) len;

    return result;
}


#endif  /* HS_STRINGS_IMPLEMENTATION */


#endif  /* HS_STRINGS_H_ */
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
