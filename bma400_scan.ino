#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 1;
BLEScan *pBLEScan;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (!advertisedDevice.haveName() || 
        advertisedDevice.getName() != "BMA400_Data") {  
      return; 
    }

    if (advertisedDevice.haveName()) {
      const char* deviceName = advertisedDevice.getName().c_str();

      if (strncmp(deviceName, "BMA400_Data", strlen(deviceName)) == 0) {
        // Serial.printf("RSSI: %i\n", advertisedDevice.getRSSI());

        if (advertisedDevice.haveManufacturerData()) {
          String manufacturerDataString = advertisedDevice.getManufacturerData();
          int len = manufacturerDataString.length();

          if (len >= 9 && len <= 32) {
            uint8_t manufacturerData[32] = {0};  // zero-initialize
            memcpy(manufacturerData, manufacturerDataString.c_str(), len);

            uint16_t manufacturerID = (manufacturerData[1] << 8) | manufacturerData[0];
            Serial.printf("Manufacturer ID: %04X\n", manufacturerID);

            uint8_t voltageRaw = manufacturerData[2];
            float voltage = voltageRaw * 0.03125;
            Serial.printf("Voltage: %.2f V\n", voltage);

            int16_t xRaw = ((uint16_t)manufacturerData[4] << 8) | manufacturerData[3];
            int16_t yRaw = ((uint16_t)manufacturerData[6] << 8) | manufacturerData[5];
            int16_t zRaw = ((uint16_t)manufacturerData[8] << 8) | manufacturerData[7];

            // The raw values are 12-bit signed values, so we need to make sure negative values are correctly signed
            if(xRaw > 2047) xRaw -= 4096;
            if(yRaw > 2047) yRaw -= 4096;
            if(zRaw > 2047) zRaw -= 4096;

            // Convert the raw values to acceleration in g's
            // BMA400 defaults to +/- 4g over the whole 12-bit range
            float xAcc = xRaw * 4.0 / pow(2, 11);
            float yAcc = yRaw * 4.0 / pow(2, 11);
            float zAcc = zRaw * 4.0 / pow(2, 11);

            // Now print out the x/y/z acceleration values
            Serial.printf("X: %.2f Y: %.2f Z: %.2f\n", xAcc, yAcc, zAcc);

            // Temperature (2 bytes), BIG-endian
            int16_t tempRaw = ((uint16_t)manufacturerData[9] << 8) | manufacturerData[10];
            float temperature = tempRaw / 100.0;  // unit = 0.01°C
            Serial.printf("Temperature: %.2f °C\n", temperature);
            // Serial.printf("Voltage: %.2f\tX: %.2f\tY: %.2f\tZ: %.2f\tTemp: %.2f\n", voltage, xAcc, yAcc, zAcc, temperature);
          } else {
            Serial.println("Manufacturer data too short or too long!");
          }
        }
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  // Serial.println("BLE scan sketch begin!");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(50);
  pBLEScan->setWindow(49);
  // Start continuous scanning (non-blocking, infinite)
  // pBLEScan->start(scanTime, nullptr);
}

void loop() {
  pBLEScan->start(scanTime, false); // Blocking scan for scanTime seconds
  pBLEScan->clearResults();  // optional: clear memory
  delay(100); // small pause before next scan
  // pBLEScan->start(scanTime, false);  // just start the scan
  // Serial.println("Scan done!");
  // delay(500);
}

