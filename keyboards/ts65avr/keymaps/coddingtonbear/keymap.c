#include "ts65avr.h"
#include QMK_KEYBOARD_H
#include "analog.h"
#include "pointing_device.h"
#include "print.h"

#define _______ KC_TRNS

#define _QWERTY 0
#define _FUNC 1
#define _FUNC2 2
#define _MOUSE 3
#define _EMOJI 4
#define _GAME 5
#define _BLANK 6

// Key exists electrically, but is unpopulated in selected layout
#define KC_NONE KC_NO

#define MO_FUNC MO(_FUNC)
#define MO_FNC2 MO(_FUNC2)
#define MO_MSE MO(_MOUSE)
#define LT_EMOJI LT(_EMOJI, KC_PPLS)
#define LT_MSE LT(_MOUSE, KC_SCLN)
#define TG_GAME TG(_GAME)
#define KC_CHOM RCTL(KC_HOME)
#define KC_CEND RCTL(KC_END)

#define ENABLE_PSP1000_MOUSE
#define MOUSE_MOVE_UNIT 10
#define MOUSE_MOVE_FINE_UNIT 2
#define MOUSE_NULL_ZONE 0.
#define X_MOUSE_PIN PF0
#define Y_MOUSE_PIN PF1
#define Y_MOUSE_INVERT
//#define X_MOUSE_INVERT

int yMouseNull = 0;
int xMouseNull = 0;
int yMouseMin = 0;
int xMouseMin = 0;
int yMouseMax = 0;
int xMouseMax = 0;
int xMouseLast = 0;
int yMouseLast = 0;
int mouseMoveUnit = MOUSE_MOVE_UNIT;

enum custom_keycodes {
  PLUS_1 = SAFE_RANGE,
  MINUS_1,
  HEART,
  SHRUG,
  SAME
};

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
// Mux input
static uint8_t aref = (1<<REFS0); // default to AREF = Vcc
int16_t adc_read(uint8_t mux)
{
#if defined(__AVR_AT90USB162__)
  return 0;
#else
  uint8_t low;

  ADCSRA = (1<<ADEN) | ADC_PRESCALER;		// enable ADC
  ADCSRB = (1<<ADHSM) | (mux & 0x20);		// high speed mode
  ADMUX = aref | (mux & 0x1F);			// configure mux input
  ADCSRA = (1<<ADEN) | ADC_PRESCALER | (1<<ADSC);	// start the conversion
  while (ADCSRA & (1<<ADSC)) ;			// wait for result
  low = ADCL;					// must read LSB first
  return (ADCH << 8) | low;			// must read MSB only once!
#endif
}

// Arduino compatible pin input
int16_t analogRead(uint8_t pin)
{
#if defined(__AVR_ATmega32U4__)
  static const uint8_t PROGMEM pin_to_mux[] = {
    0x00, 0x01, 0x04, 0x05, 0x06, 0x07,
    0x25, 0x24, 0x23, 0x22, 0x21, 0x20};
  if (pin >= 12) return 0;
  return adc_read(pgm_read_byte(pin_to_mux + pin));
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
  if (pin >= 8) return 0;
  return adc_read(pin);
#else
  return 0;
#endif
}

