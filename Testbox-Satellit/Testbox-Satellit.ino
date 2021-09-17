#include <SD.h>

bool printData = false;
bool waermelehreConnected = true;

#define CONFIGMODE 0

#define SHTC_INIT 1
#define SHTC_MEASURE 2

#define BMP_INIT 3
#define BMP_MEASURE 4

#define MLX_INIT 5
#define MLX_MEASURE 6

#define IMU_INIT 7
#define IMU_MEASURE 8

#define DS18B20_INIT 9
#define DS18B20_MEASURE 10

#define TC_INIT 11
#define TC_MEASURE 12

#define MPRLS_INIT 13
#define MPRLS_MEASURE 14

//SHTC3 Temp/Humidity Limits
// temperaturetarget,maxDeviation, humiditytarget,maxdeviation
float LIMITS_SHTC[4]={25.0,5.0,50.0,5};

//BMP384 Temp/Pressure Limits
// temperaturetarget,maxDeviation, pressuretarget,maxdeviation
float LIMITS_BMP384[4]={25.0,5.0,1000,10};

//MLX magnetic field limits (abs)
// magnetic field target,maxDeviation uT
float LIMITS_MLX[2]={45.0,5};

//IMU ACC(abs)
// acceleration target,maxDeviation (values are in g )
float LIMITS_IMU[2]={1,0.05};

//MPRLS (pressure)
// mprls pressure target,maxDeviation 
float LIMITS_MPRLS[2]={990,5};

//Thermocouple (pressure)
// temperature target,maxDeviation 
float LIMITS_TC[2]={25,5};

//DS18B20 (pressure)
//temperature target,maxDeviation 
float LIMITS_DS18B20[2]={25,5};

const int nDatapoints = 50;
const int nDatapointsDS18B20 = 10;
uint16_t SERIALNUMBER[1];

File root;
int storedDataFiles = 0;

String DEFAULTDEVICENAME = "Satellit S0000";

String Address = "";
char* serialNumberChar = "";
String SerialNumberString = "";
char buffer[21];

volatile bool INIT=false;

#include "BLEDevice.h"
// The remote service we wish to connect to.
static BLEUUID serviceUUID("cddf1001-30f7-4671-8b43-5e40ba53514a"); //phyphox data service
static BLEUUID serviceHWUUID("cddf9021-30f7-4671-8b43-5e40ba53514a"); //phyphox hardware service

