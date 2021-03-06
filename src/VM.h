#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum Op {
    Op_Invalid,

    // Exits the virtual machine
    // Arguments:
    //      Inst: op
    //      Stack:
    // Result:
    //      Stack:
    Op_Exit,

    // Pushes data onto the stack
    // Arguments:
    //      Inst: op size data
    //      Stack:
    // Result:
    //      Stack: data
    Op_Push,

    // Allocates zero-initialized space on top of the stack
    // Arguments:
    //      Inst: op size
    //      Stack:
    // Result:
    //      Stack: data
    Op_AllocStack,

    // Pops data from the stack
    // Arguments:
    //      Inst: op size
    //      Stack: data
    // Result:
    //      Stack:
    Op_Pop,

    // Duplicates data on the stack
    // Arguments:
    //      Inst: op size
    //      Stack: data
    // Result:
    //      Stack: data data
    Op_Dup,

    // Adds the 2 numbers on the top of the stack
    // Arguments:
    //      Inst: op size
    //      Stack: a b
    // Result:
    //      Stack: (a+b)
    Op_Add,

    // Subtracts the 2 numbers on the top of the stack
    // Arguments:
    //      Inst: op size
    //      Stack: a b
    // Result:
    //      Stack: (a-b)
    Op_Sub,

    // Prints the data on the top of the stack
    // Arguments:
    //      Inst: op size
    //      Stack: data
    // Result:
    //      Stack:
    Op_Print,

    // Moves the instruction pointer to the location specified
    // Arguments:
    //      Inst: op loc
    //      Stack:
    // Result:
    //      Stack:
    Op_Jump,

    // Moves the instruction pointer to the location specified on the stack
    // Arguments:
    //      Inst: op
    //      Stack: loc
    // Result:
    //      Stack:
    Op_JumpDyn,

    // Moves the instruction pointer to the location specified
    // Arguments:
    //      Inst: op size loc
    //      Stack: data
    // Result:
    //      Stack:
    Op_JumpZero,

    // Jumps to the location if the data is 0
    // Arguments:
    //      Inst: op size loc
    //      Stack: data
    // Result:
    //      Stack:
    Op_JumpNonZero,

    // Puts a pointer to the top of the stack on the stack
    // Arguments:
    //      Inst: op
    //      Stack:
    // Result:
    //      Stack: ptr
    Op_GetStackTop,

    // Puts a pointer to the bottom of the stack on the stack
    // Arguments:
    //      Inst: op
    //      Stack:
    // Result:
    //      Stack: ptr
    Op_GetStackBottom,

    // Loads from a pointer
    // Arguments:
    //      Inst: op size
    //      Stack: ptr
    // Result:
    //      Stack: data
    Op_Load,

    // Stores data into a pointer
    // Arguments:
    //      Inst: op size
    //      Stack: ptr data
    // Result:
    //      Stack:
    Op_Store,

    // Calls a function
    // Arguments:
    //      Inst: op arg-size
    //      Stack: ptr arg-data
    // Result:
    //      Stack: ret-loc arg-data
    Op_Call,

    // Returns from a function
    // Arguments:
    //      Inst: op ret-size
    //      Stack: ret-loc ret-data
    // Result:
    //      Stack: ret-data
    Op_Ret,

    // Calls a C function
    // Arguments:
    //      Inst: op arg-count arg-sizes ret-size
    //      Stack: ptr arg-data
    // Result:
    //      Stack: ret-value
    Op_CallCFunc,
} Op;

typedef struct VM {
    uint8_t* Code;
    uint64_t CodeSize;
    uint8_t* Ip;
    uint8_t Stack[4 * 1024 * 1024];
    uint64_t StackSize;
    uint8_t* Sp;
} VM;

void VM_Init(VM* vm, uint8_t* code, uint64_t codeSize);
void VM_PrintStack(VM* vm);
bool VM_Run(VM* vm);
