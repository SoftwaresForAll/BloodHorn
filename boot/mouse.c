#include "mouse.h"
#include "compat.h"
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SimplePointer.h>

static EFI_SIMPLE_POINTER_PROTOCOL* SimplePointer = NULL;

VOID EFIAPI InitMouse(VOID) {
    EFI_STATUS Status;
    Status = gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid, NULL, (VOID**)&SimplePointer);
}
// hows my code? ehehe
VOID EFIAPI GetMouseState(struct MouseState* state) {
    if (!SimplePointer || !state) return;

    EFI_SIMPLE_POINTER_STATE Mouse;
    EFI_STATUS Status = SimplePointer->GetState(SimplePointer, &Mouse);
    if (EFI_ERROR(Status)) return;
    state->x += (INT32)Mouse.RelativeMovementX;
    state->y += (INT32)Mouse.RelativeMovementY;
    state->left_button = Mouse.LeftButton ? TRUE : FALSE;
    state->right_button = Mouse.RightButton ? TRUE : FALSE;
}