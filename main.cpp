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
#include "MLX90393.h"
#include "bmp384.h"


DigitalOut LED_R(LEDRed);
DigitalOut LED_B(LEDBlue);

Timer global;
Ticker ds18b20Ticker;
Ticker thermocoupleTicker;
Ticker mprlsTicker;
Ticker imuTicker;


const int conSets = 100;
float dataSet[conSets];
float dataSetTime[conSets];
uint16_t dataSetNumber = 0;


volatile bool flagBattery = false;
volatile bool flagSHTC3 = false;
volatile bool flagMPRLS = false;
volatile bool flagTC = false;
volatile bool flagIMU = false;
volatile bool flagMLX = false;
volatile bool flagBMP = false;
volatile bool flagDS18B20 = false;
volatile bool flagLoadcell = false;

void shtc3Config();
void mlxConfig();
void imuConfig();
void bmpConfig();

void ds18b20Config();
void tcConfig();
void mprlsConfig();

bool deviceInSleepMode = true;

Ticker tickerShtc3;


I2C i2c(SDA,SCL);

BMP384 bmp384(&i2c);
bool bmpAvailable = true;

I2Chelper myI2Chelper(&i2c);

ICM42605 IMU(&myI2Chelper);
MS5607 barometer(&i2c);
MLX90393 mlx;
SHTC3 shtc3;
MPRLS mprls;
//DigitalOut test(ONEWIRE);
Onewire myOneWire(ONEWIRE);
MAX31850 Thermocouple(&myOneWire);
DS18B20 ds18b20(&myOneWire);
ADS1231 myWeightSensor( SCLK_LOADCELL, DATA_LOADCELL_PIN );

//InterruptIn IMU_DataReady(INT1);
DigitalIn IMU_DataReady(INT1);
InterruptIn mlx_DataReady(MLX_INT);

InterruptIn bmpDataReady(INT_BMP384); 
AnalogIn ADC_SUPERCAP(ADC_SUPERCAP_PIN);

DigitalInOut EN_measureBattery(measureBattery);

DigitalOut EN_LOADCELL(PWDWN_LOADCELL_PIN);
DigitalOut FAST_LOADCELL(SPEED_LOADCELL_PIN);
DigitalIn dataReadyLoadcell(DATA_LOADCELL_PIN);

struct {
    ADS1231::ADS1231_status_t status;
    ADS1231::Vector_count_t   count;
    uint8_t num_avg;
    ADS1231::Vector_mass_t    calculated_mass;
    ADS1231::Vector_voltage_t calculated_volt;
} loadcellSample;

uint8_t configData[20]={0};

struct sensor {
    bool enable;
    bool burst;
    int datapoints;
    int threshold;
    bool rising;
};

struct sensor MLX;

void blinkLed(int times, DigitalOut led){
    for (int i =0; i<times; i++) {
        led=!led;
        ThisThread::sleep_for(300ms);
        led=!led;
        ThisThread::sleep_for(300ms);
    }
}

void updateBatteryStatus(){
    EN_measureBattery.output();
    EN_measureBattery = 0;
    ThisThread::sleep_for(1ms);
    uint8_t percent = 100*ADC_SUPERCAP * 3.3 * 11/(5.0);
    EN_measureBattery.input();
    //EN_measureBattery =1;
    PhyphoxBLE::batteryService.updateBatteryLevel(percent);
}

void shtc3SetFlag(){
    flagSHTC3 = true;
}
void imuSetFlag(){
    flagIMU = true;
}
void mlxSetFlag(){
    flagMLX = true;
}
void bmpSetFlag(){
    flagBMP = true;
}
void ds18b20SetFlag(){
    flagDS18B20 = true;
}
void thermocoupleSetFlag(){
    flagTC = true;
}
void mprlsSetFlag(){
    flagMPRLS = true;
}

bool getNBit(uint8_t byte, int position){
    return byte & (1<<position);
}

