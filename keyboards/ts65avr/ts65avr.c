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
#include "ts65avr.h"
#include "i2c.h"

void led_set_kb(uint8_t usb_led)
{
    if (usb_led & (1<<USB_LED_CAPS_LOCK)) {
        // output low
        // indicator 1
        DDRC  |=  (1<<6);
        PORTC &= ~(1<<6);

        // indicator 2
        DDRD   |= (1<<7);
        PORTD &= ~(1<<7);

        // indicator 3
        DDRF  |=  (1<<1);
        PORTF &= ~(1<<1);
    } else {
        // Hi-Z
        // indicator 1
        DDRC  &= ~(1<<6);
        PORTC &= ~(1<<6);

        // indicator 2
        DDRD  &= ~(1<<7);
        PORTD &= ~(1<<7);

        // indicator 3
        DDRF  &= ~(1<<1);
        PORTF &= ~(1<<1);
    }
}
