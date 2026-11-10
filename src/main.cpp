#include <Arduino.h>
#include "wing_motion.h"
#include "serial_config.h"


void setup()
{
    initWingMotion();
    Serial.begin(115200);
}


void loop()
{
    processSerialConfig();
    updateWingMotion();
}