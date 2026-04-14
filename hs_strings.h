/* @doc hs_strings.h - A library aiming to improve working with strings, whilst
remaining backwards compatible with the LibC functions (and interoperable with other
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
    - 0.2   add hss_String_View type (a mutable, non owning view of a string)
            add functions for manipulating & mutating string views
                hss_string_view_chop_left
                hss_string_view_chop_right
                hss_string_view_chop_left_n
                hss_string_view_chop_right_n
                hss_string_view_trim_left
                hss_string_view_trimg_right
                hss_string_view_trim

    - 0.1   add hss_String type
            add hss_string_len function
            add hss_string_create & hss_string_destroy functions
            add hss_string_copy function
            make hss_string_len compatible with C strings
            make hss_string_destroy compatible with C strings
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

@note the string returned is static
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


/* @doc String type, which contains information about the length and capacity of
said string

@note all allocations for this type include a header struct, which will bloat the
size of the structure somewhat. If you operate in constrained memory environments
you may wish to use another API.

@note API functions which operate on this guarentee the string will be null-terminated,
so this is safe to pass to other APIs which expect this behaviour.
*/
typedef char *hss_String;

/* @doc Contains metadata for the string 

@note the inclusion of `start` as a mmeber is done to add some extra assurrances 
on the API's determination of whether a string is a C string, or count based. By
storing the pointer to the start of the string as well, we can better determine
whether the string was generated by the API, as it is highly unlikely that the pointer
will just so happen to point to the next address.

@member{allocator} allocator used to allocate the string
@member{len} the length of the string
@member{capacity} the capacity of the string (in bytes)
@member{start} pointer to the start of the [hss_String] instance this structure
contains the meta data for
*/
typedef struct hss_String_Header
{
    hss_IAllocator allocator;
    int len;
    size_t capacity;
    char *start;
} hss_String_Header;


/* @doc a mutable, non owning string type

@member{ptr} pointer to the string to take a view of
@member{len} the length of the view to look at
*/
typedef struct hss_String_View
{
    char *ptr;
    int len;
} hss_String_View;

/* @doc (s)printf format specifier string */
#define HSS_STRING_VIEW_FMT "%.*s"

/* @doc unpacks a [hss_String_View] for (s)printf format string */
#define HSS_STRING_VIEW_ARG(SV) (SV).len, (SV).ptr


HSS_DEF int hss_is_ascii_whitepsace(char c);


/* @doc create a string, allocating space for it on the heap via user supplied
allocator

@param{allocator} the allocator to be used to allocate the backing buffer for this
string
@param{str} pointer to [hss_Stirng] instance which will be returned to the user
@param{capacity} the amount of bytes that should be reserved for the string
*/
HSS_DEF int hss_string_create(hss_IAllocator *allocator, hss_String *str, size_t capacity);

/* @doc destroys a string 

@note C-string compatible

@param{str} pointer to string to be destroyed
*/
HSS_DEF int hss_string_destroy(hss_String *str);

/* @doc returns the length of a string, returns -1 of there was an error

@note C-string compatible
@note returns -1 when str is NULL
@note returns -1 when str exceeds [HSS__STRING_MAX_LEN], i.e. i32::max (2,147,483,647)

@param{str} pointer to a string to take the length of
*/
HSS_DEF int hss_string_len(hss_String str);

/* @doc copy a c string into destination

@note failure cases include:
    - src or dest being NULL
    - src len exceeds HSS__STRING_MAX_LEN
    - src having length > (dest capacity)

@param{src} the c string to copy from
@param{dest} the destination to copy the c string to
@return amount of chars copied from src on success, -1 on failure
*/
HSS_DEF int hss_string_copy(char *src, hss_String dest);

/* @doc create a string view from a string

@note C string compatible
*/
HSS_DEF hss_String_View hss_string_view(hss_String str);

/* @doc "chops" the view such that 1 char is remove from the left of the view */
HSS_DEF void hss_string_view_chop_left(hss_String_View *sv);

/* @doc "chops" the view such that 1 char is remove from the right of the view */
HSS_DEF void hss_string_view_chop_rigt(hss_String_View *sv);

/* @doc "chops" the view such that up to n characters, or sv->len (whichever is less)
are removed from the left of the string view.
*/
HSS_DEF void hss_string_view_chop_left_n(hss_String_View *sv, int n);

/* @doc "chops" the view such that up to n characters, or sv->len (whichever is less)
are removed from the right of the string view.
*/
HSS_DEF void hss_string_view_chop_rigt_n(hss_String_View *sv, int n);

/* @doc trims all whitespace from the left side of a string view */
HSS_DEF void hss_string_view_trim_left(hss_String_View *sv);

/* @doc trims all whitespace from the right side of a string view */
HSS_DEF void hss_string_view_trim_right(hss_String_View *sv);

/* @doc trims all whitespace from a string view */
HSS_DEF void hss_string_view_trim(hss_String_View *sv);

/* @doc splits a view into 2 separate views, returning the left hand side of the
view, consuming the delimiter

e.g.
char *str = "Hello World!";
hss_String_View rest = {...};
hss_String_View first = hss_string_view_chop_by_delim(&rest, ' ');

       first     rest
        |         |
String: [Hello][ ][World!]
                ^
                delim
*/
HSS_DEF hss_String_View hss_string_view_chop_by_delim(hss_String_View *sv, char delim);


