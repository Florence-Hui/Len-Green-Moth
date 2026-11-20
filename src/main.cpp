#include <Arduino.h>

#include "ble_control.h"
#include "wing_motion.h"
#include "wing_config.h"
#include "serial_config.h"

void setup()
{
    Serial.begin(115200);

    initWingMotion();

    if (!initBLE())
    {
        Serial.println("BLE initialization failed!");
    }

    Serial.println("Setup complete.");
}

void loop()
{
    updateBLE();
    updateWingMotion();
    processSerialConfig();
}
