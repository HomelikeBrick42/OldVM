#include "Emitter.h"

#include <stdlib.h>
#include <memory.h>

ARRAY_IMPL(uint8_t, Byte);

bool Emitter_Create(Emitter* emitter, Lexer lexer) {
    *emitter = (Emitter){
        .Lexer = lexer,
        .Code  = ByteArray_Create(),
    };
    return true;
}

void Emitter_Destroy(Emitter* emitter) {
    ByteArray_Destroy(&emitter->Code);
    Lexer_Destroy(&emitter->Lexer);
}

bool Emitter_Emit(Emitter* emitter) {
    return true;
}