/*==============================================================================
                                    Implemenataion
==============================================================================*/

#ifdef HS_STRINGS_IMPLEMENTATION

#ifdef HSS_OS_WINDOWS
    #include <BaseTsd.h>
    typedef SSIZE_T hss_isize;
#else
    #include <sys/types.h>
    typedef ssize_t hss_isize;
#endif

#define HS_STRINGS_API_VERSION_MAJOR "0"
#define HS_STRINGS_API_VERSION_MINOR "2"
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


HSS_DEF int hss_is_ascii_whitepsace(char c)
{
    int result;
    result = c == ' ' || c == '\t' || c == '\n' || c == '\r';
    return result;
}


/* ===== Strings ===== */

static hss_String_Header *hss__get_string_header(hss_String str)
{
    hss_String_Header *header;
    header = ((hss_String_Header*) str) - 1;
    return header;
}

HSS_DEF int hss_string_create(hss_IAllocator *allocator, hss_String *str, size_t capacity)
{
    int success;
    hss_String_Header *header;
    hss_String start;
    size_t total_alloc_size;

    success = 0;
    if (!allocator || capacity < 1) { return success; }

    total_alloc_size = sizeof(*header);
    total_alloc_size += (capacity + 1);

    header = allocator->alloc(total_alloc_size, allocator->ctx);
    if (!header) { return success; }

    header->allocator = *allocator;
    header->capacity = capacity;
    header->len = 0;

    start = (hss_String) (header + 1);
    header->start = start;

    *str = start;

    success = 1;
    return success;
}

HSS_DEF int hss_string_destroy(hss_String *str)
{
    hss_String_Header *header;

    header = hss__get_string_header(*str);
    if (header->start == *str)
    {
        hss_IAllocator *allocator;
        size_t total_alloc_size;
        allocator = &header->allocator;
        total_alloc_size = sizeof(*header) + header->capacity;
        allocator->free(header, total_alloc_size, allocator->ctx);
        *str = NULL;
    }
    else
    {
        extern void free(void*);
        free(*str);
        *str = NULL;
    }

    return 0;
}

HSS_DEF int hss_string_len(hss_String str)
{
    int result;
    hss_String_Header *header;

    header = hss__get_string_header(str);
    if (header->start == str)
    {
        result = header->len;
    }
    else
    {
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
    }

    return result;
}

HSS_DEF int hss_string_copy(char *src, hss_String dest)
{
    int bytes_copied;
    int i;
    hss_String_Header *header;
    int src_len;

    if (!src || !dest) { return -1; }

    bytes_copied = 0;
    header = hss__get_string_header(dest);
    src_len = hss_string_len(src);

    if (src_len == -1) { return -1; }
    if ((size_t) src_len > header->capacity) { return -1; }

    i = 0;
    while (src[i])
    {
        dest[i] = src[i];
        i += 1;
    }

    header->len = i;
    bytes_copied = i;

    dest[i] = '\0';

    return bytes_copied;
}

HSS_DEF hss_String_View hss_string_view(hss_String str)
{
    hss_String_View sv;
    sv.ptr = (char*) str;
    sv.len = hss_string_len(str);
    return sv;
}

HSS_DEF void hss_string_view_chop_left(hss_String_View *sv)
{
    if (sv->len > 0)
    {
        sv->ptr++;
        sv->len--;
    }
}

HSS_DEF void hss_string_view_chop_right(hss_String_View *sv)
{
    if (sv->len > 0)
    {
        sv->len-=1;
    }
}

HSS_DEF void hss_string_view_chop_left_n(hss_String_View *sv, int n)
{
    int i;
    n = n <= sv->len ? n : sv->len;
    for (i = 0; i < n; ++i) { hss_string_view_chop_left(sv); }
}

HSS_DEF void hss_string_view_chop_rigt_n(hss_String_View *sv, int n)
{
    int i;
    n = n <= sv->len ? n : sv->len;
    for (i = 0; i < n; ++i) { hss_string_view_chop_right(sv); }
}


HSS_DEF void hss_string_view_trim_left(hss_String_View *sv)
{
    while (hss_is_ascii_whitepsace(*sv->ptr) && sv->len > 0)
    {
        hss_string_view_chop_left(sv);
    }
}

HSS_DEF void hss_string_view_trim_right(hss_String_View *sv)
{
    while (hss_is_ascii_whitepsace(sv->ptr[sv->len - 1]) && sv->len > 0)
    {
        hss_string_view_chop_right(sv);
    }
}

HSS_DEF void hss_string_view_trim(hss_String_View *sv)
{
    hss_string_view_trim_left(sv);
    hss_string_view_trim_right(sv);
}

HSS_DEF hss_String_View hss_string_view_chop_by_delim(hss_String_View *sv, char delim)
{
    hss_String_View first;
    int i;

    i = 0;
    while (i < sv->len && sv->ptr[i] != delim)
    {
        i += 1;
    }

    if (i < sv->len)
    {
        first.ptr = sv->ptr;
        first.len = i;
        hss_string_view_chop_left_n(sv, i + 1);
    }
    else
    {
        first = *sv;
        hss_string_view_chop_left_n(sv, sv->len);
    }

    return first;
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
