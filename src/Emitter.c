#include "Emitter.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

ARRAY_IMPL(uint8_t, Byte);
ARRAY_IMPL(Label, Label);
ARRAY_IMPL(UnknownLabel, UnknownLabel);

bool Emitter_Create(Emitter* emitter, Lexer lexer) {
    *emitter               = (Emitter){};
    emitter->Lexer         = lexer;
    emitter->Code          = ByteArray_Create();
    emitter->Current       = Lexer_NextToken(&emitter->Lexer);
    emitter->Labels        = LabelArray_Create();
    emitter->UnknownLabels = UnknownLabelArray_Create();
    return true;
}

void Emitter_Destroy(Emitter* emitter) {
    ByteArray_Destroy(&emitter->Code);
    LabelArray_Destroy(&emitter->Labels);
    UnknownLabelArray_Destroy(&emitter->UnknownLabels);
    Lexer_Destroy(&emitter->Lexer);
}

void Emitter_Emit(Emitter* emitter) {
    while (true) {
        switch (emitter->Current.Kind) {
            case TokenKind_EndOfFile: {
                while (emitter->UnknownLabels.Length > 0) {
                    UnknownLabel unknown = UnknownLabelArray_Pop(&emitter->UnknownLabels);
                    if (!unknown.Resolved) {
                        emitter->WasError = true;
                        fflush(stdout);
                        fprintf(stderr,
                                "%.*s:%llu:%llu: Unknown label '%.*s'\n",
                                String_Fmt(unknown.Token.FilePath),
                                unknown.Token.Line,
                                unknown.Token.Column,
                                String_Fmt(unknown.Token.StringValue));
                    }
                }
                return;
            } break;

            case TokenKind_Colon: {
                Emitter_NextToken(emitter);
                Token name        = Emitter_ExpectToken(emitter, TokenKind_Name);
                uint64_t location = emitter->Code.Length;
                // TODO: Duplicate labels
                LabelArray_Push(&emitter->Labels,
                                (Label){
                                    .Token    = name,
                                    .Location = location,
                                });
                for (uint64_t i = 0; i < emitter->UnknownLabels.Length; i++) {
                    if (!emitter->UnknownLabels.Data[i].Resolved &&
                        String_Equal(emitter->UnknownLabels.Data[i].Token.StringValue, name.StringValue)) {
                        UnknownLabel* unknown                                     = &emitter->UnknownLabels.Data[i];
                        unknown->Resolved                                         = true;
                        *(uint64_t*)&emitter->Code.Data[unknown->IndexForAddress] = location;
                    }
                }
            } break;

            case TokenKind_Exit: {
                Emitter_NextToken(emitter);
                Emitter_EmitOp(emitter, Op_Exit);
            } break;

            case TokenKind_Push: {
                Emitter_NextToken(emitter);
                uint64_t size = Emitter_ExpectToken(emitter, TokenKind_Integer).IntValue;
                // TODO: Support strings or maybe lists of numbers?
                uint64_t value = Emitter_ExpectToken(emitter, TokenKind_Integer).IntValue;
                Emitter_EmitOp(emitter, Op_Push);
                Emitter_Emit64(emitter, size);
                Emitter_EmitBytes(emitter, (uint8_t*)&value, size);
            } break;

            case TokenKind_Add: {
                Emitter_NextToken(emitter);
                uint64_t size = Emitter_ExpectToken(emitter, TokenKind_Integer).IntValue;
                Emitter_EmitOp(emitter, Op_Add);
                Emitter_Emit64(emitter, size);
            } break;

            case TokenKind_Print: {
                Emitter_NextToken(emitter);
                uint64_t size = Emitter_ExpectToken(emitter, TokenKind_Integer).IntValue;
                Emitter_EmitOp(emitter, Op_Print);
                Emitter_Emit64(emitter, size);
            } break;

            case TokenKind_Dup: {
                Emitter_NextToken(emitter);
                uint64_t size = Emitter_ExpectToken(emitter, TokenKind_Integer).IntValue;
                Emitter_EmitOp(emitter, Op_Dup);
                Emitter_Emit64(emitter, size);
            } break;

            case TokenKind_Jump: {
                Emitter_NextToken(emitter);
                Token name        = Emitter_ExpectToken(emitter, TokenKind_Name);
                bool found        = false;
                uint64_t location = 0;
                for (uint64_t i = 0; i < emitter->Labels.Length; i++) {
                    if (String_Equal(emitter->Labels.Data[i].Token.StringValue, name.StringValue)) {
                        found    = true;
                        location = emitter->Labels.Data[i].Location;
                        break;
                    }
                }
                Emitter_EmitOp(emitter, Op_Jump);
                if (!found) {
                    UnknownLabelArray_Push(&emitter->UnknownLabels,
                                           (UnknownLabel){
                                               .Token           = name,
                                               .IndexForAddress = emitter->Code.Length,
                                               .Resolved        = false,
                                           });
                }
                Emitter_Emit64(emitter, location);
            } break;

            default: {
                fflush(stdout);
                String name = GetTokenKindName(emitter->Current.Kind);
                fprintf(stderr,
                        "%.*s:%llu:%llu: Unexpected token '%.*s'\n",
                        String_Fmt(emitter->Current.FilePath),
                        emitter->Current.Line,
                        emitter->Current.Column,
                        String_Fmt(name));
                emitter->WasError = true;
                Emitter_NextToken(emitter);
            } break;
        }
    }
}

Token Emitter_NextToken(Emitter* emitter) {
    Token current    = emitter->Current;
    emitter->Current = Lexer_NextToken(&emitter->Lexer);
    return current;
}

Token Emitter_ExpectToken(Emitter* emitter, TokenKind kind) {
    if (emitter->Current.Kind == kind) {
        return Emitter_NextToken(emitter);
    }

    emitter->WasError = true;

    String expected = GetTokenKindName(kind);
    String got      = GetTokenKindName(emitter->Current.Kind);
    fprintf(stderr,
            "%.*s:%llu:%llu: Expected token '%.*s', got token '%.*s'\n",
            String_Fmt(emitter->Current.FilePath),
            emitter->Current.Line,
            emitter->Current.Column,
            String_Fmt(expected),
            String_Fmt(got));

    return (Token){
        .Kind     = kind,
        .FilePath = emitter->Current.FilePath,
        .Source   = emitter->Current.Source,
        .Position = emitter->Current.Position,
        .Line     = emitter->Current.Line,
        .Column   = emitter->Current.Column,
        .Length   = emitter->Current.Length,
    };
}

void Emitter_EmitOp(Emitter* emitter, Op op) {
    ByteArray_Push(&emitter->Code, op);
}

void Emitter_Emit64(Emitter* emitter, uint64_t value) {
    Emitter_EmitBytes(emitter, (uint8_t*)&value, sizeof(uint64_t));
}

void Emitter_EmitBytes(Emitter* emitter, uint8_t* bytes, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        ByteArray_Push(&emitter->Code, bytes[i]);
    }
}
