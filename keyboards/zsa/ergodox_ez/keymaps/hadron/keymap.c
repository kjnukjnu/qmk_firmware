#include QMK_KEYBOARD_H
#include "version.h"
#include "keymap_finnish.h"

// TODO: better sectioning with comments

const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS]; // required by quantum, not used by us

#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)

enum layers {
    BASE,
    NAVIGATION
};

static unsigned layer = 0;

typedef uint8_t key_t; // physical switch on the keyboard
typedef uint8_t keycode_t; // HID keycode reported via USB
typedef int dir_key_t; // physical switch on the keyboard, negative == release
typedef int dir_keycode_t; // HID keycode: negative == release

typedef void (*key_func_t)(key_t key);
#define KCFUNC(m_arg_func) ((intptr_t)(m_arg_func))

static intptr_t key_release_info[KEY_COUNT];

static bool keycode_active_status[255];
typedef uint8_t mod_bits_t;
static mod_bits_t modifiers;

static const keycode_t PROGMEM modbit_keycodes[] =
{ KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI, KC_RCTL, KC_RSFT, KC_RALT, KC_RGUI };

#define MAX_TMP_KEYCODES 10
static dir_keycode_t tmp_keycodes[MAX_TMP_KEYCODES];
static int tmp_keycode_count;

static mod_bits_t keycode_modbit(keycode_t code)
{
    switch (code)
    {
    case KC_LCTL:
        return MOD_BIT_LCTRL;
    case KC_LSFT:
        return MOD_BIT_LSHIFT;
    case KC_LALT:
        return MOD_BIT_LALT;
    case KC_LGUI:
        return MOD_BIT_LGUI;
    case KC_RCTL:
        return MOD_BIT_RCTRL;
    case KC_RSFT:
        return MOD_BIT_RSHIFT;
    case KC_RALT:
        return MOD_BIT_RALT;
    case KC_RGUI:
        return MOD_BIT_RGUI;
    default:
        return 0;
    };
}

static keycode_t modbit_keycode(int bit_number)
{
    return modbit_keycodes[bit_number];
}

static bool keycode_active(keycode_t code)
{
    return keycode_active_status[code];
}

static dir_keycode_t release_keycode(keycode_t code)
{
    return -(dir_keycode_t)code;
}

static keycode_t keycode_plain(dir_keycode_t dir_code)
{
    return dir_code < 0 ? -dir_code : dir_code;
}

static bool keycode_is_press(dir_keycode_t dir_code)
{
    return dir_code >= 0;
}

static void keycode_send(dir_keycode_t dir_code)
{
    keycode_t code = keycode_plain(dir_code);
    mod_bits_t modbit = keycode_modbit(code);
    if (dir_code == KC_NO ||
        keycode_active(code) == keycode_is_press(dir_code))
    {
        return;
    }
    if (keycode_is_press(dir_code))
    {
        register_code(code);
    }
    else
    {
        unregister_code(code);
    }
    keycode_active_status[code] = keycode_is_press(dir_code);
    if (keycode_is_press(dir_code))
    {
        modifiers |= modbit;
    }
    else
    {
        modifiers &= ~modbit;
    }
}

static void simple_key_press(key_t key, keycode_t code)
{
    keycode_send(code);
    key_release_info[key] = code;
}

static void tmp_keycode(dir_keycode_t dir_code)
{
    keycode_t code = keycode_plain(dir_code);
    if (dir_code == KC_NO ||
        tmp_keycode_count >= MAX_TMP_KEYCODES ||
        keycode_active_status[code] == keycode_is_press(dir_code))
    {
        return;
    }
    keycode_send(dir_code);
    tmp_keycodes[tmp_keycode_count++] = dir_code;
}

static void tmp_modifiers_and_keycode(mod_bits_t mod_bits, keycode_t code)
{
    // TODO: don't remove existing modifiers?
    mod_bits_t remove = modifiers & ~mod_bits;
    mod_bits_t add = mod_bits & ~modifiers;
    tmp_keycode(release_keycode(code));
    for (int ix = 0; remove; ++ix, remove >>= 1)
    {
        if (remove & 1)
        {
            tmp_keycode(release_keycode(modbit_keycode(ix)));
        }
    }
    for (int ix = 0; add; ++ix, add >>= 1)
    {
        if (add & 1)
        {
            tmp_keycode(modbit_keycode(ix));
        }
    }
    tmp_keycode(code);
}

static void k_navigation_layer_off(key_t key)
{
    ergodox_right_led_3_off();
    layer = BASE;
}

static void k_navigation_layer_on(key_t key)
{
    ergodox_right_led_3_on();
    layer = NAVIGATION;
    key_release_info[key] = KCFUNC(k_navigation_layer_off);
}

