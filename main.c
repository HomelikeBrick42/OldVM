#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32)
    #include <Windows.h>
#endif

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
    //      Inst: op
    //      Stack: loc
    // Result:
    //      Stack:
    Op_Jump,

    // Puts a pointer to the top of the stack on the stack
    // Arguments:
    //      Inst: op
    //      Stack:
    // Result:
    //      Stack: ptr
    Op_GetStackTop,

    // Reads from a pointer
    // Arguments:
    //      Inst: op size
    //      Stack: ptr
    // Result:
    //      Stack: data
    Op_Read,

    // Stores data into a pointer
    // Arguments:
    //      Inst: op size
    //      Stack: ptr data
    // Result:
    //      Stack:
    Op_Store,

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
    uint8_t Stack[1024];
    uint64_t StackSize;
    uint8_t* Sp;
} VM;

#define ENCODE(ptr, type, value) \
    do {                         \
        *(type*)(ptr) = (value); \
        (ptr) += sizeof(type);   \
    } while (0)

#define DECODE(ptr, type) (((ptr) += sizeof(type)), *((type*)(ptr)-1))

#define PUSH_STACK(ptr, type, value) ENCODE(ptr, type, value)

#define POP_STACK(ptr, type) (((ptr) -= sizeof(type)), *(type*)(ptr))

void VM_Init(VM* vm, uint8_t* code, uint64_t codeSize) {
    *vm           = (VM){};
    vm->Code      = code;
    vm->CodeSize  = codeSize;
    vm->Ip        = vm->Code;
    vm->StackSize = sizeof(vm->Stack) / sizeof(vm->Stack[0]);
    vm->Sp        = vm->Stack;
}

void VM_PrintStack(VM* vm) {
    uint8_t* sp = vm->Sp;
    if (sp <= vm->Stack) {
        printf("The stack is empty\n");
    } else {
        printf("Stack:\n");
        while (sp > vm->Stack) {
            sp--;
            printf("0x%04llx | 0x%02x\n", sp - vm->Stack, *sp);
        }
    }
}

