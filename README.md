# phyphox satellite

1. [phyphox Experiments](#qr)
2. [Configuration](#config)
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