static void k_four_dollar(key_t key)
{
    // TODO: make RALT + 4 produce ¤ (just for fun)
    if ((modifiers & (MOD_BIT_LSHIFT | MOD_BIT_RSHIFT)) &&
        !(modifiers & (MOD_BIT_LCTRL | MOD_BIT_LALT | MOD_BIT_LGUI | MOD_BIT_RCTRL | MOD_BIT_RALT | MOD_BIT_RGUI)))
    {
        tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_4);
    }
    else
    {
        simple_key_press(key, KC_4);
    }
}

static void k_brace_left(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_7);}
static void k_brace_right(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_0);}
static void k_backslash(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, FI_PLUS);}
static void k_pipe(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, FI_LABK);}
static void k_greater_than(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_LSHIFT, FI_LABK);}
static void k_lbracket(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_8);}
static void k_rbracket(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_9);}
static void k_ad(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_2);}
static void k_dead_tilde(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, FI_DIAE);}
static void k_prev_word(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_LCTRL, KC_LEFT);}
static void k_next_word(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_LCTRL, KC_RIGHT);}

// TODO: tilde: if right modifiers, send the necessary keys

// TODO: tapping support: add function to receive all events until it requires removal

static const intptr_t PROGMEM keymap[][KEY_COUNT] = {
    /* BASE */
    {
        KC_ESC, KC_1, KC_2, KC_3, KCFUNC(k_four_dollar), KC_5, FI_DIAE, FI_ACUT, KC_6, KC_7, KC_8, KC_9, KC_0, FI_PLUS,

        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KCFUNC(k_brace_left), KCFUNC(k_brace_right), KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,

        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_NO, KC_NO, KC_H, KC_J, KC_K, KC_L, FI_ODIA, KC_RCTL /* ctrl/ä (FI_ADIA)) */,

        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KCFUNC(k_backslash), KC_NO /* tilde: RALT + FI_DIAE + KC_SPACE - KC_SPACE */, KC_N, KC_M, FI_COMM, FI_DOT, FI_MINS, KC_RSFT,

        FI_SECT, KCFUNC(k_pipe), FI_LABK, KCFUNC(k_greater_than), KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KCFUNC(k_navigation_layer_on), KCFUNC(k_lbracket), KCFUNC(k_rbracket), KCFUNC(k_ad), KCFUNC(k_dead_tilde),

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, FI_ARNG, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO /* teams mute/unmute */, KC_SPC, KC_NO /* FI_ADIA */, KC_BSPC, KC_NO,
    },

    /* NAVIGATION */
    {
        KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11,

        KC_TAB, KC_PGUP, KCFUNC(k_prev_word), KC_UP, KCFUNC(k_next_word), KC_NO, KCFUNC(k_brace_left), KCFUNC(k_brace_right), KC_Y, KC_BTN2, KC_MS_U, KC_BTN1, KC_NO, KC_F12,

        KC_LCTL, KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END, KC_NO, KC_NO, KC_WBAK, KC_MS_L, KC_MS_D, KC_MS_R, KC_NO /*ctrl alt, unnecessary?*/, KC_RCTL /*ctrl/Ä (FI_ADIA)*/,

        KC_LSFT, KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_NO, FI_MINS, KC_RSFT,

        KC_NO /* MAC: unnecessary? */, KC_BRK, KC_INS, KC_NO, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO /*layer*/, KC_VOLD, KC_VOLU, KC_MPRV, KC_MNXT,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, KC_PSCR, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO /* teams mute/unmute */, KC_SPC, KC_NO /* FI_ADIA */, KC_BSPC, KC_NO,
    },
};

static void hadron_key_press(uint16_t key)
{
    const intptr_t *key_info = &keymap[layer][key];

    if (*key_info > 255)
    {
        key_func_t func = (key_func_t)*key_info;
        func(key);
    }
    else if (*key_info != KC_NO)
    {
        simple_key_press(key, *key_info);
    }
}

static void hadron_key_release(uint16_t key)
{
    intptr_t *key_info = &key_release_info[key];

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
        /* TODO: allow removal of ordinary keys that are not part of the
         * temporary keys without cleaning the tmp_keycodes. This makes the
         * sequence SHIFT/5/4/-5 stay in a state where the autorepeat of $
         * works. */
        // TODO: feed requested all-event functions
        clean_tmp_keycodes();
        if (event.pressed)
        {
            hadron_key_press(key);
        }
        else
        {
            hadron_key_release(key);
        }
    }
    else
    {
        // TODO: feed requested all-event functions
    }
    return true;
}