// The characteristic of the remote service we are interested in.
static BLEUUID    imuDataAccUUID("cddf1002-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    imuDataGyrUUID("cddf1003-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    imuConfigUUID("cddf1004-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    shtcDataUUID("cddf1005-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    shtcConfigUUID("cddf1006-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    bmpDataUUID("cddf1007-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    bmpConfigUUID("cddf1008-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    mlxDataUUID("cddf1009-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    mlxConfigUUID("cddf100a-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    lcDataUUID("cddf100b-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    lcConfigUUID("cddf100c-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    wlPressureDataUUID("cddf100d-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    wlPressureConfigUUID("cddf100e-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    tcDataUUID("cddf100f-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    tcConfigUUID("cddf1010-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    ds18b20DataUUID("cddf1011-30f7-4671-8b43-5e40ba53514a");
static BLEUUID    ds18b20ConfigUUID("cddf1012-30f7-4671-8b43-5e40ba53514a");

static BLEUUID    configHWUUID("cddf9022-30f7-4671-8b43-5e40ba53514a");

uint8_t configData[20] = {0};

int volatile dataPointsRequired =0;

bool deviceWorks[2] = {false};

int countFiles();
long volatile modeDataPoint = 0;
volatile bool measuring = false;
volatile bool snOnSDCard = false;

int volatile currentMode = 0;
bool doSomeStatistics(int dataPoints,int column, float target, float deviation);
void checkLimits();
void saveData(int);

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = true;

static BLERemoteCharacteristic* hwRemoteCharacteristic;

static BLERemoteCharacteristic* imuAccRemoteCharacteristic;
static BLERemoteCharacteristic* imuGyrRemoteCharacteristic;
static BLERemoteCharacteristic* imuConfigRemoteCharacteristic;

static BLERemoteCharacteristic* mlxRemoteCharacteristic;
static BLERemoteCharacteristic* mlxConfigRemoteCharacteristic;

static BLERemoteCharacteristic* bmpRemoteCharacteristic;
static BLERemoteCharacteristic* bmpConfigRemoteCharacteristic;

static BLERemoteCharacteristic* shtcRemoteCharacteristic;
static BLERemoteCharacteristic* shtcConfigRemoteCharacteristic;

static BLERemoteCharacteristic* tcRemoteCharacteristic;
static BLERemoteCharacteristic* tcConfigRemoteCharacteristic;

static BLERemoteCharacteristic* wlPressureRemoteCharacteristic;
static BLERemoteCharacteristic* wlPressureConfigRemoteCharacteristic;

static BLERemoteCharacteristic* ds18b20RemoteCharacteristic;
static BLERemoteCharacteristic* ds18b20ConfigRemoteCharacteristic;

static BLERemoteCharacteristic* lcRemoteCharacteristic;
static BLERemoteCharacteristic* lcConfigRemoteCharacteristic;


static BLEAdvertisedDevice* myDevice;
uint8_t dataReceived[20] = {0};

float D[nDatapoints]={0};
float T[nDatapoints]={0};

int volatile currentDatapoints =0;
const int maxFloatValues = 5;
float volatile dataArray[maxFloatValues][nDatapoints];

static void shtcCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == SHTC_MEASURE && currentDatapoints<nDatapoints){
    float myfloats[3];
    memcpy(&myfloats[0], pData, 12);
    if(printData){
      Serial.print("SHTC3: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.print(myfloats[1]);
      Serial.print(" ");
      Serial.println(myfloats[2]);
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    dataArray[2][currentDatapoints]=myfloats[2];
    currentDatapoints++;
    return;
  }
}
static void bmpCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == BMP_MEASURE && currentDatapoints<nDatapoints){
    float myfloats[3];
    memcpy(&myfloats[0], pData, 12);
    if(printData){
      Serial.print("BMP384: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.print(myfloats[1]);
      Serial.print(" ");
      Serial.println(myfloats[2]);
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    dataArray[2][currentDatapoints]=myfloats[2];
    currentDatapoints++;
    return;
  }
}
static void mlxCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == MLX_MEASURE && currentDatapoints<nDatapoints){
    float myfloats[4];
    memcpy(&myfloats[0], pData, 16);
    if(printData){
      Serial.print("MLX90393: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.print(myfloats[1]);
      Serial.print(" ");
      Serial.print(myfloats[2]);
      Serial.print(" ");
      Serial.println(myfloats[3]);      
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    dataArray[2][currentDatapoints]=myfloats[2];
    dataArray[3][currentDatapoints]=myfloats[3];
    dataArray[4][currentDatapoints]=sqrt(pow(myfloats[0],2)+pow(myfloats[1],2)+pow(myfloats[2],2));
    currentDatapoints++;
    return;
  }
}
static void imuCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == IMU_MEASURE && currentDatapoints<nDatapoints){
    float myfloats[7];
    memcpy(&myfloats[0], pData, 7*4);
    if(printData){
      Serial.print("IMU42605: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.print(myfloats[1]);
      Serial.print(" ");
      Serial.print(myfloats[2]);
      Serial.print(" ");
      Serial.println(myfloats[6]);      
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    dataArray[2][currentDatapoints]=myfloats[2];
    dataArray[3][currentDatapoints]=myfloats[6];
    dataArray[4][currentDatapoints]=sqrt(pow(myfloats[0],2)+pow(myfloats[1],2)+pow(myfloats[2],2));
    currentDatapoints++;
    return;
  }
}
static void tcCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == TC_MEASURE && currentDatapoints<nDatapoints){
    float myfloats[2];
    memcpy(&myfloats[0], pData, 2*4);
    if(printData){
      Serial.print("Thermocouple: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.println(myfloats[1]);    
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    currentDatapoints++;
    return;
  }
}
static void ds18b20Callback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == DS18B20_MEASURE && currentDatapoints<nDatapointsDS18B20){
    float myfloats[2];
    memcpy(&myfloats[0], pData, 2*4);
    if(printData){
      Serial.print("DS18B20: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.println(myfloats[1]);    
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    currentDatapoints++;
    return;
  }
}
static void mprlsCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData,size_t length,bool isNotify) {
  if(currentMode == MPRLS_MEASURE && currentDatapoints<nDatapoints){
    float myfloats[2];
    memcpy(&myfloats[0], pData, 2*4);
    if(printData){
      Serial.print("MPRLS: ");
      Serial.print(myfloats[0]);
      Serial.print(" ");
      Serial.println(myfloats[1]);    
      }
    dataArray[0][currentDatapoints]=myfloats[0];
    dataArray[1][currentDatapoints]=myfloats[1];
    currentDatapoints++;
    return;
  }
}
class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
      currentMode = 0;
      snOnSDCard=false;
    }

    void onDisconnect(BLEClient* pclient) {
      connected = false;
      currentMode = 0;
      Serial.println("disconnected");
    }
};


bool connectToServer() {
  Serial.print("Forming a connection to ");
  Address = myDevice->getAddress().toString().c_str();
  Serial.println(Address);
  Address.replace(":", "");

  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* dataRemoteService = pClient->getService(serviceUUID);
  BLERemoteService* hwRemoteService = pClient->getService(serviceHWUUID);
  if (hwRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceHWUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");



  // Obtain a reference to the characteristic in the service of the remote BLE server.
  imuAccRemoteCharacteristic = dataRemoteService->getCharacteristic(imuDataAccUUID);
  imuGyrRemoteCharacteristic = dataRemoteService->getCharacteristic(imuDataGyrUUID);
  imuConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(imuConfigUUID);

  shtcRemoteCharacteristic = dataRemoteService->getCharacteristic(shtcDataUUID);
  shtcConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(shtcConfigUUID);

  bmpRemoteCharacteristic = dataRemoteService->getCharacteristic(bmpDataUUID);
  bmpConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(bmpConfigUUID);
  
  mlxRemoteCharacteristic = dataRemoteService->getCharacteristic(mlxDataUUID);
  mlxConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(mlxConfigUUID);

  wlPressureRemoteCharacteristic = dataRemoteService->getCharacteristic(wlPressureDataUUID);
  wlPressureConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(wlPressureConfigUUID);

  ds18b20RemoteCharacteristic = dataRemoteService->getCharacteristic(ds18b20DataUUID);
  ds18b20ConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(ds18b20ConfigUUID);

  tcRemoteCharacteristic = dataRemoteService->getCharacteristic(tcDataUUID);
  tcConfigRemoteCharacteristic = dataRemoteService->getCharacteristic(tcConfigUUID);

  hwRemoteCharacteristic = hwRemoteService->getCharacteristic(configHWUUID);
  
  if (hwRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(configHWUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  connected = true;
}
/**
   Scan for BLE servers and find the first one that advertises the service we are looking for.
*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /**
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.print("BLE Advertised Device found: ");
      //Serial.println(advertisedDevice.toString().c_str());
      String currentName = advertisedDevice.getName().c_str();
      //advertisedDevice.toString().c_str();
      // We have found a device, let us now see if it contains the service we are looking for.
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceHWUUID) && currentName == DEFAULTDEVICENAME) {
      //  if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceHWUUID)) {
        BLEDevice::getScan()->stop();
        myDevice = new BLEAdvertisedDevice(advertisedDevice);
        doConnect = true;
        doScan = true;

      } // Found our server
    } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  
  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("");
  Serial.println("STARTE SATELLIT TESTCENTER");
  BLEDevice::init("");
  //pinMode(LED_BUILTIN,OUTPUT);
  pinMode(32,  OUTPUT );
  pinMode(33, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(26, OUTPUT);

  digitalWrite(32, 1);
  digitalWrite(33, 1);
  digitalWrite(25, 1);
  digitalWrite(26, 1);

  if (!SD.begin(4)) {
    Serial.println("sd initialization failed!");
    return;
  }
  storedDataFiles = countFiles();

  Serial.print(storedDataFiles);
  Serial.println(" txt files found");
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    Serial.println("");
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    if(currentMode==CONFIGMODE){
        getSN();
        currentMode= SHTC_INIT;
        return;
      }
    if(currentMode==SHTC_INIT){
      configData[0] = 1;
      configData[1] = 0x04;
      shtcConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = SHTC_MEASURE;  //start with SHTC3
      shtcRemoteCharacteristic->registerForNotify(shtcCallback);
      return;        
      }
    if(currentMode==SHTC_MEASURE){
      //Serial.println(currentDatapoints);
      if(currentDatapoints>=nDatapoints){
        configData[0] = 0;
        configData[1] = 0x04;
        shtcConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(3);
        currentMode = BMP_INIT;
        return;
        }
      return;
      }
    if(currentMode==BMP_INIT){
      configData[0] = 1;
      configData[1] = 0x03;
      configData[2] = 0x03;
      configData[3] = 0x03;
      
      bmpConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = BMP_MEASURE; 
      bmpRemoteCharacteristic->registerForNotify(bmpCallback);
      return;        
      }
    if(currentMode==BMP_MEASURE){
      if(currentDatapoints>=nDatapoints){
        configData[0] = 0;
        configData[1] = 0x00;
        bmpConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(3);
        currentMode = MLX_INIT;
        return;
        }
      return;
      }            
      
    if(currentMode==MLX_INIT){
      configData[0] = 1;
      configData[1] = 0x03;
      configData[2] = 0x06;
      configData[3] = 0x03;
      configData[4] = 0x01;
      configData[5] = 0x01;
      configData[6] = 0x01;
      configData[7] = 0x01;
      mlxConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = MLX_MEASURE; 
      mlxRemoteCharacteristic->registerForNotify(mlxCallback);
      return;        
      }
    if(currentMode==MLX_MEASURE){
      //Serial.println(currentDatapoints);
      if(currentDatapoints>=nDatapoints){
        configData[0] = 0;
        mlxConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(4);
        currentMode = IMU_INIT;
        return;
        }
      return;
      }
    if(currentMode==IMU_INIT){
      configData[0] = 0x01;
      configData[1] = 0x03;
      configData[2] = 0x0A;
      configData[3] = 0x00;
      configData[4] = 0x0A;
      
      imuConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = IMU_MEASURE; 
      imuAccRemoteCharacteristic->registerForNotify(imuCallback);
      return;        
      }
    if(currentMode==IMU_MEASURE){
      if(currentDatapoints>=nDatapoints){
        configData[0] = 0;
        imuConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(4);
        if(waermelehreConnected){
          currentMode = MPRLS_INIT;
        }else{
           currentMode = 99;
           uint8_t mySNArray[3];
            memcpy(&mySNArray[0],&SERIALNUMBER[0],2);
            mySNArray[2] = 1;
            hwRemoteCharacteristic->writeValue(&mySNArray[0], 3);   
        }
        
        return;
        }  
      return;
      }                 
    if(currentMode==MPRLS_INIT){
      configData[0] = 0x01;
      configData[1] = 0x14;
      wlPressureConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = MPRLS_MEASURE; 
      wlPressureRemoteCharacteristic->registerForNotify(mprlsCallback);
      return;        
      }
    if(currentMode==MPRLS_MEASURE){
      if(currentDatapoints>=nDatapoints){
        configData[0] = 0;
        wlPressureConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(2);
        currentMode = TC_INIT;
        return;
        }
      return;
      }
    if(currentMode==TC_INIT){
      configData[0] = 0x01;
      configData[1] = 0x01;
      tcConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = TC_MEASURE; 
      tcRemoteCharacteristic->registerForNotify(tcCallback);
      return;        
      }
    if(currentMode==TC_MEASURE){
      if(currentDatapoints>=nDatapoints){
        configData[0] = 0;
        tcConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(2);
        currentMode = DS18B20_INIT;
        return;
        }
      return;
      }
    if(currentMode==DS18B20_INIT){
      configData[0] = 0x01;
      configData[1] = 0x00;
      ds18b20ConfigRemoteCharacteristic->writeValue(&configData[0], 20);
      currentDatapoints=0;
      currentMode = DS18B20_MEASURE; 
      ds18b20RemoteCharacteristic->registerForNotify(ds18b20Callback);      
      return;        
      }
    if(currentMode==DS18B20_MEASURE){
      if(currentDatapoints>=nDatapointsDS18B20){
        configData[0] = 0;
        ds18b20ConfigRemoteCharacteristic->writeValue(&configData[0], 20);
        checkLimits();
        saveData(4);
        currentMode = 99;
        uint8_t mySNArray[3];
        memcpy(&mySNArray[0],&SERIALNUMBER[0],2);
        mySNArray[2] = 1;
        hwRemoteCharacteristic->writeValue(&mySNArray[0], 3); 
        return;
        }
      return;
      }                 
  } else if (doScan) {
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }

  delay(100); // Delay a second between loops.
} // End of loop


int countFiles() {
  root = SD.open("/satellit");
  root.rewindDirectory();
  int nFiles = 0;
  
  while (true) {
    File entry =  root.openNextFile();
    if (! entry) {
      // no more files
      
      break;
    }
    nFiles++;
    entry.close();
  }
  root.close();
  return nFiles;
}

String split(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void getSN(){
  root = SD.open("/satellit");
  root.rewindDirectory();       
  //search for file with address in name
  while(true){       
    File entry = root.openNextFile();
    if (!entry) {
          // no more files
          break;
        }
        
    char* filenameP;
    char filename[50];
    filenameP = (char*)entry.name();
    memcpy(&filename[0],filenameP,50);
    String filnameS = filename;
    String txtRemoved = split(filnameS,'.',0);
    String directoryRemoved = split(txtRemoved,'/',2);
    String SN = split(directoryRemoved,'_',0);
    String mac = split(directoryRemoved,'_',1);
          
    entry.close();
    if(Address == mac){
      Serial.println("");
      Serial.print("Data found on SD-Card for this Satellite-Box: \nSN: ");
      Serial.print(SN);
      Serial.print(" MAC: ");
      Serial.println(mac);
      Serial.println("");
      SerialNumberString = SN;
      snOnSDCard = true;

      root = SD.open("/satellit/" +SerialNumberString+"_"+ Address + ".txt", FILE_WRITE);
      root.close();
      break;
      }  
  }
  root.close();

  if(!snOnSDCard){
    Serial.println("no data on sd card for this Box available... creating a file..");
    Serial.print("New SN: ");
    sprintf(buffer, "S%04d", storedDataFiles+1);
    SerialNumberString = buffer;  
    root = SD.open("/satellit/" +SerialNumberString+"_"+ Address + ".txt", FILE_WRITE);
    root.close();
    Serial.println(SerialNumberString);
    }
  //send SERIALNUMBER to device
  String myStringBuffer = SerialNumberString;
  myStringBuffer.remove(0,1);
  SERIALNUMBER[0] = myStringBuffer.toInt();
  
  uint8_t mySNArray[3];
  memcpy(&mySNArray[0],&SERIALNUMBER[0],2);
  mySNArray[2] = 0;
  hwRemoteCharacteristic->writeValue(&mySNArray[0], 3);
    
}


void saveData(int spalten){
  root = SD.open("/satellit/"  +SerialNumberString+"_"+ Address +  ".txt", FILE_APPEND);
  int saveLength =nDatapoints;
  if(currentMode == DS18B20_MEASURE){
    saveLength = nDatapointsDS18B20;
    }
  //TODO
  if(currentMode == SHTC_MEASURE){
      root.println("SHTC3");
  }else if(currentMode == BMP_MEASURE){      
      root.println("BMP384");
  }else if(currentMode == MLX_MEASURE){      
      root.println("MLX90393");
  }else if(currentMode == IMU_MEASURE){      
      root.println("ICM42605");
  }else if(currentMode == DS18B20_MEASURE){      
      root.println("DS18B20");
  }else if(currentMode == TC_MEASURE){      
      root.println("Thermocouple");
  }else if(currentMode == MPRLS_MEASURE){      
      root.println("MPRLS");
  }
//float volatile dataArray[maxFloatValues][nDatapoints];

for(int i = 0;i<saveLength;i++){
    for(int spalte = 0; spalte<spalten;spalte++){
      root.print(dataArray[spalte][i],5);
      root.print('\t');
      }
    root.println("");
  }
  root.close(); 
}
bool doSomeStatistics(int dataPoints, int column, float target, float deviation){
  float median = 0;  
  float varianz = 0;  
  for(int i=0;i<dataPoints;i++){
    median += dataArray[column][i]/dataPoints;
  }          
 
  for(int i=0;i<dataPoints;i++){
    varianz+=(dataArray[column][i]-median)*(dataArray[column][i]-median)/(dataPoints-1);
  }
  
  Serial.printf("Median\t%f ± %f (%f ± %f)\n",median,sqrt(varianz)/sqrt(dataPoints),target,deviation);

  if(median>target-deviation && median<target+deviation){
      Serial.println("All good");
    }else{
      Serial.println("#### ERROR! ####  ");
      Serial.println("# CHECK VALUES #  ");
    }
  
  Serial.println();
  //Serial.printf("standard deviation %f (%f)\n",sqrt(varianz[0]),limitSTD);
  return 0;
}

void checkLimits(){
  //######### SHTC3 #########
  if(currentMode == SHTC_MEASURE){
    //check Temperature 
    Serial.println("SHTC3 Temperature");
    doSomeStatistics(nDatapoints,0,LIMITS_SHTC[0],LIMITS_SHTC[1]);
    //check humidity 
    Serial.println("SHTC3 Humidity");
    doSomeStatistics(nDatapoints,1,LIMITS_SHTC[2],LIMITS_SHTC[3]);
    return;
  }
  
  //######### BMP384 ########
  if(currentMode == BMP_MEASURE){
    //check Temperature 
    Serial.println("BMP384 Temperature");
    doSomeStatistics(nDatapoints,1,LIMITS_BMP384[0],LIMITS_BMP384[1]);
    //check pressure
    Serial.println("BMP384 Pressure");
    doSomeStatistics(nDatapoints,0,LIMITS_BMP384[2],LIMITS_BMP384[3]);
    return;
  }  
    
  //########## MLX ##########
  if(currentMode == MLX_MEASURE){
    //absolute magnetometer
    Serial.println("MLX90393");
    doSomeStatistics(nDatapoints,4,LIMITS_MLX[0],LIMITS_MLX[1]);
    return;
  }  
  //########## IMU ##########
  if(currentMode == IMU_MEASURE){
    //absolute acc
    Serial.println("ICM42605 Accelerometer (units in g)");
    doSomeStatistics(nDatapoints,4,LIMITS_IMU[0],LIMITS_IMU[1]);
    return;
  }
  //########## DS18 #########
  if(currentMode == DS18B20_MEASURE){
    Serial.println("Waermelehre Temperatur I");
    doSomeStatistics(nDatapointsDS18B20,0,LIMITS_DS18B20[0],LIMITS_DS18B20[1]);
    return;
  }  
  //########## TC ###########
  if(currentMode == TC_MEASURE){
    Serial.println("Waermelehre Temperatur II");
    doSomeStatistics(nDatapoints,0,LIMITS_TC[0],LIMITS_TC[1]);
    return;
  }    
  //######### MPRLS #########
  if(currentMode == MPRLS_MEASURE){
    Serial.println("Waermelehre Druck");
    doSomeStatistics(nDatapoints,0,LIMITS_MPRLS[0],LIMITS_MPRLS[1]);
    return;
  }   
  }
