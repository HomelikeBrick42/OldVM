#pragma once

#include <stdint.h>

#define ARRAY_DECL(type, name)                                   \
    typedef struct name##Array {                                 \
        type* Data;                                              \
        uint64_t Length;                                         \
        uint64_t Capacity;                                       \
    } name##Array;                                               \
                                                                 \
    name##Array name##Array_Create();                            \
    void name##Array_Destroy(name##Array* array);                \
                                                                 \
    void name##Array_Clone(name##Array* out, name##Array array); \
                                                                 \
    type* name##Array_Push(name##Array* array, type value);      \
    type name##Array_Pop(name##Array* array);                    \
    type name##Array_Remove(name##Array* array, uint64_t index)

#define ARRAY_IMPL(type, name)                                                                                       \
    name##Array name##Array_Create() {                                                                               \
        return (name##Array){                                                                                        \
            .Data     = NULL,                                                                                        \
            .Length   = 0,                                                                                           \
            .Capacity = 0,                                                                                           \
        };                                                                                                           \
    }                                                                                                                \
                                                                                                                     \
    void name##Array_Destroy(name##Array* array) {                                                                   \
        free(array->Data);                                                                                           \
                                                                                                                     \
        *array = (name##Array){                                                                                      \
            .Data     = NULL,                                                                                        \
            .Length   = 0,                                                                                           \
            .Capacity = 0,                                                                                           \
        };                                                                                                           \
    }                                                                                                                \
                                                                                                                     \
    void name##Array_Clone(name##Array* out, name##Array array) {                                                    \
        out->Length   = array.Length;                                                                                \
        out->Capacity = array.Length;                                                                                \
        out->Data     = malloc(out->Length * sizeof(type));                                                          \
        memcpy(out->Data, array.Data, out->Length * sizeof(type));                                                   \
    }                                                                                                                \
                                                                                                                     \
    type* name##Array_Push(name##Array* array, type value) {                                                         \
        if (array->Length >= array->Capacity) {                                                                      \
            array->Capacity = array->Capacity == 0 ? 1 : array->Capacity * 2;                                        \
            array->Data     = realloc(array->Data, array->Capacity * sizeof(array->Data[0]));                        \
        }                                                                                                            \
                                                                                                                     \
        array->Data[array->Length] = value;                                                                          \
        array->Length++;                                                                                             \
        return &array->Data[array->Length - 1];                                                                      \
    }                                                                                                                \
                                                                                                                     \
    type name##Array_Pop(name##Array* array) {                                                                       \
        return array->Data[--array->Length];                                                                         \
    }                                                                                                                \
                                                                                                                     \
    type name##Array_Remove(name##Array* array, uint64_t index) {                                                    \
        type value = array->Data[index];                                                                             \
        memmove(&array->Data[index], &array->Data[index + 1], (array->Length - index - 1) * sizeof(array->Data[0])); \
        array->Length--;                                                                                             \
        return value;                                                                                                \
    }
