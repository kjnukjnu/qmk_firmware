#include QMK_KEYBOARD_H
#include "version.h"
#include "keymap_finnish.h"

// TODO: build in docker container

/* ccoders keymap

   In addition to defining the keymap, this file replaces the normal qmk event
   processing to be able to control low level details how to interpret the key
   events.

   Here as input we receive the key down/up events plus the timer events. As
   output we generate register_code() and unregister_code() calls and switch
   the leds on/off. Back to basics, so to speak.

   If you want to have full control how the key events are interpreted, you
   might be interested in this. Also if your keyboard is not configured with
   the US keyboard layout on the computer side, you might find something useful
   with the approach here. If you want to use graphical keymap editor, and
   avoid C coding, this isn't for you. Also this doesn't fit well to the qmk
   structure where the keymaps are separated from the generic logic.
*/

// TODO: better sectioning with comments

#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)

enum layers {
    BASE,
    NAVIGATION
};

static unsigned layer = 0;

// physical switch on the keyboard
typedef uint8_t key_t;
#define KEY_NO 255

// HID keycode reported via USB: negative == release
typedef int keycode_t;

typedef void (*key_func_t)(key_t key);
#define KCFUNC(m_arg_func) ((intptr_t)(m_arg_func))

static intptr_t key_release_info[KEY_COUNT];

static bool keycode_active_status[255];
typedef uint8_t mod_bits_t;
static mod_bits_t modifiers;

static const keycode_t PROGMEM modbit_keycodes[] =
{ KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI, KC_RCTL, KC_RSFT, KC_RALT, KC_RGUI };

#define MAX_TMP_KEYCODES 10
static keycode_t tmp_keycodes[MAX_TMP_KEYCODES];
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

static keycode_t keycode_plain(keycode_t code)
{
    return code < 0 ? -code : code;
}

static bool keycode_is_press(keycode_t code)
{
    return code >= 0;
}

