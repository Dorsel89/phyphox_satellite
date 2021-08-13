#ifndef PHYPHOXBLE
#define PHYPHOXBLE
#include <stdint.h>
#include <stdlib.h>

#define ID_ICM42605 1
#define ID_SHTC3 2
#define ID_BMP384 3
#define ID_MLX90393 4
#define ID_LOADCELL 5
#define ID_MPRLS 6
#define ID_THERMOCOUPLE 7
#define ID_DS18B20 8

static const char *phyphoxBleExperimentServiceUUID = "cddf0001-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleExperimentCharacteristicUUID = "cddf0002-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleExperimentControlCharacteristicUUID = "cddf0003-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleDataServiceUUID = "cddf1001-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleICM42605DataCharacteristicUUID = "cddf1002-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleICM42605ConfigCharacteristicUUID = "cddf1003-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleSHTC3DataCharacteristicUUID = "cddf1004-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleSHTC3ConfigCharacteristicUUID = "cddf1005-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleBMP384DataCharacteristicUUID = "cddf1006-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleBMP384ConfigCharacteristicUUID = "cddf1007-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleMLX90393DataCharacteristicUUID = "cddf1008-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleMLX90393ConfigCharacteristicUUID = "cddf1009-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleLOADCELLDataCharacteristicUUID = "cddf100a-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleLOADCELLConfigCharacteristicUUID = "cddf100b-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleMPRLSDataCharacteristicUUID = "cddf100c-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleMPRLSConfigCharacteristicUUID = "cddf100d-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleTHERMOCOUPLEDataCharacteristicUUID = "cddf100e-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleTHERMOCOUPLEConfigCharacteristicUUID = "cddf100f-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleDS18B20DataCharacteristicUUID = "cddf1010-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleDS18B20ConfigCharacteristicUUID = "cddf1011-30f7-4671-8b43-5e40ba53514a";

static const char *phyphoxBleHWConfigServiceUUID = "cddf9021-30f7-4671-8b43-5e40ba53514a";
static const char *phyphoxBleHWConfigCharacteristicUUID = "cddf9022-30f7-4671-8b43-5e40ba53514a";

#ifndef CONFIGSIZE
#define CONFIGSIZE 20
#endif

#include "phyphoxBLE_NRF52.h"

struct phyphoxBleCrc32
{
    static void generate_table(uint32_t(&table)[256])
    {
        uint32_t polynomial = 0xEDB88320;
        for (uint32_t i = 0; i < 256; i++) 
        {
            uint32_t c = i;
            for (size_t j = 0; j < 8; j++) 
            {
                if (c & 1) {
                    c = polynomial ^ (c >> 1);
                }
                else {
                    c >>= 1;
                }
            }
            table[i] = c;
        }
    }

    static uint32_t update(uint32_t (&table)[256], uint32_t initial, const uint8_t* buf, size_t len)
    {
        uint32_t c = initial ^ 0xFFFFFFFF;
        const uint8_t* u = static_cast<const uint8_t*>(buf);
        for (size_t i = 0; i < len; ++i) 
        {
            c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
        }
        return c ^ 0xFFFFFFFF;
    }
};


#endif