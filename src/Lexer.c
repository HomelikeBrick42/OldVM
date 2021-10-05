#include "Lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#if defined(_WIN32)
    #include <Windows.h>
#endif

String GetTokenKindName(TokenKind kind) {
    switch (kind) {
        case TokenKind_Invalid:
            return String_FromLiteral("Invalid");
        case TokenKind_EndOfFile:
            return String_FromLiteral("EndOfFile");
        case TokenKind_Colon:
            return String_FromLiteral(":");
        case TokenKind_Integer:
            return String_FromLiteral("Integer");
        case TokenKind_Name:
            return String_FromLiteral("Name");
        case TokenKind_Exit:
            return String_FromLiteral("exit");
        case TokenKind_Push:
            return String_FromLiteral("push");
        case TokenKind_Pop:
            return String_FromLiteral("pop");
        case TokenKind_AllocStack:
            return String_FromLiteral("alloc-stack");
        case TokenKind_Add:
            return String_FromLiteral("add");
        case TokenKind_Sub:
            return String_FromLiteral("sub");
        case TokenKind_Print:
            return String_FromLiteral("print");
        case TokenKind_Dup:
            return String_FromLiteral("dup");
        case TokenKind_Jump:
            return String_FromLiteral("jump");
        case TokenKind_JumpZero:
            return String_FromLiteral("jump-zero");
        case TokenKind_JumpNonZero:
            return String_FromLiteral("jump-non-zero");
        case TokenKind_GetTopStack:
            return String_FromLiteral("get-top-stack");
        case TokenKind_Load:
            return String_FromLiteral("load");
        case TokenKind_Store:
            return String_FromLiteral("store");
        case TokenKind_CallCFunc:
            return String_FromLiteral("call-c-func");
    }
}

struct {
    String Name;
    TokenKind Kind;
} Keywords[] = {
    {
        .Name = String_FromLiteral("exit"),
        .Kind = TokenKind_Exit,
    },
    {
        .Name = String_FromLiteral("push"),
        .Kind = TokenKind_Push,
    },
    {
        .Name = String_FromLiteral("pop"),
        .Kind = TokenKind_Pop,
    },
    {
        .Name = String_FromLiteral("alloc-stack"),
        .Kind = TokenKind_AllocStack,
    },
    {
        .Name = String_FromLiteral("add"),
        .Kind = TokenKind_Add,
    },
    {
        .Name = String_FromLiteral("sub"),
        .Kind = TokenKind_Sub,
    },
    {
        .Name = String_FromLiteral("print"),
        .Kind = TokenKind_Print,
    },
    {
        .Name = String_FromLiteral("dup"),
        .Kind = TokenKind_Dup,
    },
    {
        .Name = String_FromLiteral("jump"),
        .Kind = TokenKind_Jump,
    },
    {
        .Name = String_FromLiteral("jump-zero"),
        .Kind = TokenKind_JumpZero,
    },
    {
        .Name = String_FromLiteral("jump-non-zero"),
        .Kind = TokenKind_JumpNonZero,
    },
    {
        .Name = String_FromLiteral("get-top-stack"),
        .Kind = TokenKind_GetTopStack,
    },
    {
        .Name = String_FromLiteral("load"),
        .Kind = TokenKind_Load,
    },
    {
        .Name = String_FromLiteral("store"),
        .Kind = TokenKind_Store,
    },
    {
        .Name = String_FromLiteral("call-c-func"),
        .Kind = TokenKind_CallCFunc,
    },
};

bool Lexer_Create(Lexer* lexer, String filepath) {
    *lexer = (Lexer){};

    char* path = malloc(filepath.Length + 1);
    if (!path) {
        fflush(stdout);
        fprintf(stderr, "Failed to allocate file path\n");
        return false;
    }

    memcpy(path, filepath.Data, filepath.Length);
    path[filepath.Length] = '\0';

#if defined(_WIN32)
    {
        uint64_t length = GetFullPathNameA(path, 0, NULL, NULL);
        if (length == 0) {
            fflush(stdout);
            fprintf(stderr, "Failed to get full path to file\n");
            return false;
        }

        char* buffer = malloc(length);
        if (!buffer) {
            fflush(stdout);
            fprintf(stderr, "Failed to allocate file path\n");
            return false;
        }

        if (GetFullPathNameA(path, length, buffer, NULL) == 0) {
            fflush(stdout);
            fprintf(stderr, "Failed to get full path to file\n");
            return false;
        }

        lexer->FilePath = (String){
            .Data   = (uint8_t*)buffer,
            .Length = length,
        };
    }
#else
    #error "Unsupported platform"
#endif

    FILE* file = fopen(path, "rb");
    if (!file) {
        fflush(stdout);
        fprintf(stderr, "Failed to open file '%.*s'\n", String_Fmt(lexer->FilePath));
        return false;
    }

    fseek(file, 0, SEEK_END);
    lexer->Source.Length = ftell(file);
    fseek(file, 0, SEEK_SET);

    lexer->Source.Data = malloc(lexer->Source.Length);
    if (!lexer->Source.Data) {
        fflush(stdout);
        fprintf(stderr, "Failed allocate buffer for source '%.*s'\n", String_Fmt(lexer->FilePath));
        return false;
    }

    if (fread(lexer->Source.Data, 1, lexer->Source.Length, file) != lexer->Source.Length) {
        fflush(stdout);
        fprintf(stderr, "Failed to read file '%.*s'\n", String_Fmt(lexer->FilePath));
        return false;
    }

    lexer->Position = 0;
    lexer->Line     = 1;
    lexer->Column   = 1;
    lexer->Current  = lexer->Position < lexer->Source.Length ? lexer->Source.Data[lexer->Position] : '\0';

    fclose(file);
    free(path);
    return true;
}