void thermocoupleConfig(){
    PhyphoxBLE::read(&configData[0],20, ID_THERMOCOUPLE);
    if(configData[0]==1){
        Thermocouple.init();  
        Thermocouple.startTempConversion();   //TODO
        thermocoupleTicker.attach(&thermocoupleSetFlag, 500ms);
    }
}
void mprlsConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_MPRLS);
    if(configData[0]==1){
        mprlsTicker.attach(&mprlsSetFlag, 1s);
    }else{
        mprlsTicker.detach();
    }


}
void ds18b20Config(){
    PhyphoxBLE::read(&configData[0],20,ID_DS18B20);
    if(configData[0]==1){
        ds18b20.init();
        ds18b20.startTempConversion();   
        ds18b20Ticker.attach(&ds18b20SetFlag, 500ms);
    }
}
void bmpConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_BMP384);
    if(configData[0]==1){
        bmp384.enable();
    }else {
        bmp384.disable();
    }
}

void imuConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_ICM42605);
    IMU.setState(false, false);
    ThisThread::sleep_for(5ms);
    //ThisThread::sleep_for(1s);
    IMU.init(configData[1], configData[3], configData[2], configData[4]);
    //ThisThread::sleep_for(2s);
    //ThisThread::sleep_for(1s);
    //IMU.init(AFS_2G, GFS_1000DPS , AODR_50Hz, GODR_50Hz);
    ThisThread::sleep_for(5ms);
    //ThisThread::sleep_for(10s);
    //byte 0; bit 0 = enable accelerometer, bit 1 = enable gyroscope
    //IMU.setState(getNBit(configData[0], 0), getNBit(configData[0], 1)); 
    IMU.setState(1,1); 
    ThisThread::sleep_for(5ms);
    imuTicker.attach(imuSetFlag, IMU.tickerInterval( configData[4])*1ms);
    //imuTicker.attach(imuSetFlag, 100ms);
    
    //IMU.setState(1,1);
    global.reset(); 
}
void mlxConfig(){
    
    mlx.exitMode();
    PhyphoxBLE::read(&configData[0],20,ID_MLX90393);
    mlx.setGain(mlx90393_gain((uint8_t)configData[1]));
    mlx.setFilter(mlx90393_filter((uint8_t)configData[2]));
    mlx.setOversampling(mlx90393_oversampling((uint8_t)configData[3]));
    mlx.setResolution(MLX90393_X, mlx90393_resolution((uint8_t)configData[4]));
    mlx.setResolution(MLX90393_Y, mlx90393_resolution((uint8_t)configData[5]));
    mlx.setResolution(MLX90393_Z, mlx90393_resolution((uint8_t)configData[6]));
    
    //mlx.startBurstMode();
    
    //byte 0; bit 0: enable mlx all axis
    if(getNBit(configData[0], 0)){
        mlx.startBurstMode();
        MLX.enable = true;
    }else{
        MLX.enable = false;
        mlx.exitMode();
    }    
    //MLX.burst = getNBit(configData[0], 1);
    //MLX.rising = getNBit(configData[0], 2);
    //MLX.threshold = configData[8] << 8 | configData[7];
    //MLX.datapoints = configData[10] << 8 | configData[9];   
}
void shtc3Config(){
    //read config and set ticker according to datarate
    PhyphoxBLE::read(&configData[0],20,ID_SHTC3);
    uint16_t timeInterval = 10*configData[1]; //byte 1 = 3 -> read sensor every 30ms
    if(timeInterval<10){
        timeInterval=10;
    }
    if(configData[0]==1){
        tickerShtc3.attach(&shtc3SetFlag, timeInterval*1ms);
    }else {
        tickerShtc3.detach();
    }
    
}

