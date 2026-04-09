#define HSS_STATIC
#define HS_STRINGS_IMPLEMENTATION
#include "../hs_strings.h"

#include <stdio.h>

int main(void) {
    printf("hs_strings version: %s\n", hss_api_version());

    printf("hss_strlen %i\n", hss_string_len("foo"));

    return 0;
}
