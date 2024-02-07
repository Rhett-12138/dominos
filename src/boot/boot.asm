[org 0x7c00]

; 设置屏幕模式为文本模式，清除屏幕
mov ax,3
int 0x10

; 初始化段寄存器
mov ax, 0
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7c00

mov si, booting
call print

mov edi, 0x1000 ; 读取的目标内存
mov ecx, 2 ; 起始扇区
mov bl, 4; 扇区数量
call read_disk
mov si, readOK
call print

cmp word [0x1000], 0x55aa
jnz error

jmp 0:0x1002

; 阻塞
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

booting:
    db "Booting...", 10, 13, 0; \n\r'\0'

readOK:
    db "Read complete.", 10, 13, 0

error:
    mov si, .msg
    call print
    hlt ; CPU stop
    jmp $
    .msg db "Booting Error!!!", 10, 13, 0

; 填充 0
times 510 - ($ - $$) db 0

; 主引导扇区(512B)的最后两个字节必须是这样
; dw 0xaa55
db 0x55, 0xaa
