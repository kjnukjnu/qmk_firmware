#include QMK_KEYBOARD_H
#include "version.h"

// required by quantum
const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS];

// this function bypasses the qmk state machine
bool user_action_exec(keyevent_t event)
{
    static unsigned ticks = 0;
    static bool led_1_state = false;

    if (event.type != KEY_EVENT && event.type != TICK_EVENT)
    {
        return false;
    }
    if (event.type == KEY_EVENT)
    {
        register_code(KC_A);
        unregister_code(KC_A);
    }
    else
    {
        if (++ticks > 1000)
        {
            led_1_state = !led_1_state;
            if (led_1_state)
            {
                ergodox_right_led_1_on();
            }
            else
            {
                ergodox_right_led_1_off();
            }
            ticks = 0;
        }
    }
    return true;
}
