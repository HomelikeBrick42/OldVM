#include "Strings.h"

#include <string.h>

String String_FromCString(const char* cstring) {
    return (String){
        .Data   = (uint8_t*)cstring,
        .Length = strlen(cstring),
    };
}

bool String_Equal(String a, String b) {
    if (a.Length != b.Length) {
        return false;
    }

    if (a.Data == b.Data) {
        return true;
    }

    for (uint64_t i = 0; i < a.Length; i++) {
        if (a.Data[i] != b.Data[i]) {
            return false;
        }
    }

    return true;
}
