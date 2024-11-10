#include QMK_KEYBOARD_H
#include "version.h"
#include "keymap_finnish.h"

const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS]; // required by quantum, not used by us

#define KEY_COUNT (MATRIX_ROWS * MATRIX_COLS)

enum layers {
    BASE,
    NAVIGATION
};

struct hadron_key
{
    uint8_t keycode;
    void (*func)(uint16_t key, bool down);
};

//static_assert(MATRIX_ROWS == 6);
//static_assert(MATRIX_COLS == 14);

static const struct hadron_key PROGMEM keymap[][KEY_COUNT] = {
    /* BASE */
    {
        {}, {KC_DEL}, {KC_LGUI}, {KC_LALT}, {KC_PSCR}, {}, {}, {KC_RALT}, {KC_RGUI}, {}, {KC_SPC}, {}, {}, {},

        {FI_SECT /* § */}, { /* PIPE: RALT + FI_LABK */}, {FI_LABK /* < */}, { /* >: SHIFT + FI_LABK */}, {KC_ENT}, {}, {}, {}, {}, { /* layer */}, { /* [: RALT + KC_8 */}, { /* ]: RALT + KC_9 */}, { /* @: RALT + KC_2 */}, { /* dead tilde: RALT + KC_RIGHT_BRACKET */},

        {KC_LSFT}, {KC_Z}, {KC_X}, {KC_C}, {KC_V}, {KC_B}, { /* backslash: RALT + KC_MINUS */}, { /* tilde: RALT + KC_RIGHT_BRACKET + KC_SPACE - KC_SPACE */}, {KC_N}, {KC_M}, {FI_COMM}, {FI_DOT}, {FI_MINS}, {KC_RSFT},

        {KC_LCTL}, {KC_A}, {KC_S}, {KC_D}, {KC_F}, {KC_G}, {}, {}, {KC_H}, {KC_J}, {KC_K}, {KC_L}, {FI_ODIA}, { /* ctrl/ä (FI_ADIA)) */},

        {KC_TAB}, {KC_Q}, {KC_W}, {KC_E}, {KC_R}, {KC_T}, { /* {: RALT + KC_7 */}, { /* }: RALT + KC_0 */}, {KC_Y}, {KC_U}, {KC_I}, {KC_O}, {KC_P}, {KC_BSLS},

        {KC_ESC}, {KC_1}, {KC_2}, {KC_3}, {KC_4}, {KC_5}, {FI_DIAE}, {FI_ACUT}, {KC_6}, {KC_7}, {KC_8}, {KC_9}, {KC_0}, {FI_PLUS},
    },

    /* NAVIGATION */
    {
        {}, {KC_DEL}, {KC_LGUI}, {KC_PSCR}, {}, {}, {KC_RALT}, {KC_RGUI}, {}, {KC_SPC}, {}, {}, {}, {},

        {/* MAC */}, {KC_BRK}, {KC_INS}, {}, {KC_ENT}, {}, {}, {}, {}, {/*layer*/}, {KC_VOLD}, {KC_VOLU}, {KC_MPRV}, {KC_MNXT},

        {KC_LSFT}, {}, {}, {}, {KC_PGDN}, {}, {}, {}, {}, {}, {KC_MPLY}, {}, {FI_MINS}, {KC_RSFT},

        {KC_LCTL}, {KC_HOME}, {KC_LEFT}, {KC_DOWN}, {KC_RGHT}, {KC_END}, {}, {}, {KC_WBAK}, {KC_MS_L}, {KC_MS_D}, {KC_MS_R}, {/*ctrl alt*/}, {/*ctrl/Ä (FI_ADIA)*/},

        {KC_TAB}, {KC_PGUP}, {/*prev word*/}, {KC_UP}, {/*next word*/}, {}, {/* {: RALT + KC_7 */}, {/* }: RALT + KC_0 */}, {KC_Y}, {KC_BTN1/*?*/}, {KC_MS_U}, {KC_BTN2/*?*/}, {}, {KC_F12},

        {KC_ESC}, {KC_F1}, {KC_F2}, {KC_F3}, {KC_F4}, {KC_F5}, {}, {}, {KC_F6}, {KC_F7}, {KC_F8}, {KC_F9}, {KC_F10}, {KC_F11},
    },
};

static void hadron_tick_event(void)
{
    static bool time_valid;
    static uint32_t last_change;
    static bool led_1_state = false;
    static bool led_3_state = false;
    uint32_t now = timer_read32(); // ms
    static const uint32_t interval = 1000;
    static uint16_t calls;

    if (!time_valid)
    {
        last_change = now;
        time_valid = true;
        return;
    }
    if (now - last_change >= interval)
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
    if (++calls > 1000)
    {
        led_3_state = !led_3_state;
        if (led_3_state)
        {
            ergodox_right_led_3_on();
        }
        else
        {
            ergodox_right_led_3_off();
        }
        calls = 0;
    }
}

static uint8_t keyup_code[KEY_COUNT];
static unsigned layer = 0;

static void hadron_key_down(uint16_t key)
{
    const struct hadron_key *hkey = &keymap[layer][key];
    if (hkey->func)
    {
        // TODO
    }
    else if (hkey->keycode)
    {
        //register_code(hkey->keycode);
        keyup_code[key] = hkey->keycode;
    }
}

static void hadron_key_up(uint16_t key)
{
    if (keyup_code[key])
    {
        //unregister_code(keyup_code[key]);
        keyup_code[key] = 0;
    }
}

static void show_hex(uint8_t val)
{
    uint8_t code;
    if (val == 0)
    {
        code = KC_0;
    }
    else if (val < 10)
    {
        code = KC_1 + val - 1;
    }
    else
    {
        code = val + KC_A - 10;
    }
    register_code(code);
    unregister_code(code);
}

static void show_value(uint8_t ix)
{
    show_hex(ix >> 4);
    show_hex(ix & 0xF);
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
        uint16_t key = event.key.col + event.key.row * MATRIX_COLS;
        if (event.pressed)
        {
            hadron_key_down(key);
        }
        else
        {
            hadron_key_up(key);
        }
        if (event.pressed)
        {
            register_code(FI_PLUS);
            unregister_code(FI_PLUS);
        }
        else
        {
            register_code(FI_MINS);
            unregister_code(FI_MINS);
        }
        show_value(event.key.row);
        show_value(event.key.col);
    }
    else
    {
        hadron_tick_event();
    }
    return true;
}
