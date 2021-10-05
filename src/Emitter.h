#pragma once

#include "Lexer.h"
#include "Array.h"
#include "VM.h"

typedef struct Label {
    Token Token;
    uint64_t Location;
} Label;

typedef struct UnknownLabel {
    Token Token;
    uint64_t IndexForAddress;
    bool Resolved;
} UnknownLabel;

ARRAY_DECL(uint8_t, Byte);
ARRAY_DECL(Label, Label);
ARRAY_DECL(UnknownLabel, UnknownLabel);

typedef struct Emitter {
    Lexer Lexer;
    ByteArray Code;
    Token Current;
    LabelArray Labels;
    UnknownLabelArray UnknownLabels;
    bool WasError;
} Emitter;

bool Emitter_Create(Emitter* emitter, Lexer lexer);
void Emitter_Destroy(Emitter* emitter);
void Emitter_Emit(Emitter* emitter);
Token Emitter_NextToken(Emitter* emitter);
Token Emitter_ExpectToken(Emitter* emitter, TokenKind kind);
void Emitter_EmitOp(Emitter* emitter, Op op);
void Emitter_Emit64(Emitter* emitter, uint64_t value);
void Emitter_EmitBytes(Emitter* emitter, uint8_t* bytes, uint64_t count);
