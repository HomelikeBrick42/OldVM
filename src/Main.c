#include "VM.h"
#include "Lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fflush(stdout);
        fprintf(stderr, "Usage: %s <file>", argv[0]);
        return EXIT_FAILURE;
    }

    Lexer lexer;
    if (!Lexer_Create(&lexer, String_FromCString(argv[1]))) {
        return EXIT_FAILURE;
    }

    while (true) {
        Token token = Lexer_NextToken(&lexer);

        if (token.Kind == TokenKind_EndOfFile) {
            break;
        }
    }

    Lexer_Destroy(&lexer);
    return EXIT_SUCCESS;
}

#if 0
#define ENCODE(ptr, type, value) \
    do {                         \
        *(type*)(ptr) = (value); \
        (ptr) += sizeof(type);   \
    } while (0)

uint64_t Func(uint8_t a, uint16_t b, uint64_t* c) {
    *c = a + b;
    return *c + a;
}

int main() {
    uint8_t code[1024] = {};
    uint64_t codeSize  = 0;

    uint64_t a = 0;
    uint64_t b = 0;

    {
        uint8_t* ip = code;

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint64_t*));
        ENCODE(ip, uint64_t*, &b);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(void*));
        ENCODE(ip, void*, Func);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint8_t));
        ENCODE(ip, uint8_t, 5);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint16_t));
        ENCODE(ip, uint16_t, 6);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint64_t*));
        ENCODE(ip, uint64_t*, &a);

        *ip++ = Op_CallCFunc;
        ENCODE(ip, uint64_t, 3);
        // Argument sizes
        ENCODE(ip, uint64_t, sizeof(uint8_t));
        ENCODE(ip, uint64_t, sizeof(uint16_t));
        ENCODE(ip, uint64_t, sizeof(uint64_t*));
        // Return size
        ENCODE(ip, uint64_t, sizeof(uint64_t));

        *ip++ = Op_Store;
        ENCODE(ip, uint64_t, sizeof(uint64_t));

        *ip++ = Op_Exit;

        codeSize = ip - code;
    }

    VM vm;
    VM_Init(&vm, code, codeSize);
    if (!VM_Run(&vm)) {
        VM_PrintStack(&vm);
        return EXIT_FAILURE;
    }

    printf("a=%llu, b=%llu\n", a, b);

    return EXIT_SUCCESS;
}
#endif