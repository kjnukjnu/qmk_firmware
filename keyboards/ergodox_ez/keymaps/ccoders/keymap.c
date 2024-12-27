#include QMK_KEYBOARD_H
#include "version.h"
#include "keymap_finnish.h"

// TODO: build in docker container
// TODO: check if there is superfluous debouncing in use?
// TODO: disable oryx

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

/* Physical switch on the keyboard. */

typedef uint8_t key_t;
#define KEY_NO 255
#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)

/* HID keycode reported via USB: negative == release. */

typedef int keycode_t;

static keycode_t keycode_plain(keycode_t code)
{
    return code < 0 ? -code : code;
}

static bool keycode_is_press(keycode_t code)
{
    return code >= 0;
}

/* Macros to call a function for each of the arguments. */

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

/* Modifiers and keycodes */

typedef uint8_t mod_bits_t; // modifier bitmap type
static mod_bits_t modifiers; // current modifiers

static const keycode_t modbit_keycodes[] =
{ KC_LCTL, KC_LSFT, KC_LALT, KC_LGUI, KC_RCTL, KC_RSFT, KC_RALT, KC_RGUI };

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

/* Current keyboard status */

static intptr_t key_release_info[KEY_COUNT]; // what to do on key release
static bool keycode_active_status[255]; // what keycodes are currently reported

static bool keycode_active(keycode_t code)
{
    return keycode_active_status[code];
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

#define keycode_send(...) CALL_FUNC(keycode_send, __VA_ARGS__)

static void simple_key_press(key_t key, keycode_t code)
{
    keycode_send(code);
    key_release_info[key] = code;
}

/* Temporary keycodes to be reverted on the next key event */

#define MAX_TMP_KEYCODES 10
static keycode_t tmp_keycodes[MAX_TMP_KEYCODES];
static int tmp_keycode_count;

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

/* Temporary additional handlers to support complex key functions */

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

/* LEDs */

#define CAPS_LED 1
#define LAYER_LED 3

static void kb_led_set(int led, bool on)
{
    switch (led)
    {
    case 1:
        if (on)
        {
            ergodox_right_led_1_on();
        }
        else
        {
            ergodox_right_led_1_off();
        }
        break;
    case 2:
        if (on)
        {
            ergodox_right_led_2_on();
        }
        else
        {
            ergodox_right_led_2_off();
        }
        break;
    case 3:
        if (on)
        {
            ergodox_right_led_3_on();
        }
        else
        {
            ergodox_right_led_3_off();
        }
        break;
    default:
        break;
    }
}

/* Other */

static unsigned timer_diff(unsigned now, unsigned start)
{
    return now - start;
}

/* Key functions for the cases simple keycode mapping is not enough */

// layer definitions used by our keymap

enum layers {
    BASE,
    NAVIGATION
};
static unsigned layer = 0;

// forward declarations for key functions
static const intptr_t keymap[][KEY_COUNT];
static void key_press(key_t key);
static void key_release(key_t key);

// key function type definition and macro to use it in keymap
typedef void (*key_func_t)(key_t key);
#define KCFUNC(m_arg_func) ((intptr_t)(m_arg_func))

// the actual key functions start here

static void k_brace_left(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_7);}
static void k_brace_right(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_0);}
static void k_backslash(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, FI_PLUS);}
static void k_pipe(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, FI_LABK);}
static void k_greater_than(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_LSHIFT, FI_LABK);}
static void k_lbracket(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_8);}
static void k_rbracket(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_9);}
static void k_ad(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, KC_2);}
static void k_dead_tilde(key_t key) {tmp_modifiers_and_keycode(MOD_BIT_RALT, FI_DIAE);}

static void k_next_word(key_t key)
{
    if (!modifiers)
    {
        tmp_modifiers_and_keycode(MOD_BIT_LCTRL, KC_RIGHT);
    }
    else
    {
        simple_key_press(key, KC_R);
    }
}

