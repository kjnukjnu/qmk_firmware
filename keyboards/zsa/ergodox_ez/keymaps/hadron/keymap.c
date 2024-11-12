#include QMK_KEYBOARD_H
#include "version.h"
#include "keymap_finnish.h"

const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS]; // required by quantum, not used by us

#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)

enum layers {
    BASE,
    NAVIGATION
};

//static_assert(MATRIX_ROWS == 6);
//static_assert(MATRIX_COLS == 14);

static const intptr_t PROGMEM keymap[][KEY_COUNT] = {
    /* BASE */
    {
        KC_ESC, KC_1, KC_2, KC_3, KC_4, KC_5, FI_DIAE, FI_ACUT, KC_6, KC_7, KC_8, KC_9, KC_0, FI_PLUS,

        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_NO /* : RALT + KC_7 */, KC_NO /* : RALT + KC_0 */, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,

        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_NO, KC_NO, KC_H, KC_J, KC_K, KC_L, FI_ODIA, KC_NO /* ctrl/ä (FI_ADIA)) */,

        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_NO /* backslash: RALT + KC_MINUS */, KC_NO /* tilde: RALT + KC_RIGHT_BRACKET + KC_SPACE - KC_SPACE */, KC_N, KC_M, FI_COMM, FI_DOT, FI_MINS, KC_RSFT,

        FI_SECT, KC_NO /* PIPE: RALT + FI_LABK */, FI_LABK, KC_NO /* >: SHIFT + FI_LABK */, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO /* layer */, KC_NO /* [: RALT + KC_8 */, KC_NO /* ]: RALT + KC_9 */, KC_NO /* @: RALT + KC_2 */, KC_NO /* dead tilde: RALT + KC_RIGHT_BRACKET */,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, FI_ARNG, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO, KC_SPC, KC_NO, KC_BSPC, KC_NO,
    },

    /* NAVIGATION */
    {
        KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11,

        KC_TAB, KC_PGUP, KC_NO /* CTRL + LEFT: prev word*/, KC_UP, KC_NO /* CTRL + RIGHT next word*/, KC_NO, KC_NO /* : RALT + KC_7 */, KC_NO /* : RALT + KC_0 */, KC_Y, KC_BTN1/*?*/, KC_MS_U, KC_BTN2/*?*/, KC_NO, KC_F12,

        KC_LCTL, KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END, KC_NO, KC_NO, KC_WBAK, KC_MS_L, KC_MS_D, KC_MS_R, KC_NO /*ctrl alt*/, KC_NO /*ctrl/Ä (FI_ADIA)*/,

        KC_LSFT, KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_NO, FI_MINS, KC_RSFT,

        KC_NO /* MAC */, KC_BRK, KC_INS, KC_NO, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO /*layer*/, KC_VOLD, KC_VOLU, KC_MPRV, KC_MNXT,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, KC_PSCR, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO, KC_SPC, KC_NO, KC_BSPC, KC_NO,
    },
};

static void hadron_tick_event(void)
{
    static bool time_valid;
    static uint16_t last_change;
    static bool led_1_state = false;
    uint16_t now = timer_read(); // ms
    static const uint16_t interval = 1000;

    if (!time_valid)
    {
        last_change = now;
        time_valid = true;
        return;
    }
    if ((uint16_t)(now - last_change) >= interval)
    {
        led_1_state = !led_1_state;
        last_change += interval;
        if (led_1_state)
        {
            ergodox_right_led_1_on();
        }
        else
        {
            ergodox_right_led_1_off();
        }
    }
}

static intptr_t keyup_info[KEY_COUNT];
static unsigned layer = 0;

static void hadron_key_down(uint16_t key)
{
    const intptr_t *key_info = &keymap[layer][key];

    if (*key_info > 255)
    {
        // TODO
    }
    else if (*key_info != KC_NO)
    {
        register_code(*key_info);
        keyup_info[key] = *key_info;
    }
}

static void hadron_key_up(uint16_t key)
{
    intptr_t *key_info = &keyup_info[key];

    if (*key_info> 255)
    {
        // TODO
    }
    else if (*key_info != KC_NO)
    {
        unregister_code(*key_info);
    }
    *key_info = KC_NO;
}

// this function bypasses the qmk state machine
bool user_action_exec(keyevent_t event)
{
    if (event.type != KEY_EVENT && event.type != TICK_EVENT)
    {
        return false;
    }
    if (event.type == KEY_EVENT)
    {
        uint16_t key = event.key.row + event.key.col * MATRIX_ROWS;
        if (event.pressed)
        {
            hadron_key_down(key);
        }
        else
        {
            hadron_key_up(key);
        }
    }
    else
    {
        hadron_tick_event();
    }
    return true;
}
