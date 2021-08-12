/*
 * si5351.c
 *
 *  Created on: Jan 17, 2021
 *      Author: Thilina
 */
#include "si5351.h"

void clk_init()
{
    clk_off();
}

void clk_reset()
{
    clk_i2c_write(177, 0xA0);
}

void clk_set_frequency(uint8_t clk,uint32_t freq)
{

    uint8_t a, R = 1, shifts = 0;
    uint32_t b, c, f, fvco, outdivider;
    uint32_t MSx_P1, MSNx_P1, MSNx_P2, MSNx_P3;

    // Overclock option
    #ifdef SI_OVERCLOCK
        // user a overclock setting for the VCO, max value in my hardware
        // was 1.05 to 1.1 GHz, as usual YMMV [See README.md for details]
        outdivider = SI_OVERCLOCK / freq;
    #else
        // normal VCO from the datasheet and AN
        // With 900 MHz beeing the maximum internal PLL-Frequency
        outdivider = 900000000 / freq;
    #endif

    // use additional Output divider ("R")
    while (outdivider > 900) {
        R = R * 2;
        outdivider = outdivider / 2;
    }

    // finds the even divider which delivers the intended Frequency
    if (outdivider % 2) outdivider--;

    // Calculate the PLL-Frequency (given the even divider)
    fvco = outdivider * R * freq;

    // Convert the Output Divider to the bit-setting required in register 44
    switch (R) {
        case 1:   R = 0; break;
        case 2:   R = 16; break;
        case 4:   R = 32; break;
        case 8:   R = 48; break;
        case 16:  R = 64; break;
        case 32:  R = 80; break;
        case 64:  R = 96; break;
        case 128: R = 112; break;
    }

    // we have now the integer part of the output msynth
    // the b & c is fixed below
    MSx_P1 = 128 * outdivider - 512;

    // calc the a/b/c for the PLL Msynth
    /***************************************************************************
    * We will use integer only on the b/c relation, and will >> 5 (/32) both
    * to fit it on the 1048 k limit of C and keep the relation
    * the most accurate possible, this works fine with xtals from
    * 24 to 28 Mhz.
    *
    * This will give errors of about +/- 2 Hz maximum
    * as per my test and simulations in the worst case, well below the
    * XTAl ppm error...
    *
    * This will free more than 1K of the final eeprom
    *
    ****************************************************************************/
    a = fvco / int_xtal;
    b = (fvco % int_xtal) >> 5;     // Integer par of the fraction
                                    // scaled to match "c" limits
    c = int_xtal >> 5;              // "c" scaled to match it's limits
                                    // in the register

    // f is (128*b)/c to mimic the Floor(128*(b/c)) from the datasheet
    f = (128 * b) / c;

    // build the registers to write
    MSNx_P1 = 128 * a + f - 512;
    MSNx_P2 = 128 * b - f * c;
    MSNx_P3 = c;

    // PLLs and CLK# registers are allocated with a shift, we handle that with
    // the shifts var to make code smaller
    if (clk > 0 ) shifts = 8;

    // plls, A & B registers separated by 8 bytes
    clk_i2c_write(26 + shifts, (MSNx_P3 & 65280) >> 8);   // Bits [15:8] of MSNx_P3 in register 26
    clk_i2c_write(27 + shifts, MSNx_P3 & 255);
    clk_i2c_write(28 + shifts, (MSNx_P1 & 196608) >> 16);
    clk_i2c_write(29 + shifts, (MSNx_P1 & 65280) >> 8);   // Bits [15:8]  of MSNx_P1 in register 29
    clk_i2c_write(30 + shifts, MSNx_P1 & 255);            // Bits [7:0]  of MSNx_P1 in register 30
    clk_i2c_write(31 + shifts, ((MSNx_P3 & 983040) >> 12) | ((MSNx_P2 & 983040) >> 16)); // Parts of MSNx_P3 and MSNx_P1
    clk_i2c_write(32 + shifts, (MSNx_P2 & 65280) >> 8);   // Bits [15:8]  of MSNx_P2 in register 32
    clk_i2c_write(33 + shifts, MSNx_P2 & 255);            // Bits [7:0]  of MSNx_P2 in register 33

    // Write the output divider msynth only if we need to, in this way we can
    // speed up the frequency changes almost by half the time most of the time
    // and the main goal is to avoid the nasty click noise on freq change
    if (omsynth[clk] != outdivider) {
        // CLK# registers are exactly 8 * clk# bytes shifted from a base register.
        shifts = clk * 8;

        // multisynths
        clk_i2c_write(42 + shifts, 0);                        // Bits [15:8] of MS0_P3 (always 0) in register 42
        clk_i2c_write(43 + shifts, 1);                        // Bits [7:0]  of MS0_P3 (always 1) in register 43
        // See datasheet, special trick when R=4
        if (outdivider == 4) {
            clk_i2c_write(44 + shifts, 12 | R);
            clk_i2c_write(45 + shifts, 0);            // Bits [15:8] of MSx_P1 must be 0
            clk_i2c_write(46 + shifts, 0);            // Bits [7:0] of MSx_P1 must be 0
        } else {
            clk_i2c_write(44 + shifts, ((MSx_P1 & 196608) >> 16) | R);  // Bits [17:16] of MSx_P1 in bits [1:0] and R in [7:4]
            clk_i2c_write(45 + shifts, (MSx_P1 & 65280) >> 8);    // Bits [15:8]  of MSx_P1 in register 45
            clk_i2c_write(46 + shifts, MSx_P1 & 255);             // Bits [7:0]  of MSx_P1 in register 46
        }
        clk_i2c_write(47 + shifts, 0);                        // Bits [19:16] of MS0_P2 and MS0_P3 are always 0
        clk_i2c_write(48 + shifts, 0);                        // Bits [15:8]  of MS0_P2 are always 0
        clk_i2c_write(49 + shifts, 0);                        // Bits [7:0]   of MS0_P2 are always 0

        // must reset the so called "PLL", in fact the output msynth
        clk_reset();

        // keep track of the change
        omsynth[clk] = (uint16_t)outdivider;
    }
}

void clk_enable(uint8_t clk) {
    // var to handle the mask of the registers value
    uint8_t m = SICLK0_R;
    if (clk > 0) m = SICLK12_R;

    // write the register value
    clk_i2c_write(16 + clk, m + clkpower[clk]);

    // 1 & 2 are mutually exclusive
    if (clk == 1) clk_disable(2);
    if (clk == 2) clk_disable(1);

    // update the status of the clk
    clkOn[clk] = 1;

}

void clk_disable(uint8_t clk) {
    // send
    clk_i2c_write(16 + clk, 128);

    // update the status of the clk
    clkOn[clk] = 0;
}

void clk_set_power(uint8_t clk, uint8_t power) {
    // set the power to the correct var
    clkpower[clk] = power;

    // now enable the output to get it applied
    clk_enable(clk);
}

void clk_i2c_write(uint8_t regist, uint8_t value)
{
    uint8_t data[2];
    data[0] = regist;
    data[1] = value;
    HAL_I2C_Master_Transmit(&hi2c2, SIADDR <<1, data, 2, 100);

}

void clk_off()
{
    for (uint8_t i=0; i<3; i++) clk_disable(i);
}
