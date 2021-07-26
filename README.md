# phyphox satellite

1. [phyphox Experiments](#qr)
2. [Configuration](#config)
   * [TODO](#TODO)
3. [Specifications](#Specification)
4. [Test-Results](#Test-Results)

## 2. Configuration <a name="config"></a>
### Magnetometer MLX90393 ###
Byte | Settings
-----|---------
0    | Bit 0: enable magnetometer <br> Bit 1: enable burst mode (TODO) <br> Bit 2: rising edge (for burst mode) (TODO)
1    | uint8_t gain-mode: <br> 0: 5x <br> 1: 4x <br> 2: 3x <br> 3: 2.5x <br> 4: 2x <br> 5: 1.667x <br> 6: 1.333x <br> 7: 1x
2    | uint8_t digital filter: 0-7
3    | uint8_t oversampling: 0-3
4    | uint8_t x resolution: 0-3
5    | uint8_t y resolution: 0-3
6    | uint8_t z resolution: 0-3
7-8  | int16_t threshold (µT) (TODO)
9-10 | int16_t datapoints (µT) (TODO)

### Accelerometer/Gyroscope ICM42605 ###
Byte | Settings
-----|---------
0    | Bit 0: enable accelerometer <br> Bit 1: enable gyroscope <br> Bit 2: enable burst mode (TODO)<br>  Bit 3: rising edge (for burst mode) (TODO)
1    | uint8_t accelerometer range <br> 16G: 0 <br> 8G: 1 <br> 4G: 2 <br> 2G: 3
2    | uint8_t accelerometer rate <br> 8000Hz: 0x03 <br> 4000Hz: 0x04 <br> 2000Hz: 0x05 <br> 1000Hz: 0x06 <br> 200Hz: 0x07 <br> 100Hz: 0x08 <br> 50Hz: 0x09 <br> 25Hz: 0x0A <br> 6.25Hz: 0x0B <br> 6.25Hz: 0x0C <br> 3.12Hz: 0x0D <br> 1.5625Hz: 0x0E <br> 500Hz: 0x0F
3    | uint8_t gyroscope range <br> 2000DPS: 0x00 <br> 1000DPS: 0x01 <br> 500DPS: 0x02 <br> 250DPS: 0x03 <br> 125DPS: 0x04 <br> 62.5DPS: 0x05 <br> 31.25DPS: 0x06 <br> 15.125DPS: 0x07
4    | uint8_t gyroscope rate <br> 8000Hz: 0x03 <br> 4000Hz: 0x04 <br> 2000Hz: 0x05 <br> 1000Hz: 0x06 <br> 200Hz: 0x07 <br> 100Hz: 0x08 <br> 50Hz: 0x09 <br> 25Hz: 0x0A <br> 6.25Hz: 0x0B <br> 6.25Hz: 0x0C <br> 3.12Hz: 0x0D <br> 1.5625Hz: 0x0E <br> 500Hz: 0x0F

## 3. Specifications <a name="Specification"></a>

## 4. Test-Results <a name="Test-Results"></a>
### Current consumption

In most sensors are secondary sensors available which are used to calbrate the "main" sensor type (for example a temperature sensor in a magnetometer). These "secondary" type of sensors are written in *italic* <br>
<br>
Base current 0.84mA
Sensor      |Type   |Rate  | Current consumtion (mA)
------------|-------|------|-------------
MS5607  |Pressure, *Temperature*|  50Hz  |1.38
MLX90393 |Magnetometer, *Temperature*|    |
ICM42605  |Acceleration, Gyroscope, *Temperature*|    |
SHTC3  |Temperature, Humidity|    |



