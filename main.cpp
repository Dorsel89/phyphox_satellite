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
#include "flashUtility.h"


DigitalOut LED_R(LEDRed);
DigitalOut LED_B(LEDBlue);

Timer global;
Ticker ds18b20Ticker;
Ticker thermocoupleTicker;
Ticker mprlsTicker;
Ticker imuTicker;

uint32_t flashAddress = 0x0FE000;
FLASH myCONFIG(flashAddress);

volatile int deviceCount = 0;
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
        thermocoupleTicker.attach(&thermocoupleSetFlag, configData[1]*100ms);
    }else{
        thermocoupleTicker.detach();
    }

}
void mprlsConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_MPRLS);
    if(configData[0]==1){
        if(configData[1]==0){
            configData[1] = 1;
        }
        mprlsTicker.attach(&mprlsSetFlag, configData[1]*10ms);
    }else{
        mprlsTicker.detach();
    }


}
void ds18b20Config(){
    PhyphoxBLE::read(&configData[0],20,ID_DS18B20);
    if(configData[0]==1){
        ds18b20.init();
        ds18b20.startTempConversion();
        int interval =0;
        if(configData[1] == 0)   {
            interval = 750;
        }else {
            interval = configData[1]*1000;
        }
        ds18b20Ticker.attach(&ds18b20SetFlag, interval*1ms);
    }else{
        ds18b20Ticker.detach();
    }
}
void bmpConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_BMP384);
    
    if(configData[0]==1){
        bmp384.changeSettings(configData[1],configData[2],configData[3]);
        ThisThread::sleep_for(10ms); 
        bmp384.enable();
    }else {
        bmp384.disable();
    }
}

void imuConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_ICM42605);
    IMU.setState(false, false);

    IMU.init(configData[1], configData[3], configData[2], configData[4]);
    //byte 0; bit 0 = enable accelerometer, bit 1 = enable gyroscope
    //IMU.setState(getNBit(configData[0], 0), getNBit(configData[0], 1)); 
    
    if(configData[0]==true){
        if(IMU.setState(1,1)){
            NVIC_SystemReset();
        }
        imuTicker.attach(imuSetFlag, IMU.tickerInterval( configData[4])*1ms);
    }else {
        IMU.setState(0,0); 
        imuTicker.detach();
    }
    
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
    mlx.numberPerPackage = (uint8_t)configData[7];
    
    //byte 0; bit 0: enable mlx all axis
    if(getNBit(configData[0], 0)){
        mlx.startBurstMode();
        MLX.enable = true;
    }else{
        MLX.enable = false;
        mlx.exitMode();
    }    

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
}

void readMPRLS(){
    
    float value[2] = {mprls.readPressure(),(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(value,2,ID_MPRLS);
}
void readBMP(){
    bmp384.getData();
    float data[3]={(float)0.01*bmp384.pressure,bmp384.temperature,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(data,3,ID_BMP384);
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
        if(error == false){
            float time = (float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count();
            float accFloat[4]={IMU.ax,IMU.ay,IMU.az,time};
            float gyrFloat[4]={IMU.gx,IMU.gy,IMU.gz,time};
            PhyphoxBLE::write(accFloat,4,ID_ICM42605_ACC);
            PhyphoxBLE::write(gyrFloat,4,ID_ICM42605_GYR);
        }
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
void receivedSN() {           // get data from phyphox app
    uint8_t mySNBufferArray[20];
    PhyphoxBLE::readHWConfig(&mySNBufferArray[0],20);
    if(getNBit(mySNBufferArray[2], 0) ){
        //restart to get name
        NVIC_SystemReset();
    }
    //if second bit is true, user can set rgb led with bit 2 (blue) and 3 (red)
    if(getNBit(mySNBufferArray[2], 1) ){
        LED_B = getNBit(mySNBufferArray[2], 2);
        LED_R = getNBit(mySNBufferArray[2], 3);
    }
    
    uint16_t intSN[1];
    intSN[0] = mySNBufferArray[1] << 8 | mySNBufferArray[0];
    myCONFIG.writeSN(intSN);
    
  }
 void getDeviceName(char* myDeviceName){
    uint16_t mySN[1];
     
     myCONFIG.readSN(mySN);
     
     if(mySN[0] == 0xFFFF){
         mySN[0]=0;
     }
     
    
    std::string s = std::to_string(mySN[0]);
    if ( s.size() < 4 ){
        s = std::string(4 - s.size(), '0') + s;
    }
    std::string S;
    S.append("Satellit S");
    S.append(s);
    strcpy(myDeviceName, S.c_str());  
 }

void checkBattery(){
    flagBattery = true;
}

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
    i2c.frequency(200000);
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

    PhyphoxBLE::hwConfigHandler = &receivedSN;

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
    mlx.numberPerPackage = 1;
    mlx.exitMode();
    char DEVICENAME[30];
    getDeviceName(DEVICENAME);
    blinkLed(3, LED_R);
    PhyphoxBLE::start(DEVICENAME);        

    Ticker BatteryTicker;
    BatteryTicker.attach(checkBattery,1s);
    
    if(bmpAvailable){
        bmp384.init(0x03,0x03,0x03);
        bmp384.disable();     
    }

    mprls.setI2C(0x18 << 1 ,&i2c);  //TODO
    //EN_LOADCELL = 1;
    //initADS1231();
    
    while (true) {
        if(PhyphoxBLE::currentConnections >0 && deviceInSleepMode){
            deviceInSleepMode=false;
            global.reset();
        }
        if(PhyphoxBLE::currentConnections != deviceCount){
            if(PhyphoxBLE::currentConnections>deviceCount){
                blinkLed(2, LED_B);
            }
            deviceCount = PhyphoxBLE::currentConnections;
        }
        if(PhyphoxBLE::currentConnections ==0){
            if(!deviceInSleepMode){
                imuTicker.detach();
                IMU.setState(false, false);
                MLX.enable = false;
                mlx.exitMode();
                bmp384.disable();
                tickerShtc3.detach();

                thermocoupleTicker.detach();
                mprlsTicker.detach();                
                ds18b20Ticker.detach();
                ThisThread::sleep_for(10ms);
                flagBattery = false;
                flagSHTC3 = false;
                flagMPRLS = false;
                flagTC = false;
                flagIMU = false;
                flagMLX = false;
                flagBMP = false;
                flagDS18B20 = false;
                flagLoadcell = false;

                deviceInSleepMode = true;
            }
            ThisThread::sleep_for(200ms);
        }
        
        
        if(bmpAvailable){
            if(flagBMP){
                readBMP();
                flagBMP = false;
            }
        }        
        if(flagBattery){
            updateBatteryStatus();
            flagBattery=false;
        }
        if(flagSHTC3){
            readShtc3();
            flagSHTC3=false;
        }
        if(flagLoadcell){
            readLoadcell();
            flagLoadcell=false;
        }
        if(flagMPRLS){
            readMPRLS();
            flagMPRLS=false;    
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
            readTHERMOCOUPLE();
            flagTC=false;

        }        
    }
}

