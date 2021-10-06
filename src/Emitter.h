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
} UnknownLabel;

ARRAY_DECL(Token, Token);

typedef struct Macro {
    Token Name;
    TokenArray Tokens;
} Macro;

ARRAY_DECL(uint8_t, Byte);
ARRAY_DECL(Label, Label);
ARRAY_DECL(UnknownLabel, UnknownLabel);
ARRAY_DECL(Macro, Macro);

typedef struct Emitter {
    Lexer Lexer;
    ByteArray Code;
    Token Current;
    TokenArray NextTokens;
    LabelArray Labels;
    UnknownLabelArray UnknownLabels;
    MacroArray Macros;
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
