/*
    BLE_MIDI Example by neilbags
    
    Based on BLE_notify example by Evandro Copercini.

    Creates a BLE MIDI service and characteristic.
    Once a client subscibes, send a MIDI message every 2 seconds
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

uint8_t midiPacket[] = {
   0x80,  // header
   0x80,  // timestamp, not implemented 
   0x00,  // status
   0x3c,  // 0x3c == 60 == middle c
   0x00   // velocity
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32 MIDI Example");

  // Create the BLE Server
  BLEServer *pServer = new BLEServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create the BLE Service
  BLEService *pService = pServer->createService(BLEUUID(MIDI_SERVICE_UUID));
  
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(MIDI_CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

void loop() {
  if (deviceConnected) {

   // note down
   midiPacket[2] = 0x90; // note down, channel 0
   midiPacket[4] = 127;  // velocity
   pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
   pCharacteristic->notify();

   // play note for 500ms
   delay(500);

   // note up
   midiPacket[2] = 0x80; // note up, channel 0
   midiPacket[4] = 0;    // velocity
   pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
   pCharacteristic->notify();

   delay(2000);
  }
}
