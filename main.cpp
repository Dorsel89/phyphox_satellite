#include "mbed.h"
#include "phyphoxBle.h"
#include <chrono>
#include <cstdint>
#include <cstring>
#include "MLX90393.h"
#include "MS5607.h"
#include "SHTC3.h"
#include "I2Chelper.h"
#include "ICM42605.h"

PinName LEDGreen = P0_28;
PinName LEDRed = P0_3; 
PinName SCL = P0_13;
PinName SDA = P0_14;
PinName MLX_TRIG = P0_15;
PinName INT2 = P0_16;
PinName INT1 = P0_24;
PinName MLX_INT = P0_25;
PinName SPEED_LOADCELL = P0_2;
PinName PWDWN_LOADCELL = P0_31;
PinName ADC_SUPERCAP = P0_4;
PinName DATA_LOADCELL = P0_23;
PinName SCLK_LOADCELL = P0_21;
PinName EN_MULTIPLEXER_PIN = P0_22;
PinName IN_MULTIPLEXER_PIN = P0_17;
PinName ONEWIRE = P0_20; 



//  Multiplexer:    TS3A5018RGYR
//  Magnetometer:   MLX90393
int Address = 24;
//  Pressure:       MS560702BA3-50 Address:111011C1x,238
//  Loadcell:       ADS1231IDR
//  IMU:            ICM-42605

I2C i2c(SDA,SCL);
I2Chelper myI2C(&i2c);
ICM42605 IMU(&myI2C);

DigitalOut LEDR(LEDRed); 
DigitalOut LEDG(LEDGreen); 
AnalogIn   SUPERCAP(ADC_SUPERCAP);
DigitalIn IMU_DataReady(INT1);
DigitalOut EN_MULTIPLEXER(EN_MULTIPLEXER_PIN);
DigitalOut IN_MULTIPLEXER(IN_MULTIPLEXER_PIN);

LowPowerTicker blinkTicker;
int ledCounter=0;

char dataBuffer[1+2+2+2+2];

void powerOn() {

    LEDR=!LEDR;
    LEDG=!LEDG;
}


int main() {

    
    NRF_POWER->DCDCEN = 1;
    NRF_UICR->NFCPINS = 0x00000000;

    EN_MULTIPLEXER = 0;
    IN_MULTIPLEXER = 0;

    LEDR=0; //turn led off
    LEDG=0; //turn led off
    //blinkTicker.attach(&powerOn,1s);

    PhyphoxBLE::start("satellit");              // start BLE

    //magnetometer.SM(&dataBuffer[0], 15, 0);

    //MLX90393 magnetometer(Address,&i2c);
    MS5607 pressure(&i2c);
    pressure.init();

    //SHTC3 sht;
    //sht.init(&i2c);
    //uint16_t raw_temp, raw_humidity;

    
     
    IMU.init(AFS_16G, GFS_2000DPS, AODR_50Hz, GODR_50Hz);
    IMU.disableAG();
    float t, p,h;
    
    while (true) {                            // start loop.
        /*
        if(IMU_DataReady){
            int16_t temp[40] = {0};
            IMU.readData(&temp[0]);
            uint8_t buf[12];
            memcpy(&buf[0],&temp[0]+2,12);
            PhyphoxBLE::write(&buf[0],12);
                
            ThisThread::sleep_for(20ms);
        }
        */
        //ThisThread::sleep_for(20ms);

        //


        pressure.readData();

        //uint8_t test[20];
        //memcpy(&test[0], &pressure.P_INT, 4);
        PhyphoxBLE::write(pressure.P,pressure.T);
        //PhyphoxBLE::write(&test[0],4);
        //pressure.getData(p, t);
        //t = SUPERCAP*3.3*11;//pressure.TEMP/100.0;
        //p = pressure.P/100.0;
        //sht.read(&raw_temp, &raw_humidity);
        //p = sht.toCelsius(raw_temp);
        //h = sht.toPercentage(raw_humidity);
        
    }   
}
