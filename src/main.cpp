#include <Arduino.h>
#include <ArduinoBLE.h>
#include "wing_motion.h"
#include "serial_config.h"
#include "wing_config.h"

BLEService ornithopterService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEByteCharacteristic commandCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup() {
    initWingMotion();
    Serial.begin(115200);

    if (!BLE.begin()) {
        Serial.println("Starting BLE failed!");
        while (1);
    }

    BLE.setLocalName("LenGreenMoth");
    BLE.setAdvertisedService(ornithopterService);
    ornithopterService.addCharacteristic(commandCharacteristic);
    BLE.addService(ornithopterService);
    commandCharacteristic.writeValue(0);
    BLE.advertise();
    
    Serial.println("BLE & Motion Controller Active. Waiting for connections...");
}

void loop() {
    BLEDevice central = BLE.central();

    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());

        while (central.connected()) {
            // 1. Check for Bluetooth commands
            if (commandCharacteristic.written()) {
                byte currentCommand = commandCharacteristic.value();
                
                switch (currentCommand) {
                    case 0: // STOP
                        WING_CONFIG.LEFT_AMPLITUDE_MULTIPLIER = 0.0;
                        WING_CONFIG.RIGHT_AMPLITUDE_MULTIPLIER = 0.0;
                        break;
                    case 1: // LIFTING (Straight)
                        WING_CONFIG.LEFT_AMPLITUDE_MULTIPLIER = 1.0;
                        WING_CONFIG.RIGHT_AMPLITUDE_MULTIPLIER = 1.0;
                        break;
                    case 2: // TURN LEFT
                        WING_CONFIG.LEFT_AMPLITUDE_MULTIPLIER = 0.5;
                        WING_CONFIG.RIGHT_AMPLITUDE_MULTIPLIER = 1.0;
                        break;
                    case 3: // TURN RIGHT
                        WING_CONFIG.LEFT_AMPLITUDE_MULTIPLIER = 1.0;
                        WING_CONFIG.RIGHT_AMPLITUDE_MULTIPLIER = 0.5;
                        break;
                }
            }
            
            // 2. Keep the wings moving using your team's kinematics library
            updateWingMotion();
            
            // 3. Process any serial monitor inputs
            processSerialConfig();
        }

        // Failsafe if Bluetooth disconnects
        Serial.println("Disconnected! Failsafe triggered.");
        WING_CONFIG.LEFT_AMPLITUDE_MULTIPLIER = 0.0;
        WING_CONFIG.RIGHT_AMPLITUDE_MULTIPLIER = 0.0;
        commandCharacteristic.writeValue(0);
    }

    // Ensure the system handles failsafe state and serial commands when disconnected
    updateWingMotion();
    processSerialConfig();
}