static void k_prev_word(key_t key)
{
    if (!modifiers)
    {
        tmp_modifiers_and_keycode(MOD_BIT_LCTRL, KC_LEFT);
    }
    else
    {
        simple_key_press(key, KC_W);
    }
}

static void k_ctrl_x_b(key_t key)
{
    if (!modifiers)
    {
        keycode_send(KC_LCTL, KC_X, -KC_X, -KC_LCTL, KC_B, -KC_B);
    }
}

// tilde on FI keyboard: dead tilde plus space

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

// special handling for shift-4 to produce $ on FI keyboard

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

// mouse directions or scroll wheel

static void k_mouse_dir_or_wheel(key_t key, keycode_t dir, keycode_t wheel)
{
  if (modifiers == MOD_BIT_LALT)
    {
      tmp_keycode(-KC_LALT, wheel);
    }
  else
    {
      simple_key_press(key, dir);
    }
}

static void k_ms_u(key_t key)
{
  k_mouse_dir_or_wheel(key, KC_MS_U, KC_WH_U);
}

static void k_ms_l(key_t key)
{
  k_mouse_dir_or_wheel(key, KC_MS_L, KC_WH_L);
}

static void k_ms_d(key_t key)
{
  k_mouse_dir_or_wheel(key, KC_MS_D, KC_WH_D);
}

static void k_ms_r(key_t key)
{
  k_mouse_dir_or_wheel(key, KC_MS_R, KC_WH_R);
}

// teams mute tap(toggle)/hold(momentary) on/off

#define TEAMS_TOGGLE_TIMEOUT 500

static struct teams_mute_tap_state
{
  uint16_t key_down;
  key_t key;
} teams_mute_tap_state;

static bool teams_mute_handler(key_t key, bool pressed)
{
    if (key != KEY_NO)
    {
        keycode_send(-KC_SPC, -KC_LCTL);
        remove_tmp_handler(teams_mute_handler);
        if (key == teams_mute_tap_state.key &&
            timer_diff(timer_read(), teams_mute_tap_state.key_down) <
            TEAMS_TOGGLE_TIMEOUT)
        {
            for (uint16_t timer_start = timer_read();
                 timer_diff(timer_read(), timer_start) < 100;)
            {
            }
            keycode_send(KC_LSFT, KC_LCTL, KC_M, -KC_M, -KC_LCTL, -KC_LSFT);
            return true;
        }
    }
    return false;
}

static void k_teams_mute(key_t key)
{
    if (modifiers != 0)
    {
        return;
    }
    keycode_send(KC_LCTL, KC_SPC);
    teams_mute_tap_state.key = key;
    teams_mute_tap_state.key_down = timer_read();
    add_tmp_handler(teams_mute_handler);
}

// mechanical shift lock: kind of caps lock when tapping both shifts
//   ...except that numbers are kept without shift

#define SHIFT_ALLOWED_JITTER 500

static struct shift_state
{
    bool lock;
    bool second_seen;
    bool other_seen;
    uint8_t layer;
    uint8_t count;
    uint16_t start_time;
} shift_state;

static void k_lsft(key_t key);
static void k_rsft(key_t key);

static bool shift_key_handler(key_t key, bool pressed)
{
    if (key != KEY_NO && pressed &&
        keymap[shift_state.layer][key] != KCFUNC(k_lsft) &&
        keymap[shift_state.layer][key] != KCFUNC(k_rsft))
    {
        shift_state.other_seen = true;
    }
    return false;
}

static void shift_start(void)
{
    shift_state.second_seen = false;
    shift_state.other_seen = false;
    shift_state.layer = layer;
    shift_state.start_time = timer_read();
    add_tmp_handler(shift_key_handler);
}

