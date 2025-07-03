#include <efi.h>
#include <efilib.h>

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    Print(L"SeaOS UEFI Bootloader\n");

    // TODO: Load kernel and jump to it
    
    while(1); // Temporary halt
    return EFI_SUCCESS;
}