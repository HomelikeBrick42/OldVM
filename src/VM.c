#include "VM.h"

#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32)
    #include <Windows.h>
#endif

#define ENCODE(ptr, type, value) \
    do {                         \
        *(type*)(ptr) = (value); \
        (ptr) += sizeof(type);   \
    } while (0)

#define DECODE(ptr, type) (((ptr) += sizeof(type)), *((type*)(ptr)-1))

#define PUSH_STACK(ptr, type, value) ENCODE(ptr, type, value)

#define POP_STACK(ptr, type) (((ptr) -= sizeof(type)), *(type*)(ptr))

void VM_Init(VM* vm, uint8_t* code, uint64_t codeSize) {
    memset(vm, 0, sizeof(VM));
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
                            printf("%x ", POP_STACK(vm->Sp, uint8_t));
                        }
                        printf("\n");
                    } break;
                }
            } break;

            case Op_Jump: {
                uint64_t location = DECODE(vm->Ip, uint64_t);
                vm->Ip            = &vm->Code[location];
            } break;

            case Op_JumpZero: {
                uint64_t size     = DECODE(vm->Ip, uint64_t);
                uint64_t location = DECODE(vm->Ip, uint64_t);
                bool zero         = true;
                for (uint64_t i = 0; i < size; i++) {
                    if (*--vm->Sp != 0) {
                        zero = false;
                    }
                }
                if (zero) {
                    vm->Ip = &vm->Code[location];
                }
            } break;

            case Op_JumpNonZero: {
                uint64_t size     = DECODE(vm->Ip, uint64_t);
                uint64_t location = DECODE(vm->Ip, uint64_t);
                bool zero         = true;
                for (uint64_t i = 0; i < size; i++) {
                    if (*--vm->Sp != 0) {
                        zero = false;
                    }
                }
                if (!zero) {
                    vm->Ip = &vm->Code[location];
                }
            } break;

            case Op_GetStackTop: {
                void* ptr = vm->Sp;
                PUSH_STACK(vm->Sp, void*, ptr);
            } break;

            case Op_Load: {
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
                            *ip++ = 0b01001000;
                            *ip++ = 0b10111001;
                            ENCODE(ip, uint64_t, value);
                        } break;

                        case 1: {
                            // REX.W mov rdx, imm64
                            *ip++ = 0b01001000;
                            *ip++ = 0b10111010;
                            ENCODE(ip, uint64_t, value);
                        } break;

                        case 2: {
                            // REX.WB mov r8, imm64
                            *ip++ = 0b01001001;
                            *ip++ = 0b10111000;
                            ENCODE(ip, uint64_t, value);
                        } break;

                        case 3: {
                            // REX.WB mov r9, imm64
                            *ip++ = 0b01001001;
                            *ip++ = 0b10111001;
                            ENCODE(ip, uint64_t, value);
                        } break;

                        default: {
                            // REX.W mov rax, imm64
                            *ip++ = 0b01001000;
                            *ip++ = 0b10111000;
                            ENCODE(ip, uint64_t, value);

                            // push rax
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
#else
    #error "Unsupported platform"
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
