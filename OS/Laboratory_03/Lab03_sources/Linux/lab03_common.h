#ifndef LAB03_COMMON_H
#define LAB03_COMMON_H

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

static int parse_positive(const char* text, uint64_t* value) {
    if (text == NULL || *text == '\0' || *text == '-') return 0;
    char* end = NULL;
    errno = 0;
    unsigned long long parsed = strtoull(text, &end, 10);
    if (errno == ERANGE || *end != '\0' || parsed == 0) return 0;
    *value = (uint64_t)parsed;
    return 1;
}

#endif
