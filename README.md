# CJMCU-8118 InfluxDB
Sending measurement data(eCO2, TVOC, temperature and humidity) from CJMCU-8118(cheap chinese board with CCS811 and HDC1080) connected to ESP8266(Wemos D1, NodeMCU etc.) to your InfluxDB database.

Based on [CCS811 library](https://github.com/maarten-pennings/CCS811), [ClosedCube_HDC1080 library](https://github.com/closedcube/ClosedCube_HDC1080_Arduino) and [ESP8266Influxdb](https://github.com/hwwong/ESP8266Influxdb).

### Wiring
 Wemos D1 or NodeMCU 1.0:
 * VCC - 3.3V
 * GND - G
 * SCL - D1
 * SDA - D2
 * WAK - D3

![CJMCU-8118 CCS811 HDC1080](https://raw.githubusercontent.com/bfaliszek/CJMCU-8118_InfluxDB/master/CJMCU-8118_CCS811_HDC1080.jpg)