static bool shift_locked_handler(key_t key, bool pressed)
{
    if (key != KC_NO && pressed)
    {
        mod_bits_t shifts = MOD_BIT_LSHIFT | MOD_BIT_RSHIFT;
        keycode_t code;

        if (modifiers & ~shifts)
        {
            return false;
        }
        code = keymap[shift_state.layer][key];
        if (code == KCFUNC(k_four_dollar))
        {
            code = KC_4;
        }
        if (code >= KC_1 && code <= KC_0)
        {
            tmp_keycode(-KC_LSFT, -KC_RSFT, code);
            return true;
        }
    }
    return false;
}

static void shift_finish(void)
{
    if (!shift_state.other_seen && shift_state.second_seen &&
        timer_diff(timer_read(), shift_state.start_time) < SHIFT_ALLOWED_JITTER)
    {
        shift_state.lock = !shift_state.lock;
        kb_led_set(CAPS_LED, shift_state.lock);
        if (shift_state.lock)
        {
            add_tmp_handler(shift_locked_handler);
        }
        else
        {
            keycode_send(-KC_LSFT, -KC_RSFT);
            remove_tmp_handler(shift_locked_handler);
        }
    }
    remove_tmp_handler(shift_key_handler);
}

static void shift_down(key_t key, keycode_t kc, void (*up_func)(key_t key))
{
    key_release_info[key] = KCFUNC(up_func);
    if (!shift_state.count++)
    {
        shift_start();
    }
    else
    {
        shift_state.second_seen = true;
    }
    keycode_send(kc);
}

static void shift_up(keycode_t code)
{
    if (!--shift_state.count)
    {
        shift_finish();
    }
    if (!shift_state.lock)
    {
        keycode_send(-code);
    }
}

static void k_lsft_release(key_t key) {shift_up(KC_LSFT);}
static void k_rsft_release(key_t key) {shift_up(KC_RSFT);}
static void k_lsft(key_t key) {shift_down(key, KC_LSFT, k_lsft_release);}
static void k_rsft(key_t key) {shift_down(key, KC_RSFT, k_rsft_release);}

// layer switch from BASE to NAVIGATION
//   tap: switch layer
//   hold: switch layer momentarily

#define LAYER_SWITCH_TO_HOLD 500

static struct layer_switch_state
{
    uint16_t timer_start;
    bool hold_enabled;
    key_t key;
    enum { layer_switch_key, layer_switch_hold } state;
} layer_switch_state;

static bool layer_switch_handler(key_t key, bool pressed);

static bool layer_switch_event_key(key_t key, bool pressed)
{
    if (key == layer_switch_state.key) // !pressed
    {
        if (layer_switch_state.hold_enabled)
        {
            // already switched the layer
        }
        else
        {
            layer = !layer;
            kb_led_set(LAYER_LED, layer);
        }
        remove_tmp_handler(layer_switch_handler);
        return true;
    }
    if (key != KEY_NO && pressed)
    {
        if (layer_switch_state.hold_enabled)
        {
            layer_switch_state.state = layer_switch_hold;
        }
        else
        {
            remove_tmp_handler(layer_switch_handler);
        }
        return false;
    }
    if (key == KEY_NO && layer_switch_state.hold_enabled)
    {
        if (timer_diff(timer_read(), layer_switch_state.timer_start) >
            LAYER_SWITCH_TO_HOLD)
        {
            layer_switch_state.state = layer_switch_hold;
        }
        return true;
    }
    return false;
}

static bool layer_switch_event_hold(key_t key, bool pressed)
{
    if (key == layer_switch_state.key) // !pressed
    {
        layer = !layer;
        kb_led_set(LAYER_LED, layer);
        remove_tmp_handler(layer_switch_handler);
        return true;
    }
    return false;
}

static bool layer_switch_handler(key_t key, bool pressed)
{
    switch (layer_switch_state.state)
    {
    case layer_switch_key:
        return layer_switch_event_key(key, pressed);
    case layer_switch_hold:
        return layer_switch_event_hold(key, pressed);
    default:
        return false;
    }
}

