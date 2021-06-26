

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS     1000

PulseOximeter pox;

uint32_t tsLastReport = 0;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;
int j = 0;
int len = 0;
char buf[100];
char buf_serialinput[100];

const int averageSize = 3;
double average[averageSize] = {60};

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "d5875408-fa51-4763-a75d-7d33cecebc31"
#define CHARACTERISTIC_UUID "a4f01d8c-a037-43b6-9050-1876a8c23584"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string j = pCharacteristic->getValue();
      int len = j.length();
      Serial.println(len);
      Serial.println(j.c_str());
    }
};
MyCallbacks myCallbacks;

void onBeatDetected()
{
    Serial.println("Beat!");
    if (deviceConnected) {
       pCharacteristic->notify();
       pCharacteristic->indicate(); 
    }
}

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("NefryBT");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->setCallbacks(&myCallbacks);
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
      pox.update();
      
      int index = 0;
      bool hasData = false;
      

      if (deviceConnected) {
        if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
            Serial.print("Heart rate:");
            Serial.print(pox.getHeartRate());
            Serial.print("bpm / SpO2:");
            Serial.print(pox.getSpO2());
            Serial.println("%");
            char buffer[32];
    //      int random_num = random(255);
            sprintf(buffer, "%d", (int)pop(pox.getHeartRate()));
//          Serial.printf("%d\n", random_num);
            pCharacteristic->setValue(buffer);
    //      value++;
          tsLastReport = millis();
        }
     }  
//  delay(1000);
}

double pop(double value) {
  double ans = 0;
  for (int i = 1; i < averageSize; i++){
     average[i-1] = average[i];
     ans += average[i];
  }
  ans += value;
  average[averageSize - 1] = value;
  return ans / (double)averageSize;
}