/*
enum unicode_mode {
  UC_LNX,
  UC_WIN,
  UC_MAC,
} currentUnicodeMode;

inline unicode_mode &operator++(unicode_mode &d) {
  if(d == 2) {
    return d = unicode_mode(0);
  }
  return d = unicode_mode(d + 1);
};*/

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* 0: qwerty */
    [_QWERTY] = KEYMAP( \
        /* <------------- LEFT SIDE ------------------------------->  <------------------------- RIGHT SIDE ----------------------------------------> */
        /* 1     2        3        4         5        6       7        8       9         10       11       12       13       14       15       16 */
        KC_GESC, KC_1,    KC_2,    KC_3,     KC_4,    KC_5,   KC_6,    KC_7,   KC_8,     KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_NONE, KC_BSPC, KC_DEL,  \
        KC_TAB,  KC_Q,    KC_W,    KC_E,     KC_R,    KC_T,            KC_Y,   KC_U,     KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_PGUP, \
        KC_LCTL, KC_A,    KC_S,    KC_D,     KC_F,    KC_G,            KC_H,   KC_J,     KC_K,    KC_L,    LT_MSE,  KC_QUOT, KC_NONE, KC_ENT,  KC_PGDN, \
        KC_LSFT, KC_NONE, KC_Z,    KC_X,     KC_C,    KC_V,   KC_B,    KC_N,   KC_M,     KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   LT_EMOJI, \
        KC_LCTL, MO_FUNC, KC_LGUI, KC_LALT,  KC_SPC,                   KC_SPC,           KC_RALT, MO_FNC2, KC_RCTL, KC_LEFT,          KC_DOWN, KC_RIGHT),

    [_FUNC] = KEYMAP( \
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______, _______, KC_MUTE, \
        DEBUG,   _______, _______, _______, _______, _______,          _______, _______, KC_PGUP, _______, _______, _______, _______, _______, KC_VOLU, \
        RESET,   _______, _______, _______, _______, TG_GAME,          KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, KC_GRV,  _______, _______, KC_VOLD, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_PGDN, _______, UC(L'‽'),_______,          _______, _______, \
        _______, _______, _______, _______, _______,                   _______,          _______, _______, _______, KC_MPRV,          KC_MPLY, KC_MNXT),

    [_FUNC2] = KEYMAP( \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______,\
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          KC_CHOM, _______, \
        _______, _______, _______, _______, _______,                   _______,          _______, _______, _______, KC_HOME,          KC_CEND, KC_END ),

    [_MOUSE] = KEYMAP( \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, KC_WH_U, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, KC_BTN1, KC_BTN2, KC_BTN3, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_WH_D, _______, _______, _______,          KC_WH_U, _______, \
        _______, _______, _______, KC_BTN2, KC_BTN1,                   KC_BTN1,          KC_BTN2, _______, _______, _______,          KC_WH_D, _______),

    [_EMOJI] = KEYMAP( \
        RESET,   UC(L'①'), UC(L'②'), UC(L'③'), UC(L'④'), UC(L'⑤'), UC(L'⑥'), UC(L'⑦'), UC(L'⑧'), UC(L'⑨'), UC(L'⑩'), _______, _______, _______, _______, _______, \
        DEBUG,   _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, SAME, _______, \
        _______, _______, SHRUG, _______, _______, _______,          HEART,   _______, _______, _______, _______, _______, _______, _______, _______,\
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          PLUS_1,  _______, \
        _______, _______, _______, _______, _______,                   _______,          _______, _______, _______, _______,          MINUS_1, _______),

    [_GAME] = KEYMAP( \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______,\
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, KC_LALT, KC_SPC,  _______,                   _______,          KC_LGUI, _______, _______, _______,          _______, _______),

    [_BLANK] = KEYMAP( \
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, \
        _______, _______, _______, _______, _______, _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______,\
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, \
        _______, _______, _______, _______, _______,                   _______,          _______, _______, _______, _______,          _______, _______)
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
      case HEART:
        SEND_STRING(":heart:");
        return false;
      case SHRUG:
        SEND_STRING(":shrug:");
        return false;
      case SAME:
        SEND_STRING(":same:");
        return false;
    }
  } else if (!record->event.pressed) {
    switch(keycode) {
    }
  }
  return true;
}


uint32_t layer_state_set_user(uint32_t state) {
  switch(biton32(state)) {
    case _MOUSE:
      // Slow the mouse down
      mouseMoveUnit = MOUSE_MOVE_FINE_UNIT;
      break;
    default:
      // Speed the mouse up
      mouseMoveUnit = MOUSE_MOVE_UNIT;
  }

  return state;
}

void matrix_init_user(void) {
  set_unicode_input_mode(UC_LNX);
}