static void layer_switch_start(key_t key,
                               bool hold_enabled)
{
    layer_switch_state.timer_start = timer_read();
    layer_switch_state.key = key;
    layer_switch_state.hold_enabled = hold_enabled;
    layer_switch_state.state = layer_switch_key;
    if (layer_switch_state.hold_enabled)
    {
        layer = !layer;
        kb_led_set(LAYER_LED, layer);
    }
    add_tmp_handler(layer_switch_handler);
}

static void k_navigation_layer_on(key_t key)
{
    layer_switch_start(key, true);
}

// layer switch from NAVIGATION to BASE
//   tap: switch layer
//   hold: -

static void k_navigation_layer_off(key_t key)
{
    layer_switch_start(key, false);
}

// KC_RCTL/FI_ADIA support
//   BASE layer:
//     tap: FI_ADIA
//     hold: KC_RCTL
//   NAVIGATION layer:
//     KC_RCTL

#define TAP_HOLD_TIMEOUT 200
#define TAP_MYSTERY_TIMEOUT 50
#define TAP_TIE_TIMEOUT 10

struct tap_state
{
    tmp_handler_t handler;
    uint16_t timer_start;
    uint16_t key_down;
    uint16_t mystery_key_down;
    keycode_t tap;
    keycode_t hold;
    key_t key;
    key_t mystery;
    enum { tap_key, tap_hold, tap_key_mystery } state;
};

static void tap_start(struct tap_state *state,
                      key_t key,
                      keycode_t tap,
                      keycode_t hold,
                      tmp_handler_t handler)
{
    state->handler = handler;
    state->timer_start = timer_read();
    state->key_down = state->timer_start;
    state->tap = tap;
    state->hold = hold;
    state->key = key;
    state->state = tap_key;
    add_tmp_handler(handler);
    keycode_send(state->hold);
    key_release_info[state->key] = KC_NO;
}

static bool tap_event_key(struct tap_state *state, key_t key, bool pressed)
{
    if (key == state->key) // !pressed
    {
        keycode_send(-state->hold, state->tap, -state->tap);
        remove_tmp_handler(state->handler);
        return true;
    }
    if (key != KEY_NO && pressed)
    {
        state->state = tap_key_mystery;
        state->mystery = key;
        state->timer_start = timer_read();
        state->mystery_key_down = state->timer_start;
        return true;
    }
    if (key == KEY_NO)
    {
        if (timer_diff(timer_read(), state->timer_start) > TAP_HOLD_TIMEOUT)
        {
            state->state = tap_hold;
        }
        return true;
    }
    return false;
}

static bool tap_event_hold(struct tap_state *state, key_t key, bool pressed)
{
    if (key == state->key) // !pressed
    {
        keycode_send(-state->hold);
        remove_tmp_handler(state->handler);
        return true;
    }
    return false;
}

static bool tap_event_key_mystery(struct tap_state *state, key_t key, bool pressed)
{
    if (key == state->mystery) // !pressed
    {
        key_press(state->mystery);
        key_release(state->mystery);
        state->state = tap_hold;
        return true;
    }
    if (key == state->key) // !pressed
    {
        if (timer_diff(timer_read(), state->mystery_key_down) > TAP_TIE_TIMEOUT)
        {
            key_press(state->mystery);
            keycode_send(-state->hold);
        }
        else
        {
            keycode_send(-state->hold);
            keycode_send(state->tap);
            key_press(state->mystery);
            keycode_send(-state->tap);
        }
        remove_tmp_handler(state->handler);
        return true;
    }
    if (key != KEY_NO && pressed)
    {
        key_press(state->mystery);
        key_press(key);
        state->state = tap_hold;
        return true;
    }
    if (key == KEY_NO)
    {
        if (timer_diff(timer_read(), state->timer_start) > TAP_MYSTERY_TIMEOUT)
        {
            key_press(state->mystery);
            state->state = tap_hold;
        }
        return true;
    }
    return false;
}

static bool tap_event(struct tap_state *state, key_t key, bool pressed)
{
    switch (state->state)
    {
    case tap_key:
        return tap_event_key(state, key, pressed);
    case tap_hold:
        return tap_event_hold(state, key, pressed);
    case tap_key_mystery:
        return tap_event_key_mystery(state, key, pressed);
    default:
        return false;
    }
}

