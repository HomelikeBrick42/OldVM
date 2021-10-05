#include "Lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#if defined(_WIN32)
    #include <Windows.h>
#endif

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
               (lexer->Current >= '0' && lexer->Current <= '9') || lexer->Current == '_') {
            Lexer_NextChar(lexer);
            name.Length++;
        }
        return (Token){
            .Kind        = TokenKind_Name,
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
