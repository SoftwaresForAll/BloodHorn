#ifndef BLOODHORN_MOUSE_H
#define BLOODHORN_MOUSE_H

#include <Uefi.h>
#include "compat.h"

struct MouseState {
    INT32 x;
    INT32 y;
    BOOLEAN left_button;
    BOOLEAN right_button;
};

VOID EFIAPI InitMouse(VOID);
VOID EFIAPI GetMouseState(struct MouseState* state);

#endif // BLOODHORN_MOUSE_H