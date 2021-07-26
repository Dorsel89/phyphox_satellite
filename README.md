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
0    | Bit 0: enable accelerometer <br> Bit 1: enable gyroscope <br> Bit 2: enable burst mode (TODO)<br>  Bit 3: rising edge (for burst mode) (TODO)<br>
1    | uint8_t gain-mode: <br> 0: 5x <br> 1: 4x <br> 2: 3x <br> 3: 2.5x <br> 4: 2x <br> 5: 1.667x <br> 6: 1.333x <br> 7: 1x
2    | uint8_t digital filter: 0-7
3    | uint8_t oversampling: 0-3
4    | uint8_t x resolution: 0-3
5    | uint8_t y resolution: 0-3
6    | uint8_t z resolution: 0-3
7-8  | int16_t threshold (µT) (TODO)
9-10 | int16_t datapoints (µT) (TODO)


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



