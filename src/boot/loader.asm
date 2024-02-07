[org 0x1000]

dw 0x55aa; 魔数用于判断错误

mov si, loading
call print

detect_memory:
    xor ebx, ebx

    mov ax, 0
    mov es, ax
    mov edi, ards_buffer

    mov edx, 0x534d4150 ; 固定签名

.next:
    ; 子功能号
    mov eax, 0xe820
    ; ards 结构的大小（字节）
    mov ecx, 20
    ; 调用0x15系统调用
    int 0x15

    ; 如果CF置位，表示出错
    jc error

    ; 将缓存指针指向下一结构体
    add di, cx

    ; 将结构体数量加1
    inc dword [ards_count]

    cmp ebx, 0
    jnz .next

mov si, detecting
call print

jmp prepare_protected_mode

print:
    mov ah, 0x0e
.next:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10
    inc si
    jmp .next
.done:
    ret

loading:
    db "Loading System...", 10, 13, 0
detecting:
    db "Detect memory success.", 10, 13, 0


error:
    mov si, .msg
    call print
    hlt ; CPU stop
    jmp $
    .msg db "Loading Error!!!", 10, 13, 0

prepare_protected_mode:
    cli

    ; 打开A20线
    in al, 0x92
    or al,0b10
    out 0x92, al

    ; 加载GDT
    lgdt [gdt_ptr]

    ; 启动保护模式
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 用跳转来刷新缓存, 启用保护模式
    jmp dword code_selector:protect_mode

[bits 32]
protect_mode:
    mov ax, data_selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, 0x10000 ; 修改栈顶

    mov edi, 0x10000 ; 读取的目标内存
    mov ecx, 10 ; 起始扇区
    mov bl, 200; 扇区数量
    call read_disk

    ; 为了兼容 grub
    mov eax, 0x20220205 ; 内核魔数
    mov ebx, ards_count ; ards 数量指针 

    jmp dword code_selector:0x10000

    ud2; 表示出错
jmp $

read_disk:

    ; 设置读写扇区数量
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    inc dx ; 0x1f3
    mov al, cl ; 起始扇区低8位
    out dx, al

    inc dx ; 0x1f3
    shr ecx, 8
    mov al, cl ; 起始扇区中8位
    out dx, al

    inc dx ; 0x1f3
    shr ecx, 8
    mov al, cl ; 起始扇区高8位
    out dx, al

    inc dx
    shr ecx, 8
    and cl, 0b1111

    mov al, 0b1110_0000
    or al, cl
    out dx, al ; 主盘 LBA模式
    
    inc dx
    mov al, 0x20
    out dx, al

    xor ecx, ecx
    mov cl, bl

    .read:
        push cx
        call .waits
        call .reads
        pop cx
        loop .read
    
    ret

    .waits:
        mov dx, 0x1f7
        .check:
            in al, dx
            jmp $+2; nop
            jmp $+2
            jmp $+2
            and al, 0b1000_1000
            cmp al, 0b0000_1000
            jnz .check
        ret
    
    .reads:
        mov dx, 0x1f0
        mov cx, 256; 一个扇区256字
        .readw:
            in ax, dx
            jmp $+2; nop
            jmp $+2
            jmp $+2
            mov [edi], ax
            add edi, 2
            loop .readw
        ret


code_selector equ (1 << 3)
data_selector equ (2 << 3)

; 内存界限 4G/4K-1
memory_base equ 0
memory_limit equ ((1024*1024*1024*4)/(1024*4))-1

gdt_ptr:
    dw (gdt_end - gdt_base)-1
    dd gdt_base

gdt_base:
    dd 0, 0; NULL描述符
gdt_code:
    dw memory_limit & 0xffff ; 段界限 0-15位
    dw memory_base & 0xffff ; 基地址
    db (memory_base >>16) & 0xff
    db 0b_1_00_1_1_0_1_0
    db 0b1_1_0_0_0000 | (memory_limit >> 16)&0xf
    db (memory_base >> 24) & 0xff

gdt_data:
    dw memory_limit & 0xffff ; 段界限 0-15位
    dw memory_base & 0xffff ; 基地址
    db (memory_base >>16) & 0xff
    db 0b_1_00_1_0_0_1_0
    ; 4K - 32bit - not 64bit - 段界限 16-19
    db 0b1_1_0_0_0000 | (memory_limit >> 16)&0xf
    db (memory_base >> 24) & 0xff
gdt_end:

ards_count:
    dd 0
ards_buffer:
