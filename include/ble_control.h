#ifndef BLE_CONTROL_H
#define BLE_CONTROL_H

#include <Arduino.h>

// Initialize BLE and begin advertising.
bool initBLE();

// Process incoming BLE configuration packets.
// Returns true when a valid configuration is updated.
bool updateBLE();

#endif
