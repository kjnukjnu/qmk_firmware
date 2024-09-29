#include QMK_KEYBOARD_H
#include "version.h"

// required by quantum
const uint16_t PROGMEM keymaps[0][MATRIX_ROWS][MATRIX_COLS];

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record)
{
  register_code(KC_A);
  unregister_code(KC_A);
  return false;
}
