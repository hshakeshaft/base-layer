#define HSS_STATIC
#define HS_STRINGS_IMPLEMENTATION
#include "../hs_strings.h"

#include <stdio.h>

int main(void) {
    char *cstr1;
    hss_String str1;
    hss_IAllocator *allocator;

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

    return 0;
}
