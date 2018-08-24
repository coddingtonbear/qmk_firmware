#include "ts65avr.h"
#include "analog.h"
#include "pointing_device.h"
#include "print.h"
#include QMK_KEYBOARD_H

#define _______ KC_TRNS

#define _QWERTY 0
#define _FUNC 1
#define _MOUSE 2
#define _EMOJI 3

// Key exists electrically, but is unpopulated in selected layout
#define KC_NONE KC_NO
    
#define MO_FUNC MO(_FUNC)
#define MO_MSE MO(_MOUSE)
#define MO_EMOJI MO(_EMOJI)
#define LT_MSE LT(_MOUSE, KC_SCLN)

#define MOUSE_NULL_ZONE 10
#define Y_MOUSE_PIN PF0
#define X_MOUSE_PIN PF1

int yMouseNull = 0;
int xMouseNull = 0;

int yMouseMin = 0;
int xMouseMin = 0;
int yMouseMax = 256;
int xMouseMax = 256;

enum custom_keycodes {
  PLUS_1 = SAFE_RANGE,
  MINUS_1 = SAFE_RANGE + 1
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* 0: qwerty */
    [_QWERTY] = KEYMAP( \
        /* <------------- LEFT SIDE ------------------------------->  <------------------------- RIGHT SIDE ----------------------------------------> */
        /* 1     2        3        4         5        6       7        8       9         10       11       12       13       14       15       16 */
        KC_GESC, KC_1,    KC_2,    KC_3,     KC_4,    KC_5,   KC_6,    KC_7,   KC_8,     KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_NONE, KC_BSPC, KC_DEL,  \
        KC_TAB,  KC_Q,    KC_W,    KC_E,     KC_R,    KC_T,            KC_Y,   KC_U,     KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP, \
        KC_CAPS, KC_A,    KC_S,    KC_D,     KC_F,    KC_G,            KC_H,   KC_J,     KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_NONE, KC_ENT,  KC_PGDN, \
        KC_LSFT, KC_NONE, KC_Z,    KC_X,     KC_C,    KC_V,   KC_B,    KC_N,   KC_M,     KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   MO_EMOJI, \
        MO_FUNC, KC_LCTL, KC_LGUI, KC_LALT,  KC_SPC,                   KC_SPC,           KC_RGUI, KC_RALT, KC_RCTL, KC_LEFT,          KC_DOWN, KC_RIGHT),

    [_FUNC] = KEYMAP( \
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, KC_SLEP, KC_MUTE, \
        DEBUG,   _______, _______, _______, _______, _______,          _______, _______, KC_PGUP, _______, _______, _______, _______, _______, KC_VOLU, \
        RESET,   _______, _______, _______, _______, _______,          KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______, _______, _______, KC_VOLD, \
        _______, _______, _______, _______, _______, _______, _______, _______, KC_PGDN, _______, _______, UC(L'‽'),_______,          _______, _______, \
        _______, _______, _______, _______, _______,                   _______,          _______, _______, _______, KC_MPRV,          KC_MPLY, KC_MNXT),

    [_MOUSE] = KEYMAP( \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, KC_WH_U, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, KC_BTN1, KC_BTN2, KC_BTN3, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_WH_D, _______, _______, _______,          KC_WH_U, _______, \
        _______, _______, _______, KC_BTN2, KC_BTN1,                   KC_BTN1,          KC_BTN2, _______, _______, _______,          KC_WH_D, _______),

    [_EMOJI] = KEYMAP( \
        RESET,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        DEBUG,   _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______,\
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          PLUS_1,  _______, \
        _______, _______, _______, _______, _______,                   _______,          _______, _______, _______, _______,          MINUS_1, _______)
};
const uint16_t PROGMEM fn_actions[] = {};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if(record->event.pressed) {
    switch(keycode) {
      case PLUS_1:
        SEND_STRING(":+1:");
        return false;
      case MINUS_1:
        SEND_STRING(":-1:");
        return false;
    }
  }
  return true;
}


void matrix_init_user(void) {
  set_unicode_input_mode(UC_LNX);
}

void matrix_scan_user(void) {
  int xMouse = analogRead(X_MOUSE_PIN);
  int yMouse = analogRead(Y_MOUSE_PIN);
  printf("%d X", xMouse);
  printf("%d Y", yMouse);
  report_mouse_t currentReport = pointing_device_get_report();

  if(yMouseNull == 0) {
    yMouseNull = yMouse;
  }
  if(xMouseNull == 0) {
    xMouseNull = xMouse;
  }

  if(yMouse < (yMouseNull - MOUSE_NULL_ZONE)) {
    if(yMouse < yMouseMin) {
      yMouseMin = yMouse;
    }
    currentReport.y = -1;
  }
  if(yMouse > (yMouseNull + MOUSE_NULL_ZONE)) {
    if(yMouse > yMouseMax) {
      yMouseMax = yMouse;
    }
    currentReport.y = 1;
  }
  if(xMouse < (xMouseNull - MOUSE_NULL_ZONE)) {
    if(xMouse < xMouseMin) {
      xMouseMin = xMouse;
    }
    currentReport.x = -1;
  }
  if(xMouse > (xMouseNull + MOUSE_NULL_ZONE)) {
    if(xMouse > xMouseMax) {
      xMouseMax = xMouse;
    }
    currentReport.x = 1;
  }
  pointing_device_set_report(currentReport);
}