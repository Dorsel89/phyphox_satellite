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

//#define DEBUGGING

#ifdef DEBUGGING
    #include "USBSerial.h"
    FileHandle *mbed::mbed_override_console(int fd)
    {
        static USBSerial serial(true, 0x1f00, 0x2012, 0x0001);
        return &serial;
    }
#endif

EventQueue readoutSensorsQueue(2*64 * EVENTS_EVENT_SIZE);

DigitalOut LED_R(LEDRed);
DigitalOut LED_B(LEDBlue);

Timer global;
LowPowerTicker ds18b20Ticker;
LowPowerTicker thermocoupleTicker;
LowPowerTicker mprlsTicker;
LowPowerTicker imuTicker;

uint32_t flashAddress = 0x0FE000;
FLASH myCONFIG(flashAddress);

volatile int deviceCount = 0;
const int conSets = 100;
float dataSet[conSets];
float dataSetTime[conSets];
uint16_t dataSetNumber = 0;

void shtc3Config();
void mlxConfig();
void imuConfig();
void bmpConfig();
void ds18b20Config();
void tcConfig();
void mprlsConfig();

void onConnect();
void onDisconnect();

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
InterruptIn IMU_DataReady(INT1);
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

void blinkLed(int times, DigitalOut led){
    for (int i =0; i<times; i++) {
        led=!led;
        ThisThread::sleep_for(200ms);
        led=!led;
        ThisThread::sleep_for(200ms);
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




bool getNBit(uint8_t byte, int position){
    return byte & (1<<position);
}


void readBmp(){
    bmp384.getData();
    float data[3]={(float)0.01*bmp384.pressure,bmp384.temperature,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(data,3,ID_BMP384);
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
void readLoadcell(){
    loadcellSample.status          = myWeightSensor.ADS1231_ReadRawData(&loadcellSample.count, loadcellSample.num_avg);
    loadcellSample.calculated_volt = myWeightSensor.ADS1231_CalculateVoltage(&loadcellSample.count, 3.3);
    loadcellSample.calculated_mass = myWeightSensor.ADS1231_CalculateMass(&loadcellSample.count, 0.052, ADS1231::ADS1231_SCALE_g);
    float value[2] = {loadcellSample.calculated_mass.myMass,(float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count()};
    PhyphoxBLE::write(value,2,ID_LOADCELL);
}
void readIMU(){
        uint8_t error;
        IMU.status();
        
        error = IMU.readData();
        if(error == false){
            float time = (float)0.001*(float)duration_cast<std::chrono::milliseconds>(global.elapsed_time()).count();
            IMU.measuredDataAcc[4*IMU.currentPackage+0]=IMU.ax;
            IMU.measuredDataAcc[4*IMU.currentPackage+1]=IMU.ay;
            IMU.measuredDataAcc[4*IMU.currentPackage+2]=IMU.az;
            IMU.measuredDataAcc[4*IMU.currentPackage+3]=time;


            IMU.measuredDataGyr[4*IMU.currentPackage+0]=IMU.gx;
            IMU.measuredDataGyr[4*IMU.currentPackage+1]=IMU.gy;
            IMU.measuredDataGyr[4*IMU.currentPackage+2]=IMU.gz;
            IMU.measuredDataGyr[4*IMU.currentPackage+3]=time;
            IMU.currentPackage+=1;

            if(IMU.currentPackage == IMU.numberPerPackage){
                PhyphoxBLE::write(IMU.measuredDataAcc,IMU.numberPerPackage*4,ID_ICM42605_ACC);
                PhyphoxBLE::write(IMU.measuredDataGyr,IMU.numberPerPackage*4,ID_ICM42605_GYR);
                IMU.currentPackage = 0;
            }
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
void bmpSetFlag(){
    readoutSensorsQueue.call(readBmp);
}
void shtc3SetFlag(){
    readoutSensorsQueue.call(readShtc3);
}
void imuSetFlag(){
    //readoutSensorsQueue.call(readIMU);
}
void mlxSetFlag(){
    readoutSensorsQueue.call(readMLX);
}
void ds18b20SetFlag(){
    readoutSensorsQueue.call(readDS18B20);
}
void thermocoupleSetFlag(){
    readoutSensorsQueue.call(readTHERMOCOUPLE);
}
void mprlsSetFlag(){
    readoutSensorsQueue.call(readMPRLS);
}
void checkBattery(){
    readoutSensorsQueue.call(updateBatteryStatus);
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
        ThisThread::sleep_for(100ms);
        bmp384.enable();
    }else {
        bmp384.disable();
    }
}

void imuConfig(){
    PhyphoxBLE::read(&configData[0],20,ID_ICM42605);
    IMU.changeSettings(configData[3],configData[1], configData[2]);
    ThisThread::sleep_for(1ms);
    if((uint8_t)configData[4]>0 && (uint8_t)configData[4]<=10){
        IMU.numberPerPackage = (uint8_t)configData[4];
    }else{
        IMU.numberPerPackage = 1;
    }
    IMU.setState(1,1);    
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
    }else{
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

void receivedSN() {           // get data from phyphox app
    uint8_t mySNBufferArray[20];
    PhyphoxBLE::readHWConfig(&mySNBufferArray[0],20);
    if(mySNBufferArray[2]){
        //restart to get name
        NVIC_SystemReset();
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

void onDisconnect(){
    if(PhyphoxBLE::currentConnections == 0){
        NVIC_SystemReset(); //restart device until bug fixed
        /*
        tickerShtc3.detach();
        thermocoupleTicker.detach();
        mprlsTicker.detach();                
        ds18b20Ticker.detach();
        imuTicker.detach();
        IMU.setState(false, false);
        LED_R = 0;
        mlx.exitMode();
        bmp384.disable(); 
        */               
    }
    //blinkLed(1, LED_R);
}
void onConnect(){
    if(PhyphoxBLE::currentConnections == 1){
        global.reset();
    }
    blinkLed(1, LED_B);
}
void imuDataAvailable(){
    readoutSensorsQueue.call(readIMU);
}
int main()
{
    //mbed_trace_init();   
    printf("Starting Satellite!\r\n");
    
    PhyphoxBLE::led = &LED_R;
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

    PhyphoxBLE::myMainQueue = &readoutSensorsQueue;
    PhyphoxBLE::shtc3Handler = &shtc3Config;
    PhyphoxBLE::imuHandler = &imuConfig;
    PhyphoxBLE::mlxHandler = &mlxConfig;
    PhyphoxBLE::bmpHandler = &bmpConfig;

    PhyphoxBLE::mprlsHandler = &mprlsConfig;
    PhyphoxBLE::ds18b20Handler = &ds18b20Config;
    PhyphoxBLE::thermocoupleHandler = &thermocoupleConfig;

    PhyphoxBLE::hwConfigHandler = &receivedSN;

    PhyphoxBLE::connectHandler = &onConnect;
    PhyphoxBLE::disconnectHandler = &onDisconnect;

    // init IMU
    IMU.reset();
    IMU.init(AFS_2G, GFS_15_125DPS, AODR_25Hz , AODR_25Hz);
    IMU_DataReady.rise(&imuDataAvailable);
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
    readoutSensorsQueue.dispatch_forever();
    
    osThreadSetPriority(osThreadGetId(), osPriorityIdle);

    while (true) {
        ThisThread::sleep_for(1000ms);
    }
}

