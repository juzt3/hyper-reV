extern "C" {
#include "../../vendor/edk2/build/BaseLib/vshacks.h"
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
}

extern "C" CONST UINT32 _gUefiDriverRevision = 0;
extern "C" CHAR8 *gEfiCallerBaseName = "UefiBoot";

extern "C"
EFI_STATUS
EFIAPI
UefiUnload (
    IN EFI_HANDLE ImageHandle
    )
{
    (void)ImageHandle;
    return EFI_SUCCESS;
}

extern "C"
EFI_STATUS
EFIAPI
UefiMain (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
    )
{
    return EFI_SUCCESS;
}
