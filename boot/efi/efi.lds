/* UEFI Bootloader Linker Script */

ENTRY(efi_main)

SECTIONS
{
    . = 0;
    ImageBase = .;
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        *(.bss)
    }
}