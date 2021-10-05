#pragma once

#include "Strings.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum TokenKind {
    TokenKind_Invalid,
    TokenKind_EndOfFile,
    TokenKind_Colon,
    TokenKind_Bang,
    TokenKind_OpenParenthesis,
    TokenKind_CloseParenthesis,
    TokenKind_Integer,
    TokenKind_Name,
    TokenKind_Macro,
    TokenKind_Exit,
    TokenKind_Push,
    TokenKind_Pop,
    TokenKind_AllocStack,
    TokenKind_Add,
    TokenKind_Sub,
    TokenKind_Print,
    TokenKind_Dup,
    TokenKind_Jump,
    TokenKind_JumpZero,
    TokenKind_JumpNonZero,
    TokenKind_GetStackTop,
    TokenKind_GetStackBottom,
    TokenKind_Load,
    TokenKind_Store,
    TokenKind_CallCFunc,
} TokenKind;

String GetTokenKindName(TokenKind kind);

typedef struct Token {
    TokenKind Kind;
    String FilePath;
    String Source;
    uint64_t Position;
    uint64_t Line;
    uint64_t Column;
    uint64_t Length;
    union {
        String StringValue;
        uint64_t IntValue;
    };
} Token;

typedef struct Lexer {
    String FilePath;
    String Source;
    uint64_t Position;
    uint64_t Line;
    uint64_t Column;
    uint8_t Current;
} Lexer;

bool Lexer_Create(Lexer* lexer, String filepath);
void Lexer_Destroy(Lexer* lexer);
Token Lexer_NextToken(Lexer* lexer);
uint8_t Lexer_NextChar(Lexer* lexer);