void Lexer_Destroy(Lexer* lexer) {
    free(lexer->FilePath.Data);
    free(lexer->Source.Data);
}

Token Lexer_NextToken(Lexer* lexer) {
Start:
    uint64_t startPosition = lexer->Position;
    uint64_t startLine     = lexer->Line;
    uint64_t startColumn   = lexer->Column;

    if (lexer->Current == '\0') {
        return (Token){
            .Kind     = TokenKind_EndOfFile,
            .FilePath = lexer->FilePath,
            .Source   = lexer->Source,
            .Position = startPosition,
            .Line     = startLine,
            .Column   = startPosition,
            .Length   = 0,
        };
    } else if (lexer->Current >= '0' && lexer->Current <= '9') {
        uint64_t base     = 10;
        uint64_t intValue = 0;
        while ((lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_') {
            if (lexer->Current == '_') {
                Lexer_NextChar(lexer);
                continue;
            }
            intValue *= base;
            intValue += Lexer_NextChar(lexer) - '0';
        }
        return (Token){
            .Kind     = TokenKind_Integer,
            .FilePath = lexer->FilePath,
            .Source   = lexer->Source,
            .Position = startPosition,
            .Line     = startLine,
            .Column   = startPosition,
            .Length   = lexer->Position - startPosition,
            .IntValue = intValue,
        };
    } else if ((lexer->Current >= 'A' && lexer->Current <= 'Z') || (lexer->Current >= 'a' && lexer->Current <= 'z') ||
               lexer->Current == '_') {
        String name = (String){
            .Data   = &lexer->Source.Data[startPosition],
            .Length = 0,
        };
        while ((lexer->Current >= 'A' && lexer->Current <= 'Z') || (lexer->Current >= 'a' && lexer->Current <= 'z') ||
               (lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_' || lexer->Current == '-') {
            Lexer_NextChar(lexer);
            name.Length++;
        }
        TokenKind kind = TokenKind_Name;
        for (uint64_t i = 0; i < sizeof(Keywords) / sizeof(Keywords[0]); i++) {
            if (String_Equal(Keywords[i].Name, name)) {
                kind = Keywords[i].Kind;
                break;
            }
        }
        return (Token){
            .Kind        = kind,
            .FilePath    = lexer->FilePath,
            .Source      = lexer->Source,
            .Position    = startPosition,
            .Line        = startLine,
            .Column      = startPosition,
            .Length      = name.Length,
            .StringValue = name,
        };
    } else {
        switch (lexer->Current) {
            case ' ':
            case '\t':
            case '\n':
            case '\r': {
                Lexer_NextChar(lexer);
                goto Start;
            } break;

            case ':': {
                Lexer_NextChar(lexer);
                return (Token){
                    .Kind     = TokenKind_Colon,
                    .FilePath = lexer->FilePath,
                    .Source   = lexer->Source,
                    .Position = startPosition,
                    .Line     = startLine,
                    .Column   = startPosition,
                    .Length   = 1,
                };
            } break;

            case '/': {
                uint8_t chr = lexer->Current;
                Lexer_NextChar(lexer);
                if (lexer->Current != '/') {
                    fflush(stdout);
                    fprintf(stderr,
                            "%.*s:%llu:%llu: Unexpected character '%c'\n",
                            String_Fmt(lexer->FilePath),
                            startLine,
                            startColumn,
                            chr);
                    goto Start;
                }
                Lexer_NextChar(lexer);
                while (lexer->Current != '\n' && lexer->Current != '\0') {
                    Lexer_NextChar(lexer);
                }
                goto Start;
            } break;

            default: {
                uint8_t chr = lexer->Current;
                Lexer_NextChar(lexer);
                fflush(stdout);
                fprintf(stderr,
                        "%.*s:%llu:%llu: Unexpected character '%c'\n",
                        String_Fmt(lexer->FilePath),
                        startLine,
                        startColumn,
                        chr);
                goto Start;
            } break;
        }
    }
}

uint8_t Lexer_NextChar(Lexer* lexer) {
    uint8_t current = lexer->Current;
    lexer->Position++;
    lexer->Column++;
    if (current == '\n') {
        lexer->Line++;
        lexer->Column = 1;
    }
    lexer->Current = lexer->Position < lexer->Source.Length ? lexer->Source.Data[lexer->Position] : '\0';
    return current;
}
