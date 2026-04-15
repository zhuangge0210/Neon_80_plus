// Copyright 2022 AngryMiao 
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "keycode.h"
//enum custom_layers {
//    _BASE,
//};
typedef enum custom_keycodes {
    TEST_KEY=0x5DB1,//0x5DB1  not use
    KEY_LED_NEXT,
    KEY_LED_PREV,
    KEY_LED_SPEED_INC,
    KEY_LED_SPEED_DEC,
    KEY_LED_LIGHT_INS,
    KEY_LED_LIGHT_DEC,
	HEAD_LED_NEXT,
	HEAD_LED_PREV,
    HEAD_LED_SPEED_INC,
    HEAD_LED_SPEED_DEC,
    HEAD_LED_LIGHT_INS,
    HEAD_LED_LIGHT_DEC,
    HEAD_LED_POWER,
    KEY_LED_POWER,
}custom_keycodes_t;



const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Base */
  [0] = LAYOUT_iso(
    /*  0      1        2        3        4        5         6        7       8        9         10      11          12         13         14    */
    KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,     KC_F12,  KC_F13, KC_PSCR,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,    KC_EQL,   KC_BSPC, KC_INS,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,    KC_RBRC,  KC_BSLS ,KC_DEL,
    KC_CAPS, KC_A,KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,    KC_ENTER, KC_PAUS,  KC_SCRL,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  KC_RSFT,   KC_PGDN ,KC_PGUP,   KC_HOME,
    KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,  _______,  _______, KC_RGHT,  KC_DOWN, KC_LEFT, _______, MO(1), KC_RGUI,   KC_UP,    KC_RCTL ,   KC_END
  ),
  [1] = LAYOUT_iso(
    /*  0      1        2        3        4        5         6        7       8        9         10      11          12         13         14     */
    KC_ESC,  KC_AUDIO_MUTE,   KC_AUDIO_VOL_DOWN,   KC_AUDIO_VOL_UP,   KC_MEDIA_PREV_TRACK,   KC_MEDIA_NEXT_TRACK,   KC_MEDIA_PLAY_PAUSE,   KC_F7,   KC_F8,   KEY_LED_POWER,   HEAD_LED_POWER,  KEY_LED_NEXT,HEAD_LED_NEXT,  KC_MUTE, KC_PSCR,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KEY_LED_LIGHT_DEC,    KEY_LED_LIGHT_INS,   KC_BSPC, KC_INS,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,    KC_RBRC,  KC_BSLS ,KC_DEL,
    KC_CAPS, KC_A,KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,    KC_ENTER, KC_PAUS,  KC_SCRL,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  KC_RSFT,   KC_PGDN ,KC_PGUP,   KC_HOME,
    KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,  _______,  _______, HEAD_LED_SPEED_INC,  HEAD_LED_LIGHT_DEC, HEAD_LED_SPEED_DEC, _______, KC_RALT, KC_RGUI,   HEAD_LED_LIGHT_INS,    KC_RCTL ,   KC_END
  ),
  [2] = LAYOUT_iso(
    /*  0      1        2        3        4        5         6        7       8        9         10      11          12         13         14     */
    KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,     KC_F12,  KC_F13, KC_PSCR,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,    KC_EQL,   KC_BSPC, KC_INS,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,    KC_RBRC,  KC_BSLS ,KC_DEL,
    KC_CAPS, KC_A,KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,    KC_ENTER, KC_PAUS,  KC_SCRL,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  KC_RSFT,   KC_PGDN ,KC_PGUP,   KC_HOME,
    KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,  _______,  _______, KC_RGHT,  KC_DOWN, KC_LEFT, _______, MO(1), KC_RGUI,   KC_UP,    KC_RCTL ,   KC_END
  ),
  [3] = LAYOUT_iso(
    /*  0      1        2        3        4        5         6        7       8        9         10      11          12         13         14      */
    KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,     KC_F12,  KC_F13, KC_PSCR,
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS,    KC_EQL,   KC_BSPC, KC_INS,
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC,    KC_RBRC,  KC_BSLS ,KC_DEL,
    KC_CAPS, KC_A,KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,    KC_ENTER, KC_PAUS,  KC_SCRL,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  KC_RSFT,   KC_PGDN ,KC_PGUP,   KC_HOME,
    KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,  _______,  _______, KC_RGHT,  KC_DOWN, KC_LEFT, _______, MO(1), KC_RGUI,   KC_UP,    KC_RCTL ,   KC_END
  ),
};



//   [2] = LAYOUT_iso(
//     /*  0      1        2        3        4        5         6        7       8        9         10      11          12         13         14     */
//     KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,     KC_F12,  KC_MUTE, KC_PSCR,
//     KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    HEAD_LED_PREV,    HEAD_LED_NEXT,   KC_BSPC, KC_INS,
//     KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KEY_LED_PREV,    KEY_LED_NEXT,  KC_NUHS ,HEAD_LED_SPEED_DEC,
//     KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,    KC_ENTER, KC_PAUS,  KC_SCRL,
//     KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,  KC_RSFT,   HEAD_LED_SPEED_INC ,KC_PGUP,   HEAD_LED_LIGHT_INS,
//     KC_LCTL, KC_LGUI, KC_LALT, KC_SPC,  _______,  _______, KEY_LED_SPEED_INC,  KEY_LED_LIGHT_DEC, KEY_LED_SPEED_DEC, _______, KC_RALT, KC_RGUI,   KEY_LED_LIGHT_INS,    KC_RCTL ,   HEAD_LED_LIGHT_DEC
//   ),
