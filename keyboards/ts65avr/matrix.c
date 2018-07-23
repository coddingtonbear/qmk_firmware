/*
Copyright 2012 Jun Wako <wakojun@gmail.com>

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

/*
 * scan matrix
 */
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "print.h"
#include "debug.h"
#include "util.h"
#include "matrix.h"
#include "timer.h"

#include "i2c.h"

#ifndef DEBOUNCING_DELAY
#   define DEBOUNCING_DELAY	5
#endif

#define I2C_ADDR 0b0100000
#define I2C_ADDR_WRITE ((I2C_ADDR << 1) | I2C_WRITE)
#define I2C_ADDR_READ ((I2C_ADDR << 1) | I2C_READ)

/* matrix state(1:on, 0:off) */
// index by columns, holds rows
static matrix_row_t matrix[MATRIX_ROWS];
static matrix_row_t matrix_debouncing[MATRIX_ROWS];

static bool read_rows(uint8_t col);
static void init_rows(void);
static void unselect_cols(void);
static void select_col(uint8_t row);

static uint32_t matrix_timer;
static uint32_t matrix_scan_count;

static bool debouncing = false;
static uint16_t debouncing_time;

inline
uint8_t matrix_rows(void)
{
    return MATRIX_ROWS;
}

inline
uint8_t matrix_cols(void)
{
    return MATRIX_COLS;
}

static void init_mcp(void)
{
    i2c_master_init();

    // set IO direction
    if (i2c_master_start(I2C_ADDR_WRITE)) goto err;
    // select IODIRA register
    if (i2c_master_write(0x00)) goto err;
    // set GPA0-GPA7 (0=output)
    if (i2c_master_write(0x00)) goto err;
    // set GPB0 (0=output) and GPB1-GPB5 (1=input)
    if (i2c_master_write(0b00111110)) goto err;
    i2c_master_stop();

    // pull up inputs
    if (i2c_master_start(I2C_ADDR_WRITE)) goto err;
    // Write to GPPUB (address 0x0D) (1=enable) to pull-up inputs
    if (i2c_master_write(0x0C)) goto err;
    if (i2c_master_write(0xff)) goto err;
    if (i2c_master_write(0xff)) goto err;

    //if (i2c_master_write(0x0D)) goto err;
    //if (i2c_master_write(0b00111110)) goto err;

    i2c_master_stop();

    println("init_mcp");

    return;

err:
    println("i2c error in init_mcp");
    i2c_master_stop();
}

/* Row pin configuration
 * col: L1 L2 L3 L4 L5
 * pin: B0 B1 B2 B3 B6
 */
static void init_rows(void)
{
    // Input with pull-up (DDR:0, PORT:1)
    PORTB |=  0b01001111;
    DDRB  &= ~0b01001111;

    println("init_rows");
}

void matrix_init(void)
{
    // disable jtag
    MCUCR |= (1<<JTD);
    MCUCR |= (1<<JTD);

    // initialise i2c and i/o expander
    init_mcp();

    // set multiplexer pins PF4-PF7 as outputs
    PORTF &= ~0x80;
    DDRF |= 0b11110000;

    // initialize row and col
    init_rows();
    // reset columns to known state
    unselect_cols();

    // initialize matrix state: all keys off
    for (uint8_t i=0; i < MATRIX_ROWS; i++) {
        matrix[i] = 0;
        matrix_debouncing[i] = 0;
    }

    matrix_timer = timer_read32();
    matrix_scan_count = 0;

    debug_matrix = true;
    debug_enable = true;
}

uint8_t matrix_scan(void)
{
    matrix_scan_count++;

    uint32_t timer_now = timer_read32();

    if (TIMER_DIFF_32(timer_now, matrix_timer) > 1000) {
        print("matrix scan frequency: ");
        pdec(matrix_scan_count);
        print("\n");

        matrix_timer = timer_now;
        matrix_scan_count = 0;
    }

    // probe each column, sense on rows
    for (uint8_t i = 0; i < MATRIX_COLS; i++) {
        select_col(i);
        _delay_us(30); // was 30

        if (read_rows(i)) {
            debouncing = true;
            debouncing_time = timer_read();
        }

        unselect_cols();
    }

    if (debouncing && (timer_elapsed(debouncing_time) > DEBOUNCING_DELAY)) {
        for (uint8_t i = 0; i < MATRIX_ROWS; i++)
            matrix[i] = matrix_debouncing[i];

        debouncing = false;
    }

    return 1;
}