static void keycode_send(keycode_t code)
{
    bool press = keycode_is_press(code);
    mod_bits_t modbit;

    code = keycode_plain(code);
    modbit = keycode_modbit(code);
    if (code == KC_NO || keycode_active(code) == press)
    {
        return;
    }
    if (press)
    {
        register_code(code);
    }
    else
    {
        unregister_code(code);
    }
    keycode_active_status[code] = press;
    if (press)
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

static void tmp_keycode(keycode_t code)
{
    keycode_t orig_code = code;
    bool press = keycode_is_press(code);
    code = keycode_plain(code);
    if (code == KC_NO ||
        tmp_keycode_count >= MAX_TMP_KEYCODES ||
        keycode_active_status[code] == press)
    {
        return;
    }
    keycode_send(orig_code);
    tmp_keycodes[tmp_keycode_count++] = orig_code;
}

#define _CALL_FUNC_1(func, a) func(a)
#define _CALL_FUNC_2(func, a, b) func(a); func(b)
#define _CALL_FUNC_3(func, a, b, c) func(a); func(b); func(c)
#define _CALL_FUNC_4(func, a, b, c, d) func(a); func(b); func(c); func(d)
#define _CALL_FUNC_5(func, a, b, c, d, e) func(a); func(b); func(c); func(d); func(e)
#define _CALL_FUNC_6(func, a, b, c, d, e, f) func(a); func(b); func(c); func(d); func(e); func(f)
#define _CALL_FUNC_7(func, a, b, c, d, e, f, g) func(a); func(b); func(c); func(d); func(e); func(f); func(g)
#define _CALL_FUNC_8(func, a, b, c, d, e, f, g, h) func(a); func(b); func(c); func(d); func(e); func(f); func(g); func(h)
#define _CALL_FUNC_9(func, a, b, c, d, e, f, g, h, i) func(a); func(b); func(c); func(d); func(e); func(f); func(g); func(h); func(i)
#define _CALL_FUNC_10(func, a, b, c, d, e, f, g, h, i, j) func(a); func(b); func(c); func(d); func(e); func(f); func(g); func(h); func(i); func(j)

#define GET_CALL_FUNC_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, name, ...) name
#define CALL_FUNC(func, ...) GET_CALL_FUNC_MACRO(__VA_ARGS__, _CALL_FUNC_10, _CALL_FUNC_9, _CALL_FUNC_8, _CALL_FUNC_7, _CALL_FUNC_6, _CALL_FUNC_5, _CALL_FUNC_4, _CALL_FUNC_3, _CALL_FUNC_2, _CALL_FUNC_1)(func, __VA_ARGS__)

#define keycode_send(...) CALL_FUNC(keycode_send, __VA_ARGS__)
#define tmp_keycode(...) CALL_FUNC(tmp_keycode, __VA_ARGS__)

static void tmp_modifiers_and_keycode(mod_bits_t mod_bits, keycode_t code)
{
    mod_bits_t add = mod_bits & ~modifiers;
    tmp_keycode(-code);
    for (int ix = 0; add; ++ix, add >>= 1)
    {
        if (add & 1)
        {
            tmp_keycode(modbit_keycode(ix));
        }
    }
    tmp_keycode(code);
}

typedef bool (*tmp_handler_t)(key_t, bool);

#define MAX_TMP_HANDLERS 10
static tmp_handler_t tmp_handlers[MAX_TMP_HANDLERS];
static int tmp_handler_cnt;

static void add_tmp_handler(tmp_handler_t func)
{
    if (tmp_handler_cnt < MAX_TMP_HANDLERS)
    {
        tmp_handlers[tmp_handler_cnt++] = func;
    }
}

static void remove_tmp_handler(tmp_handler_t func)
{
    int i;

    if (tmp_handler_cnt == 0)
    {
        return;
    }
    for (i = 0; i < tmp_handler_cnt && tmp_handlers[i] != func; ++i)
    {
    }
    if (i < tmp_handler_cnt)
    {
        for (; i < tmp_handler_cnt - 1; ++i)
        {
            tmp_handlers[i] = tmp_handlers[i + 1];
        }
        tmp_handlers[i] = NULL;
    }
    --tmp_handler_cnt;
}

static void k_navigation_layer_off(key_t key)
{
    ergodox_right_led_3_off();
    layer = BASE;
}

static void k_navigation_layer_on(key_t key)
{
    ergodox_right_led_3_set(128);
    layer = NAVIGATION;
    key_release_info[key] = KCFUNC(k_navigation_layer_off);
}

static int shift_count;
static uint16_t last_change;
static bool led_state;

static bool shift_handler(key_t key, bool pressed)
{
// TODO: mechanical caps lock
//  short time both shifts won't change the status
//  when caps lock is on, should shift keys revert the status momentarily?
    static const uint16_t interval = 1000;
    uint16_t now;

    if (key != KEY_NO)
    {
        return false;
    }
    now = timer_read(); // ms
    if ((uint16_t)(now - last_change) >= interval)
    {
        led_state = !led_state;
        last_change += interval;
        ergodox_right_led_2_set(led_state ? 50 : 0);
    }
    return false;
}

static void shift_down(key_t key, keycode_t kc, void (*up_func)(key_t key))
{
    keycode_send(kc);
    key_release_info[key] = KCFUNC(up_func);
    if (!shift_count++)
    {
        ergodox_right_led_2_set(50);
        led_state = true;
        last_change = timer_read();
        add_tmp_handler(shift_handler);
    }
}

static void shift_up(keycode_t code)
{
    keycode_send(-code);
    if (!--shift_count)
    {
        ergodox_right_led_2_off();
        remove_tmp_handler(shift_handler);
    }
}

static void k_lsft_release(key_t key) {shift_up(KC_LSFT);}
static void k_rsft_release(key_t key) {shift_up(KC_RSFT);}
static void k_lsft(key_t key) {shift_down(key, KC_LSFT, k_lsft_release);}
static void k_rsft(key_t key) {shift_down(key, KC_RSFT, k_rsft_release);}

static void k_four_dollar(key_t key)
{
    if ((modifiers & (MOD_BIT_LSHIFT | MOD_BIT_RSHIFT)) &&
        !(modifiers & (MOD_BIT_LCTRL | MOD_BIT_LALT | MOD_BIT_LGUI | MOD_BIT_RCTRL | MOD_BIT_RALT | MOD_BIT_RGUI)))
    {
        tmp_keycode(-KC_LSFT, -KC_RSFT);
        tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_4);
    }
    else if (modifiers == MOD_BIT_RALT)
    {
        tmp_keycode(-KC_RALT, KC_RSFT, KC_4);
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

static void k_tilde(key_t key)
{
  if (keycode_active(KC_RALT) ||
      keycode_active(FI_DIAE) ||
      keycode_active(KC_SPC))
    {
      return;
    }
  keycode_send(KC_RALT, FI_DIAE, -FI_DIAE, -KC_RALT, KC_SPC, -KC_SPC);
}

// TODO: tapping support: add function to receive all events until it requires removal

static const intptr_t PROGMEM keymap[][KEY_COUNT] = {
    /* BASE */
    {
        KC_ESC, KC_1, KC_2, KC_3, KCFUNC(k_four_dollar), KC_5, FI_DIAE, FI_ACUT, KC_6, KC_7, KC_8, KC_9, KC_0, FI_PLUS,

        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KCFUNC(k_brace_left), KCFUNC(k_brace_right), KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,

        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_NO, KC_NO, KC_H, KC_J, KC_K, KC_L, FI_ODIA, KC_RCTL /* ctrl/ä (FI_ADIA)) */,

        KCFUNC(k_lsft), KC_Z, KC_X, KC_C, KC_V, KC_B, KCFUNC(k_backslash), KCFUNC(k_tilde), KC_N, KC_M, FI_COMM, FI_DOT, FI_MINS, KCFUNC(k_rsft),

        FI_SECT, KCFUNC(k_pipe), FI_LABK, KCFUNC(k_greater_than), KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KCFUNC(k_navigation_layer_on), KCFUNC(k_lbracket), KCFUNC(k_rbracket), KCFUNC(k_ad), KCFUNC(k_dead_tilde),

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, FI_ARNG, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO /* teams mute/unmute */, KC_SPC, FI_ADIA, KC_BSPC, KC_NO,
    },

    /* NAVIGATION */
    {
        KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11,

        KC_TAB, KC_PGUP, KCFUNC(k_prev_word), KC_UP, KCFUNC(k_next_word), KC_NO, KCFUNC(k_brace_left), KCFUNC(k_brace_right), KC_Y, KC_BTN2, KC_MS_U, KC_BTN1, KC_NO, KC_F12,

        KC_LCTL, KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END, KC_NO, KC_NO, KC_WBAK, KC_MS_L, KC_MS_D, KC_MS_R, KC_WFWD, KC_RCTL /*ctrl/Ä (FI_ADIA)*/,

        KCFUNC(k_lsft), KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_NO, FI_MINS, KCFUNC(k_rsft),

        KC_NO /* MAC: unnecessary? */, KC_BRK, KC_INS, KC_NO, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO /*layer*/, KC_VOLD, KC_VOLU, KC_MPRV, KC_MNXT,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, KC_PSCR, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO /* teams mute/unmute */, KC_SPC, KC_NO, KC_BSPC, KC_NO,
    },
};

static void key_press(key_t key)
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

static void key_release(key_t key)
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

static bool call_tmp_handlers(key_t key, bool pressed)
{
    bool ret = false;
    for (int i = 0; i < tmp_handler_cnt; ++i)
    {
        ret = ret || tmp_handlers[i](key, pressed);
    }
    return ret;
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
        clean_tmp_keycodes();
        if (!call_tmp_handlers(key, event.pressed))
        {
            if (event.pressed)
            {
                key_press(key);
            }
            else
            {
                key_release(key);
            }
        }
    }
    else
    {
        (void)call_tmp_handlers(KEY_NO, false);
    }
    return true;
}

// dummy definition required by qmk build system, not used by us
const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS];
