[bits 16]
org 0x7C00

start:
    cli                     ; 禁用中断
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; 栈顶在引导扇区末尾

    ; 打印欢迎信息
    mov si, welcome_msg
    call puts

    ; 将内核(起始 LBA = 2)加载到 0x0000:0x1000，每次读取多达 20 个扇区
    mov bx, 0x1000          ; ES:BX = 0x0000:0x1000
    mov dh, 20              ; 要读取的扇区数量
    call load_kernel

    ; 跳转到加载好的 32 位 C 入口
    jmp 0x0000:0x1000

; --------------------------------------------------
; 读取若干扇区到 ES:BX（dh = 扇区数，起始 LBA = 2）
; --------------------------------------------------
load_kernel:
    pusha
    mov ah, 0x02            ; BIOS 读磁盘函数号
    mov al, dh              ; 读取 dh 个扇区
    mov ch, 0x00            ; 柱面 0
    mov dh, 0x00            ; 磁头 0
    mov cl, 0x02            ; 从扇区 2 开始
    mov dl, 0x00            ; 驱动器号 (0x00 = A/软盘; 若用硬盘改为 0x80)
    int 0x13                ; BIOS 中断
    jc disk_error           ; 读取失败
    popa
    ret

disk_error:
    mov si, disk_err_msg
    call puts
    hlt                     ; 死机

; --------------------------------------------------
; puts: 以 0 结尾字符串打印到屏幕 (teletype 模式)
; --------------------------------------------------
puts:
    lodsb                   ; AL = [SI++]
    or  al, al
    jz  .done
    mov ah, 0x0E            ; BIOS teletype
    int 0x10
    jmp puts
.done:
    ret

welcome_msg db "SeaOS booting...",0
disk_err_msg db "Disk Read Error!",0

; 填充至 510 字节并添加引导签名 0xAA55
TIMES 510-($-$$) db 0
DW 0xAA55