bool VM_Run(VM* vm) {
    while (true) {
        if (vm->Ip - vm->Code < 0 || vm->Ip - vm->Code >= (int64_t)vm->CodeSize) {
            fflush(stdout);
            fprintf(stderr, "Instruction pointer out of range\n");
            return false;
        }

        if (vm->Sp - vm->Stack < 0 || vm->Sp - vm->Stack >= (int64_t)vm->StackSize) {
            fflush(stdout);
            fprintf(stderr, "Stack pointer out of range\n");
            return false;
        }

        switch (*vm->Ip++) {
            case Op_Exit: {
                return true;
            } break;

            case Op_Push: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                for (uint64_t i = 0; i < size; i++) {
                    *vm->Sp++ = *vm->Ip++;
                }
            } break;

            case Op_AllocStack: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                for (uint64_t i = 0; i < size; i++) {
                    *vm->Sp++ = 0;
                }
            } break;

            case Op_Pop: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                vm->Sp -= size;
            } break;

            case Op_Dup: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                uint8_t* ptr  = vm->Sp - size;
                for (uint64_t i = 0; i < size; i++) {
                    *vm->Sp++ = *ptr++;
                }
            } break;

            case Op_Add: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                switch (size) {
                    case 1: {
                        uint8_t b = POP_STACK(vm->Sp, uint8_t);
                        uint8_t a = POP_STACK(vm->Sp, uint8_t);
                        PUSH_STACK(vm->Sp, uint8_t, a + b);
                    } break;

                    case 2: {
                        uint16_t b = POP_STACK(vm->Sp, uint16_t);
                        uint16_t a = POP_STACK(vm->Sp, uint16_t);
                        PUSH_STACK(vm->Sp, uint16_t, a + b);
                    } break;

                    case 4: {
                        uint32_t b = POP_STACK(vm->Sp, uint32_t);
                        uint32_t a = POP_STACK(vm->Sp, uint32_t);
                        PUSH_STACK(vm->Sp, uint32_t, a + b);
                    } break;

                    case 8: {
                        uint64_t b = POP_STACK(vm->Sp, uint64_t);
                        uint64_t a = POP_STACK(vm->Sp, uint64_t);
                        PUSH_STACK(vm->Sp, uint64_t, a + b);
                    } break;

                    default: {
                        fflush(stdout);
                        fprintf(stderr, "Unsupported add size %llu\n", size);
                        return false;
                    } break;
                }
            } break;

            case Op_Sub: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                switch (size) {
                    case 1: {
                        uint8_t b = POP_STACK(vm->Sp, uint8_t);
                        uint8_t a = POP_STACK(vm->Sp, uint8_t);
                        PUSH_STACK(vm->Sp, uint8_t, a - b);
                    } break;

                    case 2: {
                        uint16_t b = POP_STACK(vm->Sp, uint16_t);
                        uint16_t a = POP_STACK(vm->Sp, uint16_t);
                        PUSH_STACK(vm->Sp, uint16_t, a - b);
                    } break;

                    case 4: {
                        uint32_t b = POP_STACK(vm->Sp, uint32_t);
                        uint32_t a = POP_STACK(vm->Sp, uint32_t);
                        PUSH_STACK(vm->Sp, uint32_t, a - b);
                    } break;

                    case 8: {
                        uint64_t b = POP_STACK(vm->Sp, uint64_t);
                        uint64_t a = POP_STACK(vm->Sp, uint64_t);
                        PUSH_STACK(vm->Sp, uint64_t, a - b);
                    } break;

                    default: {
                        fflush(stdout);
                        fprintf(stderr, "Unsupported subtract size %llu\n", size);
                        return false;
                    } break;
                }
            } break;

            case Op_Print: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                switch (size) {
                    case 1: {
                        printf("%u\n", POP_STACK(vm->Sp, uint8_t));
                    } break;

                    case 2: {
                        printf("%u\n", POP_STACK(vm->Sp, uint16_t));
                    } break;

                    case 4: {
                        printf("%u\n", POP_STACK(vm->Sp, uint32_t));
                    } break;

                    case 8: {
                        printf("%llu\n", POP_STACK(vm->Sp, uint64_t));
                    } break;

                    default: {
                        for (uint64_t i = 0; i < size; i++) {
                            printf("%u ", POP_STACK(vm->Sp, uint8_t));
                        }
                        printf("\n");
                    } break;
                }
            } break;

            case Op_Jump: {
                uint64_t location = POP_STACK(vm->Sp, uint64_t);
                vm->Ip            = &vm->Code[location];
            } break;

            case Op_GetStackTop: {
                void* ptr = vm->Sp;
                PUSH_STACK(vm->Sp, void*, ptr);
            } break;

            case Op_Read: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                uint8_t* ptr  = POP_STACK(vm->Sp, uint8_t*);
                for (uint64_t i = 0; i < size; i++) {
                    *vm->Sp++ = *ptr++;
                }
            } break;

            case Op_Store: {
                uint64_t size = DECODE(vm->Ip, uint64_t);
                uint8_t data[size];
                vm->Sp -= size;
                for (uint64_t i = 0; i < size; i++) {
                    data[i] = vm->Sp[i];
                }
                uint8_t* ptr = POP_STACK(vm->Sp, uint8_t*);
                for (uint64_t i = 0; i < size; i++) {
                    *ptr++ = data[i];
                }
            } break;

            case Op_CallCFunc: {
                uint64_t argCount = DECODE(vm->Ip, uint64_t);
                uint64_t argSizes[argCount];
                for (uint64_t i = 0; i < argCount; i++) {
                    argSizes[i] = DECODE(vm->Ip, uint64_t);
                    if (argSizes[i] > 8) {
                        fflush(stdout);
                        fprintf(stderr, "Cannot call C function with argument size greater than 8\n");
                        return false;
                    }
                }

                uint64_t retSize = DECODE(vm->Ip, uint64_t);
                if (retSize > 8) {
                    fflush(stdout);
                    fprintf(stderr, "Cannot call C function with return size greater than 8\n");
                    return false;
                }

#if defined(_WIN32)
                uint64_t (*func)(void) = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if (!func) {
                    fflush(stdout);
                    fprintf(stderr, "Failed to allocate executable memory for calling C function\n");
                    return false;
                }

                uint8_t* ip = (uint8_t*)func;

                for (int64_t i = (int64_t)argCount - 1; i >= 0; i--) {
                    vm->Sp -= argSizes[i];
                    uint64_t value = 0;
                    for (uint64_t j = 0; j < argSizes[i]; j++) {
                        ((uint8_t*)&value)[j] = vm->Sp[j];
                    }

                    switch (i) {
                        case 0: {
                            // REX.W mov rcx, imm64
                            *ip++          = 0b01001000;
                            *ip++          = 0b10111001;
                            *(uint64_t*)ip = value;
                            ip += sizeof(uint64_t);
                        } break;

                        case 1: {
                            // REX.W mov rdx, imm64
                            *ip++          = 0b01001000;
                            *ip++          = 0b10111010;
                            *(uint64_t*)ip = value;
                            ip += sizeof(uint64_t);
                        } break;

                        case 2: {
                            // REX.WB mov r8, imm64
                            *ip++          = 0b01001001;
                            *ip++          = 0b10111000;
                            *(uint64_t*)ip = value;
                            ip += sizeof(uint64_t);
                        } break;

                        case 3: {
                            // REX.WB mov r9, imm64
                            *ip++          = 0b01001001;
                            *ip++          = 0b10111001;
                            *(uint64_t*)ip = value;
                            ip += sizeof(uint64_t);
                        } break;

                        default: {
                            // REX.W mov rax, imm64
                            *ip++          = 0b01001000;
                            *ip++          = 0b10111000;
                            *(uint64_t*)ip = value;
                            ip += sizeof(uint64_t);

                            // Push rax
                            *ip++ = 0x50;
                        } break;
                    }
                }

                void* ptr = POP_STACK(vm->Sp, void*);

                // REX.W mov rax, imm64
                *ip++       = 0b01001000;
                *ip++       = 0b10111000;
                *(void**)ip = ptr;
                ip += sizeof(void*);

                // call rax
                *ip++ = 0xFF;
                *ip++ = 0xD0;

                // ret
                *ip++ = 0xC3;

                uint64_t result = func();
                VirtualFree(func, 0, MEM_RELEASE);
#endif
                for (uint64_t i = 0; i < retSize; i++) {
                    *vm->Sp++ = ((uint8_t*)&result)[i];
                }
            } break;

            default: {
                fflush(stdout);
                fprintf(stderr, "Invalid instruction\n");
                return false;
            } break;
        }
    }
}

