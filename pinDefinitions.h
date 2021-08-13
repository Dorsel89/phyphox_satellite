#ifndef PINDEFINITIONS_H
#define PINDEFINITIONS_H
#include "mbed.h"

//pin names
PinName LEDBlue = P0_3; //gpio16
PinName LEDRed = P0_28; //gpio17
PinName SCL = P0_13;//gpio 1
PinName SDA = P0_14;//gpio 2
PinName MLX_TRIG = P0_15;//gpio3
PinName INT2 = P0_16;//gpio 4
PinName INT1 = P0_24;//gpio 5
PinName MLX_INT = P0_25;//gpio 7
PinName SPEED_LOADCELL_PIN = P0_2;//gpio18
PinName PWDWN_LOADCELL_PIN = P0_31;//gpio20
PinName ADC_SUPERCAP_PIN = P0_4;//gpio25
PinName DATA_LOADCELL_PIN = P0_23;//gpio 47
PinName SCLK_LOADCELL = P0_21;//gpio 48

PinName ONEWIRE = P0_20;//gpio50
PinName INT_BMP384 = P0_17;//gpio
PinName SWO = P1_0; //gpio8
PinName measureBattery = P0_22; //gpio49
#endif