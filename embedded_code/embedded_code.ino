#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <HIDTypes.h>

// Pin Definitions
#define MOUSE_LEFT_PIN 2
#define MOUSE_RIGHT_PIN 3
#define MOUSE_UP_PIN 4
#define MOUSE_DOWN_PIN 5

// Mouse movement step
#define MOUSE_MOVE_STEP 5

// Mouse object
BLECharacteristic* mouseInputChar;
BLECharacteristic* protocolModeChar;

// Button state variables
bool leftButtonState = false;
bool rightButtonState = false;

// Callback function for left button press
void leftButtonPress() {
  leftButtonState = true;
}

// Callback function for left button release
void leftButtonRelease() {
  leftButtonState = false;
}

// Callback function for right button press
void rightButtonPress() {
  rightButtonState = true;
}

// Callback function for right button release
void rightButtonRelease() {
  rightButtonState = false;
}

// Callback function for up button press
void upButtonPress() {
  uint8_t movement[] = {0, 0, MOUSE_MOVE_STEP, 0, 0, 0, 0, 0};
  mouseInputChar->setValue(movement, sizeof(movement));
  mouseInputChar->notify();
}

// Callback function for down button press
void downButtonPress() {
  uint8_t movement[] = {0, 0, -MOUSE_MOVE_STEP, 0, 0, 0, 0, 0};
  mouseInputChar->setValue(movement, sizeof(movement));
  mouseInputChar->notify();
}

// Callback function for disconnect event
class MyServerCallbacks : public BLEServerCallbacks {
  void onDisconnect(BLEServer* pServer) {
    // Stop mouse movement when disconnected
    uint8_t movement[] = {0, 0, 0, 0, 0, 0, 0, 0};
    mouseInputChar->setValue(movement, sizeof(movement));
    mouseInputChar->notify();
  }
};

void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);

  // Initialize BLE
  BLEDevice::init("ESP32-C3 Mouse");

  // Create BLE server
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  // Create HID service
  BLEService* hidService = server->createService(BLEUUID((uint16_t)0x1812));

  // Create HID mouse input characteristic
  mouseInputChar = hidService->createCharacteristic(
    BLEUUID((uint16_t)0x2A33),
    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );

  // Create protocol mode characteristic
  protocolModeChar = hidService->createCharacteristic(
    BLEUUID((uint16_t)0x2A4E),
    BLECharacteristic::PROPERTY_READ
  );
  uint8_t protocolMode[] = {0x01};
  protocolModeChar->setValue(protocolMode, sizeof(protocolMode));

  // Start HID service
  hidService->start();

  // Start advertising
  BLEAdvertising* advertising = server->getAdvertising();
  advertising->setAppearance(0x03C2); // Generic HID Device
  advertising->addServiceUUID(hidService->getUUID());
  advertising->start();

  // Register button press and release callbacks
  pinMode(MOUSE_LEFT_PIN, INPUT_PULLUP);
  pinMode(MOUSE_RIGHT_PIN, INPUT_PULLUP);
  pinMode(MOUSE_UP_PIN, INPUT_PULLUP);
  pinMode(MOUSE_DOWN_PIN, INPUT_PULLUP);
  attachInterrupt(MOUSE_LEFT_PIN, leftButtonPress, FALLING);
  attachInterrupt(MOUSE_LEFT_PIN, leftButtonRelease, RISING);
  attachInterrupt(MOUSE_RIGHT_PIN, rightButtonPress, FALLING);
  attachInterrupt(MOUSE_RIGHT_PIN, rightButtonRelease, RISING);
  attachInterrupt(MOUSE_UP_PIN, upButtonPress, FALLING);
  attachInterrupt(MOUSE_DOWN_PIN, downButtonPress, FALLING);
}

void loop() {
  // Do nothing
}