void readShtc3(){
    uint16_t value[3];
    shtc3.read(&value[0], &value[1],false); //always high precision mode
    float floatA[3] = {shtc3.toCelsius(value[0]),shtc3.toPercentage(value[1]),(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(&floatA[0],3,ID_SHTC3);
    flagSHTC3=false;
}

void readMPRLS(){
    
    float value[2] = {mprls.readPressure(),(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(value,2,ID_MPRLS);
    flagMPRLS=false;    
}
void readLoadcell(){
    loadcellSample.status          = myWeightSensor.ADS1231_ReadRawData(&loadcellSample.count, loadcellSample.num_avg);
    loadcellSample.calculated_volt = myWeightSensor.ADS1231_CalculateVoltage(&loadcellSample.count, 3.3);
    loadcellSample.calculated_mass = myWeightSensor.ADS1231_CalculateMass(&loadcellSample.count, 0.052, ADS1231::ADS1231_SCALE_g);
    float value[2] = {loadcellSample.calculated_mass.myMass,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(value,2,ID_LOADCELL);
}
void readIMU(){
        uint8_t error;
        error = IMU.readData();
        float copyValues[6] = {IMU.ax,IMU.ay,IMU.az,IMU.gx,IMU.gy,IMU.gz};
        wait_us(50);
        error = IMU.readData();
        if(error == false){
            float values[7]= {IMU.ax,IMU.ay,IMU.az,IMU.gx,IMU.gy,IMU.gz,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
            for(int i=0; i<6;i++){
                if(copyValues[i]==!values[i]){
                    return;
                }
            }
            PhyphoxBLE::write(values,7,ID_ICM42605);
        }
        /*

        uint8_t status = IMU.status();
        if(getNBit(status, 3)){
            error = IMU.readData();
            if(error == false){
                float values[7]= {IMU.ax,IMU.ay,IMU.az,IMU.gx,IMU.gy,IMU.gz,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
                PhyphoxBLE::write(values,7,ID_ICM42605);
            }
        }   
        */
}

void readTHERMOCOUPLE(){
        float value[2] = {Thermocouple.getTemp(),(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
        PhyphoxBLE::write(value,2,ID_THERMOCOUPLE);
        Thermocouple.startTempConversion();
}
void readDS18B20(){
    float floatData[2] = {ds18b20.getTemp(),(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    ds18b20.startTempConversion();
    PhyphoxBLE::write(floatData, 2, ID_DS18B20);  
}

void readMLX(){
    float value[4];
    mlx.readMeasurement(&value[0], &value[1], &value[2]);    
    mlx.measuredData[4*mlx.currentPackage+0]=value[0];
    mlx.measuredData[4*mlx.currentPackage+1]=value[1];
    mlx.measuredData[4*mlx.currentPackage+2]=value[2];
    mlx.measuredData[4*mlx.currentPackage+3]=0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count();
    mlx.currentPackage+=1;
    if(mlx.currentPackage == mlx.numberPerPackage){
        PhyphoxBLE::write(mlx.measuredData,mlx.numberPerPackage*4,ID_MLX90393);
        mlx.currentPackage = 0;
    }
}
void checkBattery(){
    flagBattery = true;
}
/*    
struct FLOATBUFFER{
    float DATABUFFER[1000];
    int valuesStored;
};

struct FLOATBUFFER myFloatbuffer;

void burstMode(float* newDataPoints,int numberOfDataPoints){
    myFloatbuffer.DATABUFFER[myFloatbuffer.valuesStored*numberOfDataPoints]
}
*/

void initADS1231(){
    /*
    loadcellSample.num_avg=1;
    uint8_t num_avg_cal = 4;
    ADS1231::ADS1231_status_t sts;
    test=0;
    sts = myWeightSensor.ADS1231_PowerDown();
    ThisThread::sleep_for(500ms);
    sts = myWeightSensor.ADS1231_Reset();
    ThisThread::sleep_for(500ms);
    myWeightSensor.ADS1231_ReadData_WithoutMass(&loadcellSample.count, num_avg_cal);
    test=1;
    ThisThread::sleep_for(5s);
    test=0;
    myWeightSensor.ADS1231_ReadData_WithCalibratedMass(&loadcellSample.count, num_avg_cal);
    ThisThread::sleep_for(500ms);
    myWeightSensor.ADS1231_SetAutoTare(0.052, ADS1231::ADS1231_SCALE_g, &loadcellSample.count, num_avg_cal);
    */
}

int main()
{
    
    global.reset();
    global.start();
    i2c.frequency(400000);
    NRF_POWER->DCDCEN = 1;
    NRF_UICR->NFCPINS = 0x00000000;
    
    EN_measureBattery.input();
    FAST_LOADCELL = 1;
    EN_LOADCELL = 0;
    LED_B=0;
    LED_R=0;

    PhyphoxBLE::shtc3Handler = &shtc3Config;
    PhyphoxBLE::imuHandler = &imuConfig;
    PhyphoxBLE::mlxHandler = &mlxConfig;
    PhyphoxBLE::bmpHandler = &bmpConfig;

    PhyphoxBLE::mprlsHandler = &mprlsConfig;
    PhyphoxBLE::ds18b20Handler = &ds18b20Config;
    PhyphoxBLE::thermocoupleHandler = &thermocoupleConfig;

    struct sensor MLX = {0,0,0,0,true};

    // init IMU
    IMU.reset();
    IMU.init(AFS_2G, GFS_15_125DPS, AODR_12_5Hz, GODR_12_5Hz);
    //IMU.setState(true, true);

    // init SHTC3
    shtc3.init(&i2c);

    //init mlx
    mlx_DataReady.rise(&mlxSetFlag);
    bmpDataReady.rise(&bmpSetFlag);
    mlx.begin_I2C(24, &i2c);//0x18
    mlx.numberPerPackage = 8;
    mlx.exitMode();
    PhyphoxBLE::start("Satellit S0000");        

    Ticker BatteryTicker;
    BatteryTicker.attach(checkBattery,200ms);
    
    if(bmpAvailable){
        bmp384.init();
        bmp384.disable();   //TODO   
        
    }
    mprls.setI2C(0x18 << 1 ,&i2c);  //TODO
    //EN_LOADCELL = 1;
    //initADS1231();
    while (true) {
        if(PhyphoxBLE::currentConnections >0 && deviceInSleepMode){
            deviceInSleepMode=false;
            global.reset();
        }
        if(PhyphoxBLE::currentConnections ==0){
            if(!deviceInSleepMode){
                IMU.setState(false, false);
                MLX.enable = false;
                mlx.startSingleMeasurement(); // "disabled"
                mlx.setOversampling(mlx90393_oversampling(0x03));
                mlx.setFilter(mlx90393_filter(7));
                thermocoupleTicker.detach();
                mprlsTicker.detach();
                imuTicker.detach();
                ds18b20Ticker.detach();
                deviceInSleepMode = true;
                bmp384.disable();
            }
            ThisThread::sleep_for(1s);            
            continue;
        }
        if(bmpAvailable){
            if(flagBMP){
                bmp384.getData();
                float data[3]={(float)0.01*bmp384.pressure,bmp384.temperature,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
                PhyphoxBLE::write(data,3,ID_BMP384);
                flagBMP = false;
            }
        }        
        if(flagBattery){
            updateBatteryStatus();
            flagBattery=false;
        }
        if(flagSHTC3){
            readShtc3();
        }
        if(flagLoadcell){
            readLoadcell();
            flagLoadcell=false;
        }
        if(flagMPRLS){
            readMPRLS();
        }
        if(flagIMU){
            readIMU();
            flagIMU=false;
        } 
            
        if(flagMLX){
            flagMLX=false;
            readMLX();
        }
        
         if(flagDS18B20){
            flagDS18B20=false;
            readDS18B20();
        }
         if(flagTC){
            flagTC=false;
            readTHERMOCOUPLE();
        }        
    }
}