inline
bool matrix_is_on(uint8_t row, uint8_t col)
{
    return (matrix[row] & ((matrix_row_t)1<<col));
}

inline
matrix_row_t matrix_get_row(uint8_t row)
{
    return matrix[row];
}

void matrix_print(void)
{
    print("\nr/c 0123456789ABCDEF\n");
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        phex(row); print(": ");
        pbin_reverse16(matrix_get_row(row));
        print("\n");
    }
}

static bool set_rows(uint8_t col, uint8_t rows)
{
    bool changed = false;

    for (int row = 0; row < MATRIX_ROWS; row++) {
        // save previous value
        matrix_row_t prev_value = matrix_debouncing[row];

        // clear current bit for column
        matrix_debouncing[row] &= ~((matrix_row_t)1 << col);
        // set target bit
        matrix_debouncing[row] |= (((matrix_row_t)rows & (1 << row)) >> row) << col;

        if (prev_value != matrix_debouncing[row])
            changed = true;
    }

    return changed;
}

/* Returns status of switches(1:on, 0:off) */
static bool read_rows(uint8_t col)
{
    uint8_t rows;

    // read from left hand column
    // pins B0 B1 B2 B3 B6
    if (col < MATRIX_COLS_LEFT) {
        rows = ~PINB;

        rows = (rows & 0b00001111) | ((rows & 0b01000000) >> 2);

        if (matrix_scan_count == 0)
            xprintf("read_rows %d (left): %08b\n", col, rows);
    } else {
        if (i2c_master_start(I2C_ADDR_WRITE)) goto err;
        // set read address (GPIOB)
        if (i2c_master_write(0x13)) goto err;
        // issue a restart to get into read mode
        if (i2c_master_start(I2C_ADDR_READ)) goto err;

        rows = i2c_master_read(I2C_NACK);
        rows = ~rows;

        i2c_master_stop();

        rows = (rows & 0b00111110) >> 1;

        if (matrix_scan_count == 0)
            xprintf("read_rows %d (right): %08b\n", col, rows);
    }

    // transpose scanned column into matrix array
    return set_rows(col, rows);

err:
    println("i2c error in read_rows");
    i2c_master_stop();

    return false;
}

// deselect columns by driving all outputs high
static void unselect_cols(void)
{
    //println("unselect_cols");

    // left hand
    // PF7 high to disable demultiplexer
    PORTF |=  0x80;
    // Clear address bits
    //PORTF &= ~0b01110000;

    // right hand
    if (i2c_master_start(I2C_ADDR_WRITE)) goto err;
    // set hi-z
    /* if (i2c_master_write(0x00)) goto err; */
    /* if (i2c_master_write(0xff)) goto err; */
    /* if (i2c_master_write(0xff)) goto err; */
    // set OLATA address
    if (i2c_master_write(0x12)) goto err;
    // drive GPA0-GPA7 high
    if (i2c_master_write(0xff)) goto err;
    // drive GPB0 high
    if (i2c_master_write(0x01)) goto err;
    i2c_master_stop();

    return;

err:
    println("i2c error in unselect_cols");
    i2c_master_stop();
}

/*
 * Left cols are enabled (active low) when PF7 is high
 */
static void select_col(uint8_t col)
{
    //xprintf("select_col %d\n", col);

    if (col < MATRIX_COLS_LEFT) {
        // left hand
        // set address bits
        PORTF = (PORTF & (~0b01110000)) | ((col & 0b111) << 4);
        // PF7 low to enable
        PORTF &= ~0b10000000;
    } else {
        // right hand
        // reset col to zero-based
        col -= MATRIX_COLS_LEFT;
        if (i2c_master_start(I2C_ADDR_WRITE)) goto err;

        // write OLAT address -- OLATA for cols 8-15 (0-7), OLATB for col 16 (8)
        if (i2c_master_write(0x12 + (col >= 8 ? 1 : 0))) goto err;

        if (col < 8) {
            if (i2c_master_write(~(1 << col))) goto err;
        } else {
            if (i2c_master_write(0x00)) goto err;
        }

        i2c_master_stop();
    }

    return;
err:
    print("i2c error in select_col\n");
    i2c_master_stop();
}
