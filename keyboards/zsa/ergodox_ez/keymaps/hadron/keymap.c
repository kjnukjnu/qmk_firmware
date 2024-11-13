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

typedef void (*key_func_t)(bool down);
#define KCFUNC(m_arg_func) ((intptr_t)(m_arg_func))

static intptr_t keyup_info[KEY_COUNT];
static bool keycode_active_status[255];
#define MAX_TMP_KEYCODES 10
static int tmp_keycodes[MAX_TMP_KEYCODES];
static int tmp_keycode_count;

static bool keycode_active(uint8_t keycode)
{
    return keycode_active_status[keycode];
}

static uint8_t keycode_plain(int keycode)
{
    return keycode < 0 ? -keycode : keycode;
}

static bool keycode_is_down(int keycode)
{
    return keycode >= 0;
}

static void keycode_send(int keycode)
{
    int ix = keycode_plain(keycode);
    if (keycode == KC_NO || keycode_active_status[ix] == keycode_is_down(keycode))
    {
        return;
    }
    if (keycode_is_down(keycode))
    {
        register_code(ix);
    }
    else
    {
        unregister_code(ix);
    }
    keycode_active_status[ix] = keycode_is_down(keycode);
}

static void tmp_keycode(int keycode)
{
    int ix = keycode_plain(keycode);
    if (keycode == KC_NO ||
        tmp_keycode_count >= MAX_TMP_KEYCODES ||
        keycode_active_status[ix] == keycode_is_down(keycode))
    {
        return;
    }
    keycode_send(keycode);
    tmp_keycodes[tmp_keycode_count++] = keycode;
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

static void navigation_layer(bool down)
{
    if (down)
    {
        ergodox_right_led_3_on();
    }
    else
    {
        ergodox_right_led_3_off();
    }
    layer = down;
}

static void four_dollar(bool down)
{
    if (!down)
    {
        return;
    }
    if (shift_active() & !ctrl_alt_gui_active())
    {
        tmp_keycode(-KC_LSFT);
        tmp_keycode(-KC_RSFT);
        tmp_keycode(KC_RALT);
        tmp_keycode(KC_4);
    }
    else
    {
        keycode_send(KC_4);
        keyup_info[4] = KC_4;
    }
}

static const intptr_t PROGMEM keymap[][KEY_COUNT] = {
    /* BASE */
    {
        KC_ESC, KC_1, KC_2, KC_3, KCFUNC(four_dollar), KC_5, FI_DIAE, FI_ACUT, KC_6, KC_7, KC_8, KC_9, KC_0, FI_PLUS,

        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_NO /* : RALT + KC_7 */, KC_NO /* : RALT + KC_0 */, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,

        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_NO, KC_NO, KC_H, KC_J, KC_K, KC_L, FI_ODIA, KC_RCTL /* ctrl/ä (FI_ADIA)) */,

        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_NO /* backslash: RALT + KC_MINUS */, KC_NO /* tilde: RALT + KC_RIGHT_BRACKET + KC_SPACE - KC_SPACE */, KC_N, KC_M, FI_COMM, FI_DOT, FI_MINS, KC_RSFT,

        FI_SECT, KC_NO /* PIPE: RALT + FI_LABK */, FI_LABK, KC_NO /* >: SHIFT + FI_LABK */, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KCFUNC(navigation_layer), KC_NO /* [: RALT + KC_8 */, KC_NO /* ]: RALT + KC_9 */, KC_NO /* @: RALT + KC_2 */, KC_NO /* dead tilde: RALT + KC_RIGHT_BRACKET */,

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

    keyup_info[key] = *key_info;
    if (*key_info > 255)
    {
        key_func_t func = (key_func_t)*key_info;
        func(true);
    }
    else if (*key_info != KC_NO)
    {
        keycode_send(*key_info);
    }
}

static void hadron_key_up(uint16_t key)
{
    intptr_t *key_info = &keyup_info[key];

    if (*key_info > 255)
    {
        key_func_t func = (key_func_t)*key_info;
        func(false);
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
