#include "mouse.h"
#include <Uefi.h>
#include <Protocol/SimplePointer.h>

static EFI_SIMPLE_POINTER_PROTOCOL* SimplePointer = NULL;

void InitMouse(void) {
    EFI_STATUS Status;
    Status = gBS->LocateProtocol(&gEfiSimplePointerProtocolGuid, NULL, (void**)&SimplePointer);
}

void GetMouseState(struct MouseState* state) {
    if (!SimplePointer) return;
    EFI_SIMPLE_POINTER_STATE Mouse;
    EFI_STATUS Status = SimplePointer->GetState(SimplePointer, &Mouse);
    if (EFI_ERROR(Status)) return;
    state->x += Mouse.RelativeMovementX;
    state->y += Mouse.RelativeMovementY;
    state->left_button = Mouse.LeftButton;
    state->right_button = Mouse.RightButton;
} 