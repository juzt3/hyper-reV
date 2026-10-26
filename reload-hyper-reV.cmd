@echo off
setlocal

mountvol Z: /S >nul 2>&1

set boot_directory=Z:\EFI\Microsoft\Boot\

if exist "%boot_directory%bootmgfw.original.efi" (
    copy /Y "%~dp0hyperv-attachment.dll" "%boot_directory%" >nul
) else (
    attrib -s "%boot_directory%bootmgfw.efi"
    move "%boot_directory%bootmgfw.efi" "%boot_directory%bootmgfw.original.efi"

    copy /Y "%~dp0uefi-boot.efi" "%boot_directory%bootmgfw.efi" >nul
    copy /Y "%~dp0hyperv-attachment.dll" "%boot_directory%" >nul

    bcdedit /set hypervisorlaunchtype auto >nul
)

endlocal
