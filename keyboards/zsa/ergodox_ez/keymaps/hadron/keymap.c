#include QMK_KEYBOARD_H
#include "version.h"
#include "keymap_finnish.h"

const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS]; // required by quantum, not used by us

#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)

enum layers {
    BASE,
    NAVIGATION
};

static unsigned layer = 0;

typedef uint8_t key_t; // physical switch on the keyboard
typedef uint8_t keycode_t; // HID keycode reported via USB
typedef int dir_key_t; // physical switch on the keyboard, negative == up
typedef int dir_keycode_t; // HID keycode: negative == up

typedef void (*key_func_t)(key_t key);
#define KCFUNC(m_arg_func) ((intptr_t)(m_arg_func))

static intptr_t keyup_info[KEY_COUNT];

// todo: more efficient status of modifiers
static bool keycode_active_status[255];

#define MAX_TMP_KEYCODES 10
static dir_keycode_t tmp_keycodes[MAX_TMP_KEYCODES];
static int tmp_keycode_count;

static bool keycode_active(keycode_t code)
{
    return keycode_active_status[code];
}

static keycode_t keycode_plain(dir_keycode_t dir_code)
{
    return dir_code < 0 ? -dir_code : dir_code;
}

static bool keycode_is_down(dir_keycode_t dir_code)
{
    return dir_code >= 0;
}

static void keycode_send(dir_keycode_t dir_code)
{
    keycode_t code = keycode_plain(dir_code);
    if (dir_code == KC_NO ||
        keycode_active_status[code] == keycode_is_down(dir_code))
    {
        return;
    }
    if (keycode_is_down(dir_code))
    {
        register_code(code);
    }
    else
    {
        unregister_code(code);
    }
    keycode_active_status[code] = keycode_is_down(dir_code);
}

static void simple_key_down(key_t key, keycode_t code)
{
    keycode_send(code);
    keyup_info[key] = code;
}

static void tmp_keycode(dir_keycode_t dir_code)
{
    keycode_t code = keycode_plain(dir_code);
    if (dir_code == KC_NO ||
        tmp_keycode_count >= MAX_TMP_KEYCODES ||
        keycode_active_status[code] == keycode_is_down(dir_code))
    {
        return;
    }
    keycode_send(dir_code);
    tmp_keycodes[tmp_keycode_count++] = dir_code;
}

static bool shift_active(void)
{
    return keycode_active(KC_LSFT) || keycode_active(KC_RSFT);
}

static bool ctrl_alt_gui_active(void)
{
    return keycode_active(KC_LCTL) || keycode_active(KC_RCTL) ||
        keycode_active(KC_LALT) || keycode_active(KC_RALT) ||
        keycode_active(KC_LGUI) || keycode_active(KC_RGUI);
}

static void navigation_layer_off(key_t key)
{
    ergodox_right_led_3_off();
    layer = BASE;
}

static void navigation_layer_on(key_t key)
{
    ergodox_right_led_3_on();
    layer = NAVIGATION;
    keyup_info[key] = KCFUNC(navigation_layer_off);
}

static void four_dollar(key_t key)
{
    if (shift_active() & !ctrl_alt_gui_active())
    {
        /* TODO: instead of sending tmp keycodes, give the desired modifier
         * setup and keycode(s) after that. */
        tmp_keycode(-KC_LSFT);
        tmp_keycode(-KC_RSFT);
        tmp_keycode(KC_RALT);
        tmp_keycode(KC_4);
    }
    else
    {
        simple_key_down(key, KC_4);
    }
}

static const intptr_t PROGMEM keymap[][KEY_COUNT] = {
    /* BASE */
    {
        KC_ESC, KC_1, KC_2, KC_3, KCFUNC(four_dollar), KC_5, FI_DIAE, FI_ACUT, KC_6, KC_7, KC_8, KC_9, KC_0, FI_PLUS,

        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_NO /* : RALT + KC_7 */, KC_NO /* : RALT + KC_0 */, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,

        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_NO, KC_NO, KC_H, KC_J, KC_K, KC_L, FI_ODIA, KC_RCTL /* ctrl/ä (FI_ADIA)) */,

        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_NO /* backslash: RALT + KC_MINUS */, KC_NO /* tilde: RALT + KC_RIGHT_BRACKET + KC_SPACE - KC_SPACE */, KC_N, KC_M, FI_COMM, FI_DOT, FI_MINS, KC_RSFT,

        FI_SECT, KC_NO /* PIPE: RALT + FI_LABK */, FI_LABK, KC_NO /* >: SHIFT + FI_LABK */, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KCFUNC(navigation_layer_on), KC_NO /* [: RALT + KC_8 */, KC_NO /* ]: RALT + KC_9 */, KC_NO /* @: RALT + KC_2 */, KC_NO /* dead tilde: RALT + KC_RIGHT_BRACKET */,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, FI_ARNG, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO, KC_SPC, KC_NO, KC_BSPC, KC_NO,
    },

    /* NAVIGATION */
    {
        KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11,

        KC_TAB, KC_PGUP, KC_NO /* CTRL + LEFT: prev word/CTRL-W*/, KC_UP, KC_NO /* CTRL + RIGHT next word*/, KC_NO, KC_NO /* : RALT + KC_7 */, KC_NO /* : RALT + KC_0 */, KC_Y, KC_BTN2, KC_MS_U, KC_BTN1, KC_NO, KC_F12,

        KC_LCTL, KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END, KC_NO, KC_NO, KC_WBAK, KC_MS_L, KC_MS_D, KC_MS_R, KC_NO /*ctrl alt*/, KC_RCTL /*ctrl/Ä (FI_ADIA)*/,

        KC_LSFT, KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_NO, FI_MINS, KC_RSFT,

        KC_NO /* MAC */, KC_BRK, KC_INS, KC_NO, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO /*layer*/, KC_VOLD, KC_VOLU, KC_MPRV, KC_MNXT,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, KC_PSCR, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO, KC_SPC, KC_NO, KC_BSPC, KC_NO,
    },
};

static void hadron_key_down(uint16_t key)
{
    const intptr_t *key_info = &keymap[layer][key];

    if (*key_info > 255)
    {
        key_func_t func = (key_func_t)*key_info;
        func(key);
    }
    else if (*key_info != KC_NO)
    {
        simple_key_down(key, *key_info);
    }
}

static void hadron_key_up(uint16_t key)
{
    intptr_t *key_info = &keyup_info[key];

    if (*key_info > 255)
    {
        key_func_t func = (key_func_t)*key_info;
        func(key);
    }
    else if (*key_info != KC_NO)
    {
        keycode_send(-*key_info);
    }
    *key_info = KC_NO;
}

static void clean_tmp_keycodes(void)
{
    while (tmp_keycode_count > 0)
    {
        keycode_send(-tmp_keycodes[--tmp_keycode_count]);
    }
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
        clean_tmp_keycodes();
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
        // TODO
    }
    return true;
}
