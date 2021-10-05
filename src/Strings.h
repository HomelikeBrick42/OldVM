#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct String {
    uint8_t* Data;
    uint64_t Length;
} String;

#define String_FromLiteral(s)    \
    ((String){                   \
        .Data   = (uint8_t*)(s), \
        .Length = sizeof(s) - 1, \
    })

#define String_Fmt(s) (uint32_t)(s).Length, (s).Data

String String_FromCString(const char* cstring);
bool String_Equal(String a, String b);
