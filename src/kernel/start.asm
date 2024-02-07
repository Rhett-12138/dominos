[bits 32]

extern kernel_init
extern console_init
extern gdt_init
extern _ZN6memory11memory_initEjj

global _start
_start:
    
    push ebx ; ards_count
    push eax ; magic
    call console_init
    call gdt_init
    call _ZN6memory11memory_initEjj
    call kernel_init
    jmp $