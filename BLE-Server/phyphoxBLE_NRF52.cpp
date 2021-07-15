#include "phyphoxBLE_NRF52.h"
#include <cstdint>
#include <cstring>

const UUID PhyphoxBLE::phyphoxExperimentServiceUUID = UUID(phyphoxBleExperimentServiceUUID);
const UUID PhyphoxBLE::experimentCharacteristicUUID = UUID(phyphoxBleExperimentCharacteristicUUID);

const UUID PhyphoxBLE::phyphoxDataServiceUUID = UUID(phyphoxBleDataServiceUUID);

const UUID PhyphoxBLE::batteryServiceUUID = UUID(GattService::UUID_BATTERY_SERVICE);

const UUID PhyphoxBLE::icm42605DataCharacteristicUUID = UUID(phyphoxBleICM42605DataCharacteristicUUID);
const UUID PhyphoxBLE::icm42605ConfigCharacteristicUUID = UUID(phyphoxBleICM42605ConfigCharacteristicUUID);

const UUID PhyphoxBLE::shtc3DataCharacteristicUUID = UUID(phyphoxBleSHTC3DataCharacteristicUUID);
const UUID PhyphoxBLE::shtc3ConfigCharacteristicUUID = UUID(phyphoxBleSHTC3ConfigCharacteristicUUID);

const UUID PhyphoxBLE::ms5607DataCharacteristicUUID = UUID(phyphoxBleMS5607DataCharacteristicUUID);
const UUID PhyphoxBLE::ms5607ConfigCharacteristicUUID = UUID(phyphoxBleMS5607ConfigCharacteristicUUID);

const UUID PhyphoxBLE::mlx90393DataCharacteristicUUID = UUID(phyphoxBleMLX90393DataCharacteristicUUID);
const UUID PhyphoxBLE::mlx90393ConfigCharacteristicUUID = UUID(phyphoxBleMLX90393ConfigCharacteristicUUID);

const UUID PhyphoxBLE::loadcellDataCharacteristicUUID = UUID(phyphoxBleLOADCELLDataCharacteristicUUID);
const UUID PhyphoxBLE::loadcellConfigCharacteristicUUID = UUID(phyphoxBleLOADCELLConfigCharacteristicUUID);

const UUID PhyphoxBLE::thermocoupleDataCharacteristicUUID = UUID(phyphoxBleTHERMOCOUPLEDataCharacteristicUUID);
const UUID PhyphoxBLE::thermocoupleConfigCharacteristicUUID = UUID(phyphoxBleTHERMOCOUPLEConfigCharacteristicUUID);

const UUID PhyphoxBLE::ds18b20DataCharacteristicUUID = UUID(phyphoxBleDS18B20DataCharacteristicUUID);
const UUID PhyphoxBLE::ds18b20ConfigCharacteristicUUID = UUID(phyphoxBleDS18B20ConfigCharacteristicUUID);

const UUID PhyphoxBLE::mprlsDataCharacteristicUUID = UUID(phyphoxBleMPRLSDataCharacteristicUUID);
const UUID PhyphoxBLE::mprlsConfigCharacteristicUUID = UUID(phyphoxBleMPRLSConfigCharacteristicUUID);

uint16_t PhyphoxBLE::currentConnections =0;
char PhyphoxBLE::name[50] = "";


Thread PhyphoxBLE::bleEventThread;
Thread PhyphoxBLE::transferExpThread;

uint8_t PhyphoxBLE::data_package[50] = {0};
uint8_t PhyphoxBLE::config_package[CONFIGSIZE] = {0};

/*BLE stuff*/
BLE& bleInstance = BLE::Instance(BLE::DEFAULT_INSTANCE);
BLE& PhyphoxBLE::ble = bleInstance;

uint8_t PhyphoxBLE::readValue[DATASIZE] = {0};

