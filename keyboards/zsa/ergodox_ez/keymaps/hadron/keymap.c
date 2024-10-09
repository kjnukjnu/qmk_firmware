#include QMK_KEYBOARD_H
#include "version.h"

// required by quantum
const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS];

struct hadron_key_exception
{
};

enum layers {
    BASE,
    NAVIGATION
};

struct hadron_key
{
    uint8_t keycode;
    struct hadron_key_exception* exceptions;
};

static const struct hadron_key PROGMEM keymap[][6][14] = {
    /* BASE */
    {
        {{KC_NO}, {KC_DEL}, {KC_LGUI}, {KC_LALT}, {KC_PRINT_SCREEN}, {KC_NO}, {KC_NO}, {KC_RALT}, {KC_RGUI}, {KC_NO}, {KC_SPACE}, {KC_NO}, {KC_NO}, {KC_NO}, },

        {{KC_GRV}, {KC_NO /* PIPE: RALT + KC_GRAVE */}, {KC_GRAVE /* < */}, {KC_NO /* SHIFT + KC_GRAVE */}, {KC_ENTER}, {KC_NO}, {KC_NO}, {KC_NO}, {KC_NO}, {KC_NO /* layer */}, {KC_NO /* [: RALT + KC_8 */}, {KC_NO /* ]: RALT + KC_9 */}, {KC_NO /* @: RALT + KC_2 */}, {KC_NO /* dead tilde: RALT + KC_RIGHT_BRACKET */}},

        {{KC_LSFT}, {KC_Z}, {KC_X}, {KC_C}, {KC_V}, {KC_B}, {KC_NO /* backslash: RALT + KC_MINUS */}, {KC_NO /* tilde: RALT + KC_RIGHT_BRACKET + KC_SPACE - KC_SPACE */}, {KC_N}, {KC_M}, {KC_COMMA}, {KC_DOT}, {KC_SLASH}, {KC_RSFT}, },

        {{KC_LEFT_CTRL}, {KC_A}, {KC_S}, {KC_D}, {KC_F}, {KC_G}, {KC_NO}, {KC_NO}, {KC_H}, {KC_J}, {KC_K}, {KC_L}, {KC_SEMICOLON}, {KC_NO /* ctrl/ä (KC_QUOTE) */},},

        {{KC_TAB}, {KC_Q}, {KC_W}, {KC_E}, {KC_R}, {KC_T}, {KC_NO /* {: RALT + KC_7 */}, {KC_NO /* }: RALT + KC_0 */}, {KC_Y}, {KC_U}, {KC_I}, {KC_O}, {KC_P}, {KC_BACKSLASH},},

        {{KC_ESCAPE}, {KC_1}, {KC_2}, {KC_3}, {KC_4}, {KC_5}, {KC_RIGHT_BRACKET}, {KC_NO /* dead acute: RALT + KC_RIGHT_BRACKET */}, {KC_6}, {KC_7}, {KC_8}, {KC_9}, {KC_0}, {KC_MINUS}},
    },

    /* NAVIGATION */
    {
    },
};

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
