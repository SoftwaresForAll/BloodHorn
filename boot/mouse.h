#ifndef BLOODHORN_MOUSE_H
#define BLOODHORN_MOUSE_H
#include <stdint.h>

struct MouseState {
    int x;
    int y;
    int left_button;
    int right_button;
};

void InitMouse(void);
void GetMouseState(struct MouseState* state);

#endif // BLOODHORN_MOUSE_H 