/*
Copyright 2012,2013 Jun Wako <wakojun@gmail.com>

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
#ifndef TS65AVR_H
#define TS65AVR_H

#include "quantum.h"

/* TS65AVR keymap definition macro */
#define KEYMAP( \
    K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, K1C, K1D, K1E, K1F, K1G, \
    K21, K22, K23, K24, K25, K26,      K28, K29, K2A, K2B, K2C, K2D, K2E, K2F, K2G, \
    K31, K32, K33, K34, K35, K36,      K38, K39, K3A, K3B, K3C, K3D, K3E, K3F, K3G, \
    K41, K42, K43, K44, K45, K46, K47, K48, K49, K4A, K4B, K4C, K4D,      K4F, K4G, \
    K51, K52, K53, K54, K55,           K58,      K5A, K5B, K5C, K5D,      K5F, K5G  \
) { \
    { K11, K12, K13, K14, K15, K16,   K17,   K18, K19,   K1A, K1B, K1C, K1D, K1E,   K1F, K1G }, \
    { K21, K22, K23, K24, K25, K26,   KC_NO, K28, K29,   K2A, K2B, K2C, K2D, K2E,   K2F, K2G }, \
    { K31, K32, K33, K34, K35, K36,   KC_NO, K38, K39,   K3A, K3B, K3C, K3D, K3E,   K3F, K3G }, \
    { K41, K42, K43, K44, K45, K46,   K47,   K48, K49,   K4A, K4B, K4C, K4D, KC_NO, K4F, K4G }, \
    { K51, K52, K53, K54, K55, KC_NO, KC_NO, K58, KC_NO, K5A, K5B, K5C, K5D, KC_NO, K5F, K5G }, \
}

#endif
