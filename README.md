# phyphox satellite

1. [phyphox Experiments](#qr)
2. [Configuration](#config)
   * [Satellite Magnetometer MLX90393](#MLX90393)
   * [Satellite Accelerometer/Gyroscope ICM42605](#ICM42605)
   * [Satellite Temperature/Humidity SHTC3](#SHTC3)
   * [Satellite Pressure/Temperature BMP384](#BMP384)
   * [Thermodynamics Thermocouple](#Thermocouple)
   * [Thermodynamics 1Wire DS18B20](#DS18B20)
   * [Thermodynamics MPRLS](#MPRLS)
3. [Specifications](#Specification)
4. [Test-Results](#Test-Results)

## 1. Configuration <a name="config"></a>

**BMP384 Pressure/Temperature** <br>
![phyphox Experiments](experiments/bmp384.png?raw=true "BMP384")

**MLX90393 Magnetic field** <br>
![phyphox Experiments](experiments/mlx90393.png?raw=true "MLX90393")

**ICM42605 Acc/Gyr** <br>
![phyphox Experiments](experiments/icm42605.png?raw=true "ICM42605")

**SHTC3 Temperature/Humidity** <br>
![phyphox Experiments](experiments/shtc3.png?raw=true "SHTC3")

**DS18B20 Temperature I** <br>
![phyphox Experiments](experiments/ds18b20.png?raw=true "DS18B20")

**Thermocouple Temperature II** <br>
![phyphox Experiments](experiments/thermocouple.png?raw=true "Thermocouple")

**MPRLS Druck** <br>
![phyphox Experiments](experiments/mprls.png?raw=true "MPRLS")


## 2. Configuration <a name="config"></a>
### Satellite  Magnetometer MLX90393 <a name="MLX90393"></a> ###
charateristic | uuid
--------------|-----
data (repeatable)         | cddf1009-30f7-4671-8b43-5e40ba53514a
byte 0-3          | x type: float
byte 4-7          | y type: float
byte 8-11         | z type: float
byte 12-15        | timestamp type: float

Byte | Settings
-----|---------
config | cddf100a-30f7-4671-8b43-5e40ba53514a
0    | 0x00 disable magnetometer <br> 0x01 enable magnetometer
1    | gain-mode: <br> 0x00: 5x <br> 0x01: 4x <br> 0x02: 3x <br> 0x03: 2.5x <br> 0x04: 2x <br> 0x05: 1.667x <br> 0x06: 1.333x <br> 0x07: 1x
2    | uint8_t digital filter: 0-7
3    | uint8_t oversampling: 0-3
4    | uint8_t x resolution: 0-3
5    | uint8_t y resolution: 0-3
6    | uint8_t z resolution: 0-3
7    | uint8_t number of datasets per packages

Output data rate as a function of oversampling rate and digital filter: <br>
![Output Data Rate](https://rwth-aachen.sciebo.de/s/VrZH27MGOftCfsH/download)


Sensitivity table for given gain and resolution selection (at 25°C): <br>
![Sensitivity](https://rwth-aachen.sciebo.de/s/o0M256IjcfFzzC7/download)


### Satellite  Accelerometer/Gyroscope ICM42605 <a name="ICM42605"></a> ### 
charateristic | uuid
--------------|-----
acc data          | cddf1002-30f7-4671-8b43-5e40ba53514a
byte 0-3          | x type: float
byte 4-7          | y type: float
byte 8-11         | z type: float
byte 12-15        | timestamp type: float
------------------|------------------
gyr data          | cddf1003-30f7-4671-8b43-5e40ba53514a
byte 0-3          | x type: float
byte 4-7          | y type: float
byte 8-11         | z type: float
byte 12-15        | timestamp type: float

Byte | Settings
-----|---------
config | cddf1004-30f7-4671-8b43-5e40ba53514a
0    | 0x00: disable IMU <br> 0x01: enable IMU
1    | gyroscope range <br> 2000DPS: 0x00 <br> 1000DPS: 0x01 <br> 500DPS: 0x02 <br> 250DPS: 0x03 <br> 125DPS: 0x04 <br> 62.5DPS: 0x05 <br> 31.25DPS: 0x06 <br> 15.125DPS: 0x07
2    | accelerometer range <br> 16G: 0x00 <br> 8G: 0x01 <br> 4G: 0x02 <br> 2G: 0x03
3    | accelerometer rate <br> 8000Hz: 0x03 <br> 4000Hz: 0x04 <br> 2000Hz: 0x05 <br> 1000Hz: 0x06 <br> 200Hz: 0x07 <br> 100Hz: 0x08 <br> 50Hz: 0x09 <br> 25Hz: 0x0A <br> 6.25Hz: 0x0B <br> 6.25Hz: 0x0C <br> 3.12Hz: 0x0D <br> 1.5625Hz: 0x0E <br> 500Hz: 0x0F
4    | number of datasets per packages (1-8)
### Satellite  Temperature/Humidity SHTC3  <a name="SHTC3"></a> ###
charateristic | uuid
--------------|-----
data          | cddf1005-30f7-4671-8b43-5e40ba53514a
byte 0-3          | temperature type: float
byte 4-7          | humidity type: float
byte 8-11         | timestamp type: float
config        | cddf1006-30f7-4671-8b43-5e40ba53514a

Byte | Settings
-----|---------
0    | uint8_t enable sensor
1    | uint8_t measurement interval in 10ms (0x03 equals 30ms measurement interval)

### Satellite  Pressure/Temperature BMP384  <a name="BMP384"></a> ###
charateristic | uuid
--------------|-----
data          | cddf1007-30f7-4671-8b43-5e40ba53514a
byte 0-3          | pressure type: float
byte 4-7          | temperature type: float
byte 8-11         | timestamp type: float

Byte | Settings
-----|---------
config        | cddf1008-30f7-4671-8b43-5e40ba53514a
0    | 0x00: disable BMP384 <br>  0x01 enable BMP384
1    | pressure oversampling: <br> no oversampling 0x00 <br> 2x: 0x01 <br> 4x: 0x02 <br> 8x: 0x03 <br> 16x: 0x04 <br> 32x: 0x05 <br>
3    | iir filter coefficient: <br> disable: 0x00 <br> 1: 0x01 <br> 3: 0x02 <br> 7: 0x03 <br> 15: 0x04 <br> 31: 0x05 <br> 63: 0x06 <br> 127: 0x07
4    | data rate: <br> 200Hz: 0x00 <br> 100Hz: 0x01 <br> 50Hz: 0x02 <br> 25Hz: 0x03 <br> 12.5Hz: 0x04 <br> 6.25Hz: 0x05 <br> 3.1Hz: 0x06 <br> 1.5Hz: 0x07 <br> 0.78Hz: 0x08 <br> 0.39Hz: 0x09 <br> 0_2Hz: 0x0A

![Sensitivity](https://rwth-aachen.sciebo.de/s/aUygVN1HklB3eOO/download)


### Thermodynamics Thermocouple  <a name="Thermocouple"></a> ###
charateristic | uuid
--------------|-----
data          | cddf100f-30f7-4671-8b43-5e40ba53514a
byte 0-3          | temperature type: float
byte 4-7          | timestamp type: float
config        | cddf1010-30f7-4671-8b43-5e40ba53514a

Byte | Settings
-----|---------
0    | uint8_t enable sensor
1    | uint8_t measurement interval in 100ms (0x03 equals 300ms measurement interval)

### Thermodynamics 1Wire DS18B20  <a name="DS18B20"></a> ###
charateristic | uuid
--------------|-----
data          | cddf1011-30f7-4671-8b43-5e40ba53514a
byte 0-3          | temperature type: float
byte 4-7          | timestamp type: float
config        | cddf1012-30f7-4671-8b43-5e40ba53514a

Byte | Settings
-----|---------
0    | uint8_t enable sensor
1    | uint8_t measurement interval in 1000ms (0x03 equals 3000ms measurement interval, 0x00 represents the minimal measurement interval of 750ms)

### Thermodynamics MPRLS  <a name="MPRLS"></a> ###
charateristic | uuid
--------------|-----
data          | cddf100d-30f7-4671-8b43-5e40ba53514a
byte 0-3          | pressure type: float
byte 4-7          | timestamp type: float
config        | cddf100e-30f7-4671-8b43-5e40ba53514a

Byte | Settings
-----|---------
0    | uint8_t enable sensor
1    | uint8_t measurement interval in 10ms (0x03 equals 30ms measurement interval)

## 3. Specifications <a name="Specification"></a>

## 4. Test-Results <a name="Test-Results"></a>
### Current consumption

In most sensors are secondary sensors available which are used to calbrate the "main" sensor type (for example a temperature sensor in a magnetometer). These "secondary" type of sensors are written in *italic* <br>
=======
   * [TODO](#TODO)
3. [Specifications](#Specification)
4. [Test-Results](#Test-Results)

## phyphox Experiments <a name="qr"></a>

## Config <a name="config"></a>
All data in float32LittleEndian

Sensor | Data  | byte offset 
-------|-------|--------
ICM42605|Acceleration X  |0
ICM42605 | Acceleration y  |4
ICM42605| Acceleration z  |8
ICM42605| Gyroscope x     |12
ICM42605| Gyroscope y     |12
ICM42605| Gyroscope Z     |12
ICM42605| *Temperature*    |16
MLX90393| Magnetometer x    |20
MLX90393| Magnetometer y    |24
MLX90393| Magnetometer z    |28
MLX90393| *Temperature*    |32
SHTC3| Humidity    |36
SHTC3| Temperature    |40
MS5607| Pressure    |44
MS5607| *Temperature*    |48


## Specifications <a name="Specification"></a>

## Test-Results <a name="Test-Results"></a>
### Current consumption

In most sensors are secondary sensors available which are used to calbrate the "main" sensor type (for example a temperature sensor in a magnetometer). These "secondary" type of sensors are written in italic <br>
<br>
Base current 0.84mA
Sensor      |Type   |Rate  | Current consumtion (mA)
------------|-------|------|-------------
MS5607  |Pressure, *Temperature*|  50Hz  |1.38
MLX90393 |Magnetometer, *Temperature*|    |
ICM42605  |Acceleration, Gyroscope, *Temperature*|    |
SHTC3  |Temperature, Humidity|    |