//Characteristics (data+config) for
//  - IMU
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::icm42605DataCharacteristic{PhyphoxBLE::icm42605DataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::icm42605ConfigCharacteristic{PhyphoxBLE::icm42605ConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - SHTC3
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::shtc3DataCharacteristic{PhyphoxBLE::shtc3DataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::shtc3ConfigCharacteristic{PhyphoxBLE::shtc3ConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - MAGNETOMETER
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::ms5607DataCharacteristic{PhyphoxBLE::ms5607DataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::ms5607ConfigCharacteristic{PhyphoxBLE::ms5607ConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - MS5607
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::mlx90393DataCharacteristic{PhyphoxBLE::mlx90393DataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::mlx90393ConfigCharacteristic{PhyphoxBLE::mlx90393ConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - LOADCELL
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::loadcellDataCharacteristic{PhyphoxBLE::loadcellDataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::loadcellConfigCharacteristic{PhyphoxBLE::loadcellConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - MPRLS
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::mprlsDataCharacteristic{PhyphoxBLE::mprlsDataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::mprlsConfigCharacteristic{PhyphoxBLE::mprlsConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - THERMOCOUPLE
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::thermocoupleDataCharacteristic{PhyphoxBLE::thermocoupleDataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::thermocoupleConfigCharacteristic{PhyphoxBLE::thermocoupleConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

//  - DS18B20
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::ds18b20DataCharacteristic{PhyphoxBLE::ds18b20DataCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY}; //Note: Use { } instead of () google most vexing parse
ReadWriteArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::data_package)> PhyphoxBLE::ds18b20ConfigCharacteristic{PhyphoxBLE::ds18b20ConfigCharacteristicUUID, PhyphoxBLE::data_package, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};



//Experiment Transfer
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(PhyphoxBLE::readValue)> PhyphoxBLE::experimentCharacteristic{PhyphoxBLE::experimentCharacteristicUUID, PhyphoxBLE::readValue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY};

EventQueue PhyphoxBLE::queue{64 * EVENTS_EVENT_SIZE};
/*end BLE stuff*/
EventQueue PhyphoxBLE::transferQueue{64 * EVENTS_EVENT_SIZE};

PhyphoxBleEventHandler PhyphoxBLE::eventHandler(bleInstance);

GattCharacteristic* PhyphoxBLE::phyphoxCharacteristics[1] = {&PhyphoxBLE::experimentCharacteristic};
GattService PhyphoxBLE::phyphoxService{PhyphoxBLE::phyphoxExperimentServiceUUID, PhyphoxBLE::phyphoxCharacteristics, sizeof(PhyphoxBLE::phyphoxCharacteristics) / sizeof(GattCharacteristic *)};

BatteryService PhyphoxBLE::batteryService(ble,50);

GattCharacteristic* PhyphoxBLE::phyphoxDataCharacteristics[16] = {
                                                                &PhyphoxBLE::icm42605DataCharacteristic,
                                                                &PhyphoxBLE::icm42605ConfigCharacteristic,
                                                                &PhyphoxBLE::shtc3DataCharacteristic,
                                                                &PhyphoxBLE::shtc3ConfigCharacteristic,
                                                                &PhyphoxBLE::ms5607DataCharacteristic,
                                                                &PhyphoxBLE::ms5607ConfigCharacteristic,
                                                                &PhyphoxBLE::mlx90393DataCharacteristic,
                                                                &PhyphoxBLE::mlx90393ConfigCharacteristic,
                                                                &PhyphoxBLE::loadcellDataCharacteristic,
                                                                &PhyphoxBLE::loadcellConfigCharacteristic,
                                                                &PhyphoxBLE::mprlsDataCharacteristic,
                                                                &PhyphoxBLE::mprlsConfigCharacteristic,
                                                                &PhyphoxBLE::ds18b20DataCharacteristic,
                                                                &PhyphoxBLE::ds18b20ConfigCharacteristic,
                                                                &PhyphoxBLE::thermocoupleDataCharacteristic,
                                                                &PhyphoxBLE::thermocoupleConfigCharacteristic
                                                                };

GattService PhyphoxBLE::phyphoxDataService{PhyphoxBLE::phyphoxDataServiceUUID, PhyphoxBLE::phyphoxDataCharacteristics, sizeof(PhyphoxBLE::phyphoxDataCharacteristics) / sizeof(GattCharacteristic *)};

uint8_t* PhyphoxBLE::data = nullptr; //this pointer points to the data the user wants to write in the characteristic
uint8_t* PhyphoxBLE::config =nullptr;
uint8_t* PhyphoxBLE::p_exp = nullptr; //this pointer will point to the byte array which holds an experiment

uint8_t PhyphoxBLE::EXPARRAY[2000] = {0};// block some storage
size_t PhyphoxBLE::expLen = 0; //try o avoid this maybe use std::array or std::vector

void (*PhyphoxBLE::configHandler)() = nullptr;
void (*PhyphoxBLE::shtc3Handler)() = nullptr;
void (*PhyphoxBLE::mlxHandler)() = nullptr;
void (*PhyphoxBLE::bmpHandler)() = nullptr;
void (*PhyphoxBLE::imuHandler)() = nullptr;

void PhyphoxBleEventHandler::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
	//#ifndef NDEBUG
	//if(printer)
	//	printer -> println("Disconnection");
	//#endif
	ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    PhyphoxBLE::currentConnections= PhyphoxBLE::currentConnections -1;

}

void PhyphoxBleEventHandler::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{

    
	//#ifndef NDEBUG
	//if(printer)
	//	printer -> println("Connection with device");
	//#endif	
    PhyphoxBLE::currentConnections+=1;

}

#ifndef NDEBUG
void PhyphoxBLE::begin(HardwareSerial* hwPrint)
{
	 printer = hwPrint;
      if(printer)
		printer->begin(9600);       
}

void PhyphoxBLE::output(const char* s)
{
	if(printer)
		printer->println(s);
}

void PhyphoxBLE::output(const uint32_t num)
{
	if(printer)
		printer -> println(num);
}

#endif



void PhyphoxBLE::when_subscription_received(GattAttribute::Handle_t handle)
{
	#ifndef NDEBUG
	output("Received a subscription");
	#endif
	bool sub_dChar = false;
	ble.gattServer().areUpdatesEnabled(experimentCharacteristic, &sub_dChar);
	
	if(sub_dChar)
	{
		transferQueue.call(transferExp);
		sub_dChar = false;
	}
	//after experiment has been transfered start advertising again
	ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

}
void PhyphoxBLE::configReceived(const GattWriteCallbackParams *params)
{
    if (params->handle == PhyphoxBLE::shtc3ConfigCharacteristic.getValueHandle() ) {
        if(shtc3Handler != nullptr){
            transferQueue.call(shtc3Handler); 		
        }
    }
    if (params->handle == PhyphoxBLE::mlx90393ConfigCharacteristic.getValueHandle() ) {
        if(mlxHandler != nullptr){
            transferQueue.call(mlxHandler); 		
        }
    }
    if (params->handle == PhyphoxBLE::icm42605ConfigCharacteristic.getValueHandle() ) {
        if(imuHandler != nullptr){
            transferQueue.call(imuHandler); 		
        }
    }
    // BMP MISSING
}
void PhyphoxBLE::transferExp()
{
	BLE &ble = PhyphoxBLE::ble;
	uint8_t* exp = PhyphoxBLE::p_exp;
	size_t exp_len = PhyphoxBLE::expLen;

	#ifndef NDEBUG
	PhyphoxBLE::output("In transfer exp");
	#endif

	uint8_t header[20] = {0}; //20 byte as standard package size for ble transfer
	const char phyphox[] = "phyphox";
	uint32_t table[256];
	phyphoxBleCrc32::generate_table(table);
	uint32_t checksum = phyphoxBleCrc32::update(table, 0, exp, exp_len);
	#ifndef NDEBUG
	PhyphoxBLE::output("checksum: ");
	PhyphoxBLE::output(checksum);
	#endif
	size_t arrayLength = exp_len;
	uint8_t experimentSizeArray[4] = {0};
	experimentSizeArray[0]=  (arrayLength >> 24);
	experimentSizeArray[1]=  (arrayLength >> 16);
	experimentSizeArray[2]=  (arrayLength >> 8);
	experimentSizeArray[3]=  arrayLength; 

	uint8_t checksumArray[4] = {0};
	checksumArray[0]= (checksum >> 24) & 0xFF;
	checksumArray[1]= (checksum >> 16) & 0xFF;  
	checksumArray[2]= (checksum >> 8) & 0xFF;
	checksumArray[3]= checksum & 0xFF; 

	copy(phyphox, phyphox+7, header);
	copy(experimentSizeArray, experimentSizeArray+ 4, header + 7);
	copy(checksumArray, checksumArray +  4, header +11); 
	
	ble.gattServer().write(PhyphoxBLE::experimentCharacteristic.getValueHandle(), header, sizeof(header));
	//wait_us(30000);
    ThisThread::sleep_for(60ms);
	
	for(size_t i = 0; i < exp_len/20; ++i)
	{
		copy(exp+i*20, exp+i*20+20, header);
		ble.gattServer().write(PhyphoxBLE::experimentCharacteristic.getValueHandle(), header, sizeof(header));
		//wait_us(30000);
        ThisThread::sleep_for(60ms);
	}
	
	if(exp_len%20 != 0)
	{
		const size_t rest = exp_len%20;
		uint8_t slice[rest];
		copy(exp + exp_len - rest, exp + exp_len, slice);
		ble.gattServer().write(PhyphoxBLE::experimentCharacteristic.getValueHandle(), slice, sizeof(slice));
		//wait_us(30000);
        ThisThread::sleep_for(60ms);
	}
}

void PhyphoxBLE::bleInitComplete(BLE::InitializationCompleteCallbackContext* params)
{	
	
	ble.gattServer().onUpdatesEnabled(PhyphoxBLE::when_subscription_received);
	ble.gattServer().onDataWritten(PhyphoxBLE::configReceived);
	ble.gap().setEventHandler(&PhyphoxBLE::eventHandler);

	uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
	ble::AdvertisingDataBuilder adv_data_builder(_adv_buffer);
	ble::AdvertisingParameters adv_parameters(ble::advertising_type_t::CONNECTABLE_UNDIRECTED, ble::adv_interval_t(ble::millisecond_t(1000)));
	adv_data_builder.setFlags();
    adv_data_builder.setLocalServiceList(mbed::make_Span(&phyphoxExperimentServiceUUID, 1));
    ble_error_t error = adv_data_builder.setName(name);


  	#ifndef NDEBUG
    if(error == BLE_ERROR_BUFFER_OVERFLOW){
		output("BLE_ERROR_BUFFER_OVERFLOW");
    }else if(error == BLE_ERROR_NONE){
		output("BLE_ERROR_NONE");
    }else if(error == BLE_ERROR_INVALID_PARAM){
		output("BLE_ERROR_INVALID_PARAM");
    }
	#endif

    ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_parameters);
    ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE,adv_data_builder.getAdvertisingData());
    ble.gattServer().addService(phyphoxService);
    ble.gattServer().addService(phyphoxDataService);
    //ble.gattServer().addService(batteryService);
 
	ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
	#ifndef NDEBUG
	output("in init");
	#endif
}

void PhyphoxBLE::start(const char* DEVICE_NAME, uint8_t* exp_pointer, size_t len)
{
	/**
	 * \brief start the server and begin advertising
	 * This methods starts the server by. By specifying
	 * exp_pointer and len you can send your own phyphox
	 * experiment to your smartphone. Otherwise a default
	 * experiment will be sent. 
	 * \param exp_pointer a pointer to an experiment 
	 * \param len the len of the experiment 
	 */
	#ifndef NDEBUG
	output("In start");
	#endif

	strncpy(name, DEVICE_NAME, 49);
    name[50] = '\0';

	if(exp_pointer != nullptr){
		p_exp = exp_pointer;
		expLen = len;
	}else{
		
      PhyphoxBleExperiment defaultExperiment;

      //View
      PhyphoxBleExperiment::View firstView;

      //Graph
      PhyphoxBleExperiment::Graph firstGraph;      //Create graph which will plot random numbers over time     
      firstGraph.setChannel(0,1);    

      firstView.addElement(firstGraph);       
      defaultExperiment.addView(firstView);
      
      addExperiment(defaultExperiment);
      
	}

	bleEventThread.start(callback(&queue, &EventQueue::dispatch_forever));
  	transferExpThread.start(callback(&transferQueue, &EventQueue::dispatch_forever));
	ble.onEventsToProcess(PhyphoxBLE::schedule_ble_events);
	ble.init(PhyphoxBLE::bleInitComplete);
}

void PhyphoxBLE::start(uint8_t* exp_pointer, size_t len) {
    start("phyphox", exp_pointer, len);
}

void PhyphoxBLE::start(const char* DEVICE_NAME) {
    start(DEVICE_NAME, nullptr, 0);
}

void PhyphoxBLE::start() {
    start("phyphox", nullptr, 0);
}

void PhyphoxBLE::poll() {
}

void PhyphoxBLE::poll(int timeout) {
}
	
void PhyphoxBLE::write(float *value, uint8_t arrayLength, uint8_t sensorID)
{
    uint8_t dataBuf[4*arrayLength];
    memcpy(&dataBuf[0], value, 4*arrayLength);

    switch (sensorID) {
        case ID_ICM42605:
            ble.gattServer().write(icm42605DataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);
        case ID_MLX90393:
            ble.gattServer().write(mlx90393DataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);            
        case ID_MS5607:
            ble.gattServer().write(ms5607DataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);     
        case ID_SHTC3:
            ble.gattServer().write(shtc3DataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);     
        case ID_LOADCELL:
            ble.gattServer().write(loadcellDataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);   
        case ID_MPRLS:
            ble.gattServer().write(mprlsDataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);   
        case ID_THERMOCOUPLE:
            ble.gattServer().write(thermocoupleDataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);   
        case ID_DS18B20:
            ble.gattServer().write(ds18b20DataCharacteristic.getValueHandle(), &dataBuf[0], 4*arrayLength);                                                       
    }
	
	
}

void PhyphoxBLE::write(uint8_t *arrayPointer, unsigned int arraySize, uint8_t sensorID)
{
    switch (sensorID) {
        case ID_ICM42605:
            ble.gattServer().write(icm42605DataCharacteristic.getValueHandle(), arrayPointer, arraySize);
        case ID_MLX90393:
            ble.gattServer().write(mlx90393DataCharacteristic.getValueHandle(), arrayPointer, arraySize);
        case ID_MS5607:
            ble.gattServer().write(ms5607DataCharacteristic.getValueHandle(), arrayPointer, arraySize);
        case ID_SHTC3:
            ble.gattServer().write(shtc3DataCharacteristic.getValueHandle(), arrayPointer, arraySize); 
        case ID_LOADCELL:
            ble.gattServer().write(loadcellDataCharacteristic.getValueHandle(), arrayPointer, arraySize);      
        case ID_MPRLS:
            ble.gattServer().write(mprlsDataCharacteristic.getValueHandle(), arrayPointer, arraySize);  
        case ID_THERMOCOUPLE:
            ble.gattServer().write(thermocoupleDataCharacteristic.getValueHandle(), arrayPointer, arraySize);  
        case ID_DS18B20:
            ble.gattServer().write(ds18b20DataCharacteristic.getValueHandle(), arrayPointer, arraySize);              

    }
}

void PhyphoxBLE::read(uint8_t *arrayPointer, unsigned int arraySize, uint8_t sensorID)
{
	uint16_t myArraySize = arraySize;
    if(sensorID== ID_ICM42605){
        ble.gattServer().read(icm42605ConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_MLX90393){
        ble.gattServer().read(mlx90393ConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_MS5607){
        ble.gattServer().read(ms5607ConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_SHTC3){
        ble.gattServer().read(shtc3ConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_LOADCELL){
        ble.gattServer().read(loadcellConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_MPRLS){
        ble.gattServer().read(mprlsConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_THERMOCOUPLE){
        ble.gattServer().read(thermocoupleConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
    if(sensorID== ID_DS18B20){
        ble.gattServer().read(ds18b20ConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
    }
}
void PhyphoxBLE::readIMU(uint8_t *arrayPointer, unsigned int arraySize, uint8_t sensorID)
{
    uint16_t myArraySize = arraySize;
    ble.gattServer().read(icm42605ConfigCharacteristic.getValueHandle(), arrayPointer, &myArraySize);
}

void PhyphoxBLE::addExperiment(PhyphoxBleExperiment& exp)
{
	char buffer[2000] ="";
	exp.getBytes(buffer);
	memcpy(&EXPARRAY[0],&buffer[0],strlen(buffer));
	p_exp = &EXPARRAY[0];
	expLen = strlen(buffer);
}

void PhyphoxBLE::schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
    PhyphoxBLE::queue.call(mbed::Callback<void()>(&context->ble, &BLE::processEvents));
}
