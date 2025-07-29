[bits 16]
org 0x7C00

; = SeaOS Boot Sector ===============================================
; 1. Print welcome
; 2. Load 20 sectors (kernel) @ 0x0000:0x1000
; 3. Enable A20
; 4. Load GDT & switch to 32-bit protected mode
; 5. Far-jump to kernel entry (_start at 0x1000)
; ====================================================================

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; --- Welcome -----------------------------------------------------
    mov si, welcome_msg
    call puts

    ; --- Load kernel -------------------------------------------------
    mov bx, 0x1000      ; ES:BX points to 0x0000:0x1000
    mov dh, 20          ; 20 sectors (≈10 KiB sample kernel)
    call load_kernel

    ; --- Enable A20 --------------------------------------------------
    call enable_a20

    ; --- Enter Protected Mode ---------------------------------------
    lgdt  [gdt_descriptor]
    mov   eax, cr0
    or    eax, 1        ; PE = 1
    mov   cr0, eax
    jmp   CODE_SEL:pm_start  ; Far jump flushes pipeline & loads CS

; --------------------------------------------------------------------
; 32-bit Protected-Mode Stub
; --------------------------------------------------------------------
[bits 32]
pm_start:
    mov  ax, DATA_SEL
    mov  ds, ax
    mov  es, ax
    mov  fs, ax
    mov  gs, ax
    mov  ss, ax
    mov  esp, 0x9FC00        ; Temporary stack under 640 KiB

    jmp  CODE_SEL:0x1000     ; Jump to kernel (_start)

; --------------------------------------------------------------------
; Back to 16-bit helpers & data
; --------------------------------------------------------------------
[bits 16]

; BIOS read sectors – ES:BX dest, dh = count, starts from LBA 2
load_kernel:
    pusha
    mov ah, 0x02       ; BIOS read
    mov al, dh         ; sector count
    mov ch, 0x00
    mov dh, 0x00
    mov cl, 0x02       ; LBA 2
    mov dl, 0x00       ; Drive 0 (floppy); for HDD use 0x80
    int 0x13
    jc  disk_error
    popa
    ret

enable_a20:
    mov ax, 0x2401     ; INT 15h AX=2401 -> A20 gate on
    int 0x15
    ret

disk_error:
    mov si, disk_err_msg
    call puts
    hlt
    jmp $

; --- puts: print zero-terminated string -----------------------------
puts:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp puts
.done:
    ret

; -------------------- GDT -------------------------------------------
align 8
gdt_start:
    dq 0x0000000000000000      ; Null descriptor
    dq 0x00CF9A000000FFFF      ; Code 0x08: base=0, limit=4 GB, 32-bit, RX
    dq 0x00CF92000000FFFF      ; Data 0x10: base=0, limit=4 GB, RW

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEL equ 0x08
DATA_SEL equ 0x10

; -------------------- Messages & signature --------------------------
welcome_msg db "SeaOS booting... entering protected mode.",0
disk_err_msg db "Disk read error!",0

; Pad to 510 bytes then add boot signature 0xAA55
TIMES 510-($-$$) db 0
DW 0xAA55