void matrix_scan_user(void) {
#ifdef ENABLE_PSP1000_MOUSE
  int xMouse = analogRead(X_MOUSE_PIN);
  int yMouse = analogRead(Y_MOUSE_PIN);
  report_mouse_t currentReport = pointing_device_get_report();

  if(yMouseNull == 0) {
    yMouseNull = yMouse;
    yMouseMin = yMouse;
    yMouseMax = yMouse;
  }
  if(xMouseNull == 0) {
    xMouseNull = xMouse;
    xMouseMin = xMouse;
    xMouseMax = xMouse;
  }

  int xMouseMaxRange = xMouseMax - xMouseNull;
  int xMouseMinRange = xMouseNull - xMouseMin;
  int yMouseMaxRange = yMouseMax - yMouseNull;
  int yMouseMinRange = yMouseNull - yMouseMin;

  if(yMouse < (yMouseNull - (yMouseMinRange * MOUSE_NULL_ZONE))) {
    if(yMouse < yMouseMin) {
      yMouseMin = yMouse;
    }
    xprintf("X: %d\t[%d, %d]\tY: %d\t[%d, %d]\t\t(Null: %d, %d)\n", xMouse, xMouseMin, xMouseMax, yMouse, yMouseMin, yMouseMax, xMouseNull, yMouseNull);
    int value = round(pow((float)(yMouseNull - yMouse) / yMouseMinRange, 2) * mouseMoveUnit);
    #ifdef Y_MOUSE_INVERT
      value = value * -1;
    #endif
    xprintf("Move Y: %d\n", value);
    currentReport.y = value;
  }
  if(yMouse > (yMouseNull + (yMouseMaxRange * MOUSE_NULL_ZONE))) {
    if(yMouse > yMouseMax) {
      yMouseMax = yMouse;
    }
    xprintf("X: %d\t[%d, %d]\tY: %d\t[%d, %d]\t\t(Null: %d, %d)\n", xMouse, xMouseMin, xMouseMax, yMouse, yMouseMin, yMouseMax, xMouseNull, yMouseNull);
    int value = round(pow((float)(yMouse - yMouseNull) / yMouseMaxRange, 2) * mouseMoveUnit);
    #ifndef Y_MOUSE_INVERT
      value = value * -1;
    #endif
    xprintf("Move Y: %d\n", value);
    currentReport.y = value;
  }
  if(xMouse < (xMouseNull - (xMouseMinRange * MOUSE_NULL_ZONE))) {
    if(xMouse < xMouseMin) {
      xMouseMin = xMouse;
    }
    xprintf("X: %d\t[%d, %d]\tY: %d\t[%d, %d]\t\t(Null: %d, %d)\n", xMouse, xMouseMin, xMouseMax, yMouse, yMouseMin, yMouseMax, xMouseNull, yMouseNull);
    int value = round(pow((float)(xMouseNull - xMouse) / xMouseMinRange, 2) * mouseMoveUnit);
    #ifdef X_MOUSE_INVERT
      value = value * -1;
    #endif
    xprintf("Move X: %d\n", value);
    currentReport.x = value;
  }
  if(xMouse > (xMouseNull + (xMouseMaxRange * MOUSE_NULL_ZONE))) {
    if(xMouse > xMouseMax) {
      xMouseMax = xMouse;
    }
    xprintf("X: %d\t[%d, %d]\tY: %d\t[%d, %d]\t\t(Null: %d, %d)\n", xMouse, xMouseMin, xMouseMax, yMouse, yMouseMin, yMouseMax, xMouseNull, yMouseNull);
    int value = round(pow((float)(xMouse - xMouseNull) / xMouseMaxRange, 2) * mouseMoveUnit);
    #ifndef X_MOUSE_INVERT
      value = value * -1;
    #endif
    xprintf("Move X: %d\n", value);
    currentReport.x = value;
  }
  pointing_device_set_report(currentReport);
#endif
}
