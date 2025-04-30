/*
Copyright 2023 @ Nuphy <https://nuphy.com/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // layer Mac
    [0] = LAYOUT_75_ansi(
        KC_ESC,     KC_F1,      KC_F2,      KC_F3,     KC_F4,      KC_F5,       KC_MPLY,   KC_MUTE,    KC_VOLD,     KC_VOLU,   KC_F10,     KC_F11,     KC_F12,      _______,    _______,    _______,
        KC_GRV,     KC_1,      KC_2,        KC_3,      KC_4,       KC_5,        KC_6,      KC_7,       KC_8,        KC_9,      KC_0,       KC_MINS,    KC_EQL,      KC_BSPC,                _______,
        KC_TAB,     KC_Q,      KC_W,        KC_E,      KC_R,       KC_T,        KC_Y,      KC_U,       KC_I,        KC_O,      KC_P,       KC_LBRC,    KC_RBRC,                 KC_BSLS,    _______,
        KC_LGUI,    KC_A,      KC_S,        KC_D,      KC_F,       KC_G,        KC_H,      KC_J,       KC_K,        KC_L,      KC_SCLN,    KC_QUOT,                             KC_ENT,     _______,
        KC_LSFT,               KC_Z,        KC_X,      KC_C,       KC_V,        KC_B,      KC_N,       KC_M,        KC_COMM,   KC_DOT,     KC_SLSH,                 KC_RSFT,    KC_UP,      _______,
        KC_LCTL,    KC_LALT,   KC_LGUI,                                         KC_SPC,                                        KC_RGUI,    KC_RALT,    MO(1),       KC_LEFT,    KC_DOWN,    KC_RGHT
    ),

    // layer Mac Fn
    [1] = LAYOUT_75_ansi(
        _______,    KC_F1,      KC_F2,      KC_F3,      KC_F4,      KC_F5,       KC_F6,     KC_F7,      KC_F8,       KC_F9,     KC_F10,     KC_F11,     KC_F12,      _______,   _______,    _______,
        _______,    LNK_BLE1,   LNK_BLE2,   LNK_BLE3,   LNK_RF,     _______,     _______,   _______,    _______,     _______,   _______,    _______,	_______,     KC_DEL,                _______,
        _______,    _______,    _______,    _______,    _______,    _______,     RGB_SPI,   RGB_RMOD,   RGB_VAI,     RGB_HUI,   RGB_SAI,    _______,    _______,                BAT_NUM,    _______,
        KC_CAPS,    _______,    _______,    _______,    _______,    _______,     RGB_SPD,   RGB_RMOD,   RGB_VAD,     RGB_HUD,   RGB_SAD,    _______,                            _______,    _______,
        _______,                _______,    _______,    _______,    _______,     BAT_SHOW,  _______,    _______,     _______,   _______,    _______,                 _______,   _______,    _______,
        _______,    _______,    _______,                                         _______,                                       _______,    _______,    _______,     _______,   _______,    _______
    ),

    // layer win
    [8] = LAYOUT_75_ansi(
        KC_ESC,     KC_F1,      KC_F2,      KC_F3,     KC_F4,      KC_F5,       KC_MPLY,   KC_MUTE,    KC_VOLD,     KC_VOLU,   KC_F10,     KC_F11,     KC_F12,      _______,    _______,    _______,
        KC_GRV,     KC_1,      KC_2,        KC_3,      KC_4,       KC_5,        KC_6,      KC_7,       KC_8,        KC_9,      KC_0,       KC_MINS,    KC_EQL,      KC_BSPC,                _______,
        KC_TAB,     KC_Q,      KC_W,        KC_E,      KC_R,       KC_T,        KC_Y,      KC_U,       KC_I,        KC_O,      KC_P,       KC_LBRC,    KC_RBRC,                 KC_BSLS,    _______,
        KC_LCTL,    KC_A,      KC_S,        KC_D,      KC_F,       KC_G,        KC_H,      KC_J,       KC_K,        KC_L,      KC_SCLN,    KC_QUOT,                             KC_ENT,     _______,
        KC_LSFT,               KC_Z,        KC_X,      KC_C,       KC_V,        KC_B,      KC_N,       KC_M,        KC_COMM,   KC_DOT,     KC_SLSH,                 KC_RSFT,    KC_UP,      _______,
        KC_LCTL,    KC_LGUI,   KC_LALT,                                         KC_SPC,                                        KC_RALT,    KC_RCTL,    MO(6),       KC_LEFT,    KC_DOWN,    KC_RGHT
    ),

    // layer win Fn
    [9] = LAYOUT_75_ansi(
        _______,    KC_F1,      KC_F2,      KC_F3,      KC_F4,      KC_F5,       KC_F6,     KC_F7,      KC_F8,       KC_F9,     KC_F10,     KC_F11,     KC_F12,      _______,   _______,    _______,
        _______,    LNK_BLE1,   LNK_BLE2,   LNK_BLE3,   LNK_RF,     _______,     _______,   _______,    _______,     _______,   _______,    _______,	_______,     KC_DEL,                _______,
        _______,    _______,    _______,    _______,    _______,    _______,     RGB_SPI,   RGB_RMOD,   RGB_VAI,     RGB_HUI,   RGB_SAI,    _______,    _______,                BAT_NUM,    _______,
        KC_CAPS,    _______,    _______,    _______,    _______,    _______,     RGB_SPD,   RGB_RMOD,   RGB_VAD,     RGB_HUD,   RGB_SAD,    _______,                            _______,    _______,
        _______,                _______,    _______,    _______,    _______,     BAT_SHOW,  _______,    _______,     _______,   _______,    _______,                 _______,   _______,    _______,
        _______,    _______,    _______,                                         _______,                                       _______,    _______,    _______,     _______,   _______,    _______
    )

};
