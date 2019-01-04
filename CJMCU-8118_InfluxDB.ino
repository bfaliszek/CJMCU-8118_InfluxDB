/***************************************************

  Wemos D1 mini or NodeMCU 1.0
  VCC - 3.3V
  GND - G
  SCL - D1 -- GPIO 5
  SDA - D2 -- GPIO 4
  WAK - D3 -- GPIO 0

 ****************************************************/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>

#include "src/ClosedCube_HDC1080.h" // HDC1080 library - https://github.com/closedcube/ClosedCube_HDC1080_Arduino
#include "src/ccs811.h"  // CCS811 library - https://github.com/maarten-pennings/CCS811

#include "src/ESPinfluxdb.h" // https://github.com/hwwong/ESP_influxdb

// ********************** Config **********************

// DeepSleep time – send data every 60 seconds
const int sleepTimeS = 60;

CCS811 ccs811(D3); // nWAKE on D3
ClosedCube_HDC1080 hdc1080;

// InfluxDB – Config
#define INFLUXDB_HOST "INFLUXDB_ADRESS"
#define INFLUXDB_PORT 8086

#define DATABASE  "mydb"
#define DB_USER "mydb_username"
#define DB_PASSWORD "mydb_password"
#define DEVICE_NAME "CJMCU-8118"

// WiFi Config
#define WiFi_SSID "WiFi_SSID"
#define WiFi_Password "WiFi_Password"


// ******************** Config End ********************

ESP8266WiFiMulti WiFiMulti;

Influxdb influxdb(INFLUXDB_HOST, INFLUXDB_PORT);

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("");

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WiFi_SSID, WiFi_Password);
  Serial.println();
  Serial.print("Waiting for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // hdc1080 info
  hdc1080.begin(0x40);
  Serial.print("Manufacturer ID=0x");
  Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
  Serial.print("Device ID=0x");
  Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device

  // Enable I2C for ESP8266 NodeMCU boards [VDD to 3V3, GND to GND, nWAKE to D3, SDA to D2, SCL to D1]
  //Wire.begin(4, 5);
  Wire.begin();

  Serial.println("CCS811 test");
  // Enable CCS811
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok = ccs811.begin();
  if ( !ok ) Serial.println("setup: CCS811 begin FAILED");

  // Print CCS811 versions
  Serial.print("setup: hardware    version: "); Serial.println(ccs811.hardware_version(), HEX);
  Serial.print("setup: bootloader  version: "); Serial.println(ccs811.bootloader_version(), HEX);
  Serial.print("setup: application version: "); Serial.println(ccs811.application_version(), HEX);

  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if ( !ok ) Serial.println("init: CCS811 start FAILED");

  if (influxdb.opendb(DATABASE) != DB_SUCCESS) {
    Serial.println("Connecting to database failed");
  }
}

void loop()
{
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2, &etvoc, &errstat, &raw);
  if ( errstat == CCS811_ERRSTAT_OK ) {

    Serial.print("\n\ntemperature: ");
    Serial.print(hdc1080.readTemperature());
    Serial.print(" C");

    Serial.print("\nhumidity: ");
    Serial.print(hdc1080.readHumidity());
    Serial.print(" %");

    Serial.print("\neCO2 concentration: ");
    Serial.print(eco2);
    Serial.print(" ppm");

    Serial.print("\nTVOC concentration: ");
    Serial.print(etvoc);
    Serial.print(" ppb");

    dbMeasurement row(DEVICE_NAME);
    row.addField("temperature", (int(hdc1080.readTemperature())));
    row.addField("humidity", (int(hdc1080.readHumidity())));
    row.addField("eCO2", (int(eco2)));
    row.addField("TVOC", (int(etvoc)));
    if (influxdb.write(row) == DB_SUCCESS) {
      Serial.println("\n\nData send to InfluxDB");
    }
    row.empty();
  } else if ( errstat == CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
  } else if ( errstat & CCS811_ERRSTAT_I2CFAIL ) {
    Serial.println("CCS811: I2C error");
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat, HEX);
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) );
  }
  Serial.println();

  Serial.println("Waiting...");
  delay(sleepTimeS * 1000);

}
