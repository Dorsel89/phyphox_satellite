#ifndef PHYPHOXBLE_NRF52_H
#define PHYPHOXBLE_NRF52_H
#define NDEBUG

#include <phyphoxBle.h>
#include <mbed.h>
#include <ble/BLE.h>
#include <ble/GattServer.h>
#include <Gap.h>
#include <algorithm>
#include <UUID.h>
#include <string>
#include <AdvertisingParameters.h>
#include <AdvertisingDataBuilder.h>
#include "ble/services/BatteryService.h"

#include "phyphoxBleExperiment.h"

#ifndef NDEBUG
using arduino::HardwareSerial;
#endif
using events::EventQueue; 
using rtos::Thread;
using mbed::callback;
using std::copy;

#ifndef DATASIZE
#define DATASIZE 50
#endif


#define GAP_CON_MIN_INTERVAL 7.5
#define GAP_CON_MAX_INTERVAL 30
#define GAP_CON_SLAVE_LATENCY 0

class PhyphoxBleEventHandler : public ble::Gap::EventHandler {
    public:

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&);
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent&);
    PhyphoxBleEventHandler(BLE& ble):
        ble(ble) {
    }

    private: 
    BLE& ble;
};

class PhyphoxBLE
{
	
	private:

    static PhyphoxBleEventHandler eventHandler;
    
	static const UUID phyphoxExperimentServiceUUID;
	static const UUID phyphoxDataServiceUUID;
    static const UUID phyphoxHWConfigServiceUUID;

	static const UUID experimentCharacteristicUUID;
	
    static const UUID icm42605DataCharacteristicUUID;
	static const UUID shtc3DataCharacteristicUUID;
    static const UUID bmp384DataCharacteristicUUID;
    static const UUID mlx90393DataCharacteristicUUID;
    static const UUID loadcellDataCharacteristicUUID;
    static const UUID mprlsDataCharacteristicUUID;
    static const UUID thermocoupleDataCharacteristicUUID;
    static const UUID ds18b20DataCharacteristicUUID;
    
    static const UUID icm42605ConfigCharacteristicUUID;
	static const UUID shtc3ConfigCharacteristicUUID;
    static const UUID bmp384ConfigCharacteristicUUID;
    static const UUID mlx90393ConfigCharacteristicUUID;
    static const UUID loadcellConfigCharacteristicUUID;
    static const UUID thermocoupleConfigCharacteristicUUID;
    static const UUID ds18b20ConfigCharacteristicUUID;
    static const UUID mprlsConfigCharacteristicUUID;

    static const UUID batteryServiceUUID;

    static const UUID hwConfigCharacteristicUUID;
	static const UUID configCharacteristicUUID;

    static char name[50];

	static uint8_t data_package[182];
	static uint8_t config_package[CONFIGSIZE];

	/*BLE stuff*/
	
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> icm42605DataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> icm42605ConfigCharacteristic;

    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> shtc3DataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> shtc3ConfigCharacteristic;
    
    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> bmp384DataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> bmp384ConfigCharacteristic;

    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> mlx90393DataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> mlx90393ConfigCharacteristic;

    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> loadcellDataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> loadcellConfigCharacteristic;

    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> mprlsDataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> mprlsConfigCharacteristic;

    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> ds18b20DataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> ds18b20ConfigCharacteristic;

    static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> thermocoupleDataCharacteristic;
	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> thermocoupleConfigCharacteristic;

	static uint8_t readValue[DATASIZE];
	static ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(readValue)> experimentCharacteristic;

	static ReadWriteArrayGattCharacteristic<uint8_t, sizeof(data_package)> hwConfigCharacteristic;


	static Thread bleEventThread;
	static Thread transferExpThread;
	static EventQueue queue;
	/*end BLE stuff*/
	static EventQueue transferQueue;

	
	//helper function to initialize BLE server and for connection poperties
	static void bleInitComplete(BLE::InitializationCompleteCallbackContext*);
	//static void when_disconnection(const Gap::DisconnectionCallbackParams_t *);
	static void when_subscription_received(GattAttribute::Handle_t);
	static void configReceived(const GattWriteCallbackParams *params);

	//static void when_connected(const Gap::ConnectionCallbackParams_t *);
   
	//helper functon that runs in the thread ble_server
	//static void waitForEvent();
	static void transferExp();
	static GattCharacteristic* phyphoxCharacteristics[];
	static GattService phyphoxService;
    
	static GattCharacteristic* phyphoxDataCharacteristics[];
	static GattCharacteristic* phyphoxHWConfigCharacteristics[];

	static GattService phyphoxDataService;
    static GattService phyphoxHWConfigService;

	static void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context);

	#ifndef NDEBUG
	static inline HardwareSerial* printer; //for debug purpose
	#endif
	static uint8_t* data; //this pointer points to the data the user wants to write in the characteristic
	static uint8_t* config;
	static uint8_t* p_exp; //this pointer will point to the byte array which holds an experiment


	public:
    static BLE& ble;
    static BatteryService batteryService;
    static inline uint16_t minConInterval = 6;	//7.5ms
	static inline uint16_t maxConInterval = 20; //30ms
	static inline uint16_t slaveLatency = 0;
	static inline uint16_t timeout = 50;
	static uint16_t currentConnections;	

    
	static uint8_t EXPARRAY[2000];// block some storage
	static size_t expLen; //try o avoid this maybe use std::array or std::vector

	static void (*configHandler)();
    static void (*shtc3Handler)();
    static void (*mlxHandler)();
    static void (*ds18b20Handler)();
    static void (*thermocoupleHandler)();
    static void (*mprlsHandler)();
    static void (*bmpHandler)();
    static void (*imuHandler)();
    static void (*hwConfigHandler)();

    static void poll();
    static void poll(int timeout);

    static void start(const char* DEVICE_NAME, uint8_t* p, size_t n = 0); 
    static void start(const char* DEVICE_NAME);
    static void start(uint8_t* p, size_t n = 0); 
    static void start();

	static void write(uint8_t*, unsigned int, uint8_t);	
	static void write(float*, uint8_t, uint8_t);

	static void read(uint8_t*, unsigned int,uint8_t);
    static void readIMU(uint8_t*, unsigned int,uint8_t);
    
    static void readHWConfig(uint8_t*, unsigned int);

	static void addExperiment(PhyphoxBleExperiment&);
	#ifndef NDEBUG
	static void begin(HardwareSerial*); //for debug purpose
	static void output(const char*); //for debug purpose
	static void output(const uint32_t);
	#endif
};

#endif