uint64_t Func(uint8_t a, uint16_t b, uint64_t* c) {
    *c = a + (uint64_t)b;
    return *c + a;
}

int main() {
    uint8_t code[1024] = {};
    uint64_t codeSize  = 0;

    uint64_t a = 0;
    uint64_t b = 0;

    {
        uint8_t* ip = code;

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint64_t*));
        ENCODE(ip, uint64_t*, &b);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(void*));
        ENCODE(ip, void*, Func);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint64_t));
        ENCODE(ip, uint64_t, 5);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint16_t));
        ENCODE(ip, uint16_t, 6);

        *ip++ = Op_Push;
        ENCODE(ip, uint64_t, sizeof(uint64_t*));
        ENCODE(ip, uint64_t*, &a);

        *ip++ = Op_CallCFunc;
        ENCODE(ip, uint64_t, 3);
        // Argument sizes
        ENCODE(ip, uint64_t, sizeof(uint64_t));
        ENCODE(ip, uint64_t, sizeof(uint16_t));
        ENCODE(ip, uint64_t, sizeof(uint64_t*));
        // Return size
        ENCODE(ip, uint64_t, sizeof(uint64_t));

        *ip++ = Op_Store;
        ENCODE(ip, uint64_t, sizeof(uint64_t));

        *ip++ = Op_Exit;

        codeSize = ip - code;
    }

    VM vm;
    VM_Init(&vm, code, codeSize);
    if (!VM_Run(&vm)) {
        VM_PrintStack(&vm);
        return EXIT_FAILURE;
    }

    printf("a=%llu, b=%llu\n", a, b);

    return EXIT_SUCCESS;
}
