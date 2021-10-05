#pragma once

#include "Lexer.h"
#include "Array.h"

ARRAY_DECL(uint8_t, Byte);

typedef struct Emitter {
    Lexer Lexer;
    ByteArray Code;
} Emitter;

bool Emitter_Create(Emitter* emitter, Lexer lexer);
void Emitter_Destroy(Emitter* emitter);
bool Emitter_Emit(Emitter* emitter);
