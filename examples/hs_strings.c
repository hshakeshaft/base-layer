#define HS_STRINGS_IMPLEMENTATION
#include "../hs_strings.h"

#include <stdio.h>

int main(void) {
    char *cstr1;
    char *cstr2;
    hss_String str1;
    hss_IAllocator *allocator;
    hss_String_View sv, sv2;

    allocator = &hss_default_allocator;

    printf("hs_strings version: %s\n", hss_api_version());

    cstr1 = "foo";
    printf("\"%s\"\n", cstr1);
    printf("  hss_string_len %i\n", hss_string_len(cstr1));

    hss_string_create(allocator, &str1, 12);
    hss_string_copy("Hello, World", str1);
    printf("\"%s\"\n", str1);
    printf("  hss_string_len %i\n", hss_string_len(str1));
    hss_string_destroy(&str1);

    cstr2 = "          int main(int argc, char **argv)    ";
    sv = hss_string_view(cstr2);
    printf("\""HSS_STRING_VIEW_FMT"\"\n", HSS_STRING_VIEW_ARG(sv));
    hss_string_view_trim(&sv);
    printf("  \""HSS_STRING_VIEW_FMT"\"\n", HSS_STRING_VIEW_ARG(sv));
    sv2 = hss_string_view_chop_by_delim(&sv, ' ');
    printf("  first: \""HSS_STRING_VIEW_FMT"\"\n", HSS_STRING_VIEW_ARG(sv2));
    printf("  rest:  \""HSS_STRING_VIEW_FMT"\"\n", HSS_STRING_VIEW_ARG(sv));

    return 0;
}
