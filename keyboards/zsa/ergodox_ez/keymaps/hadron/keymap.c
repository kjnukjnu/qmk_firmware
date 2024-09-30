#include QMK_KEYBOARD_H
#include "version.h"

// required by quantum
const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS];

// this function bypasses the qmk state machine
bool user_action_exec(keyevent_t event)
{
    if (event.type != KEY_EVENT && event.type != TICK_EVENT)
    {
        return false;
    }
    register_code(KC_A);
    unregister_code(KC_A);
    return true;
}
