#include "mbed.h"
#include "pinDefinitions.h"
//ble server
#include "phyphoxBle.h"
//utility
#include "I2Chelper.h"
//sensors
#include "SHTC3.h"
#include "ICM42605.h"
#include "MS5607.h"
#include "MPRLS.h"
#include "ADS1231.h"
#include "Onewire.h"
#include "MAX31850.h"
#include "DS18B20.h"
#include <cstdint>

DigitalOut LED_R(LEDRed);
DigitalOut LED_B(LEDBlue);

Timer global;

volatile bool flagBattery = false;
volatile bool flagSHTC3 = false;
volatile bool flagIMU = false;

void shtc3Config();
void mlxConfig();
void imuConfig();

Ticker tickerShtc3;

I2C i2c(SDA,SCL);


I2Chelper myI2Chelper(&i2c);

ICM42605 IMU(&myI2Chelper);
MS5607 barometer(&i2c);

SHTC3 shtc3;

MPRLS WL_Pressure;

InterruptIn IMU_DataReady(INT1);
AnalogIn ADC_SUPERCAP(ADC_SUPERCAP_PIN);
DigitalOut EN_MULTIPLEXER(EN_MULTIPLEXER_PIN);
DigitalOut IN_MULTIPLEXER(IN_MULTIPLEXER_PIN);
DigitalOut EN_LOADCELL(PWDWN_LOADCELL_PIN);
DigitalOut FAST_LOADCELL(SPEED_LOADCELL_PIN);

uint8_t configData[20]={0};

void blinkLed(int times, DigitalOut led){
    for (int i =0; i<times; i++) {
        led=!led;
        ThisThread::sleep_for(300ms);
        led=!led;
        ThisThread::sleep_for(300ms);
    }
}

void updateBatteryStatus(){
    uint8_t percent = 100*ADC_SUPERCAP * 3.3 * 11/5.0;
    PhyphoxBLE::batteryService.updateBatteryLevel(percent);
}

void shtc3SetFlag(){
    flagSHTC3 = true;
    
}
void imuSetFlag(){
    flagIMU = true;
    
}
void mlxConfig(){
    //TODO
}
bool getNBit(uint8_t byte, int position){
    return byte & (1<<position);
}
void imuConfig(){
    
    PhyphoxBLE::read(&configData[0],20,ID_ICM42605);
    //byte 0; bit 0 = enable accelerometer, bit 1 = enable gyroscope
    IMU.setState(getNBit(configData[0], 0), getNBit(configData[0], 1));
    
}
void shtc3Config(){
    //read config and set ticker according to datarate
    PhyphoxBLE::read(&configData[0],20,ID_SHTC3);
    uint16_t timeInterval = 10*configData[1]; //byte 1 = 3 -> read sensor every 30ms
    if(timeInterval<10){
        timeInterval=10;
    }
    if(&configData[0]){
        tickerShtc3.attach(&shtc3SetFlag, timeInterval*1ms);
    }else {
        tickerShtc3.detach();
    }
    
}

void readShtc3(){
    uint16_t value[2];
    shtc3.read(&value[0], &value[1],false); //always high precision mode
    float floatA[2] = {shtc3.toCelsius(value[0]),shtc3.toPercentage(value[1])};
    PhyphoxBLE::write(&floatA[0],2,ID_SHTC3);
    flagSHTC3=false;
}
void readIMU(){
        uint8_t error;
        error = IMU.readData();
        if(error == false){
            float values[7]= {IMU.ax,IMU.ay,IMU.az,IMU.gx,IMU.gy,IMU.gz,(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
            PhyphoxBLE::write(values,7,ID_ICM42605);
        }   
}



int main()
{
    
    global.reset();
    global.start();
    i2c.frequency(1000);
    NRF_POWER->DCDCEN = 1;
    NRF_UICR->NFCPINS = 0x00000000;
    
    EN_MULTIPLEXER = 0; 
    IN_MULTIPLEXER = 1;
    FAST_LOADCELL = 1;
    EN_LOADCELL = 0;
    
    PhyphoxBLE::shtc3Handler = &shtc3Config;
    PhyphoxBLE::imuHandler = &imuConfig;
    PhyphoxBLE::mlxHandler = &mlxConfig;
    //PhyphoxBLE::bmpHandler = &shtc3Config;

    
    // init IMU
    IMU.init(AFS_2G, GFS_15_125DPS, AODR_12_5Hz, GODR_12_5Hz);
    //IMU.disableA();
    //IMU.disableG();
    IMU_DataReady.rise(&imuSetFlag);

    // onboard barometer 
    barometer.init();

    // onboard temp
    shtc3.init(&i2c);

    // MPRLS
    WL_Pressure.setI2C(0x18 << 1,&i2c);
/*
    //loadcell
    ADS1231 myWeightSensor( SCLK_LOADCELL, DATA_LOADCELL_PIN );
*/

    Onewire myOneWire(ONEWIRE);
    PhyphoxBLE::start("satellit");        

/*
    MAX31850 myThermocouple(&myOneWire);
    myThermocouple.init();  
    myThermocouple.startTempConversion();
    ThisThread::sleep_for(1s);      
*/
    //ds18b20 oneWireAdress[8]={40,138,229,69,146,17,2,161};f
    
    DS18B20 myTemp(&myOneWire);
    /*
    myTemp.init();
    ThisThread::sleep_for(100ms);
    myTemp.startTempConversion();
    ThisThread::sleep_for(1s);
    volatile float test =  myTemp.getTemp();
    */

    while (true) {
        /*
        float pres = WL_Pressure.readPressure();
        float floatData[1] = {myThermocouple.getTemp()};
        myThermocouple.startTempConversion();
        */
        

        //float floatData[1] = {myTemp.getTemp()};
        //myTemp.startTempConversion();
        //PhyphoxBLE::write(&floatData[0], 1, ID_MPRLS); 
        
        

        if(flagBattery){
            updateBatteryStatus();
            flagBattery=false;
        }
        if(flagSHTC3){
            readShtc3();
        }
        
        if(flagIMU){
            flagIMU=false;
            readIMU();
        }        

    }
}