static struct tap_state rctl_adia_state;

static bool rctl_adia_handler(key_t key, bool pressed)
{
    return tap_event(&rctl_adia_state, key, pressed);
}

static void k_rctl_adia(key_t key)
{
    if (layer != BASE)
    {
        simple_key_press(key, KC_RCTL);
        return;
    }
    tap_start(&rctl_adia_state, key, FI_ADIA, KC_RCTL, rctl_adia_handler);
}

/* The keymap */

static const intptr_t keymap[][KEY_COUNT] = {
    /* BASE */
    {
        KC_ESC, KC_1, KC_2, KC_3, KCFUNC(k_four_dollar), KC_5, FI_DIAE, FI_ACUT, KC_6, KC_7, KC_8, KC_9, KC_0, FI_PLUS,

        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KCFUNC(k_brace_left), KCFUNC(k_brace_right), KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSLS,

        KC_LCTL, KC_A, KC_S, KC_D, KC_F, KC_G, KC_NO, KC_NO, KC_H, KC_J, KC_K, KC_L, FI_ODIA, KCFUNC(k_rctl_adia),

        KCFUNC(k_lsft), KC_Z, KC_X, KC_C, KC_V, KC_B, KCFUNC(k_backslash), KCFUNC(k_tilde), KC_N, KC_M, FI_COMM, FI_DOT, FI_MINS, KCFUNC(k_rsft),

        FI_SECT, KCFUNC(k_pipe), FI_LABK, KCFUNC(k_greater_than), KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KCFUNC(k_navigation_layer_on), KCFUNC(k_lbracket), KCFUNC(k_rbracket), KCFUNC(k_ad), KCFUNC(k_dead_tilde),

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, FI_ARNG, KCFUNC(k_teams_mute), KC_MUTE, KC_RALT, KC_RGUI, KC_NO, KC_SPC, KCFUNC(k_ctrl_x_b), KC_BSPC, KC_NO,
    },

    /* NAVIGATION */
    {
        KC_ESC, KC_F1, KC_F2, KC_F3, KC_F4, KC_F5, KC_NO, KC_NO, KC_F6, KC_F7, KC_F8, KC_F9, KC_F10, KC_F11,

        KC_TAB, KC_PGUP, KCFUNC(k_prev_word), KC_UP, KCFUNC(k_next_word), KC_NO, KCFUNC(k_brace_left), KCFUNC(k_brace_right), KC_Y, KC_BTN2, KCFUNC(k_ms_u), KC_BTN1, KC_NO, KC_F12,

        KC_LCTL, KC_HOME, KC_LEFT, KC_DOWN, KC_RGHT, KC_END, KC_NO, KC_NO, KC_WBAK, KCFUNC(k_ms_l), KCFUNC(k_ms_d), KCFUNC(k_ms_r), KC_WFWD, KC_RCTL,

        KCFUNC(k_lsft), KC_NO, KC_NO, KC_NO, KC_PGDN, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_MPLY, KC_NO, FI_MINS, KCFUNC(k_rsft),

        KC_NO, KC_BRK, KC_INS, KC_NO, KC_ENT, KC_NO, KC_NO, KC_NO, KC_NO, KCFUNC(k_navigation_layer_off), KC_VOLD, KC_VOLU, KC_MPRV, KC_MNXT,

        KC_NO, KC_DEL, KC_LGUI, KC_LALT, KC_PSCR, KC_NO, KC_MUTE, KC_RALT, KC_RGUI, KC_NO, KC_SPC, KC_NO, KC_BSPC, KC_NO,
    },
};

/* Event handling (replaces qmk's default event handling */

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

// hook early into qmk's event handling

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

/* Dummy definitions required by qmk build system, not used by us */

const uint16_t keymaps[0][MATRIX_ROWS][MATRIX_COLS];
