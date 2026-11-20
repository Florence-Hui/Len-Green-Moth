#include "ble_control.h"
#include "wing_config.h"

#include <ArduinoBLE.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// --------------------------------
// BLE identifiers
// --------------------------------

static BLEService ornithopterService(
    "19B10000-E8F2-537E-4F6C-D104768A1214"
);

// Maxiumum packet length: 20 characters
static BLECharacteristic commandCharacteristic(
    "19B10001-E8F2-537E-4F6C-D104768A1214",
    BLERead | BLEWrite,
    20,
    false
);

static char receivedPacket[21];

// --------------------------------
// Parameter IDs
// --------------------------------

enum ConfigParameter : int {
    LEFT_NEUTRAL = 0,
    RIGHT_NEUTRAL = 1,
    FLAP_FREQUENCY = 2,
    FLAP_AMPLITUDE = 3,
    MAX_FLAP_AMPLITUDE = 4,
    STEERING = 5,
    FLAP_WAVEFORM = 6,
    LEFT_MULTIPLIER = 7,
    RIGHT_MULTIPLIER = 8,
    QUICK_RETURN = 9
};

// --------------------------------
// Parameter validation and updates
// --------------------------------

static bool updateParameter(int id, float value)
{
    if (!isfinite(value))
        return false;
    
    switch (id)
    {
        case LEFT_NEUTRAL:
            if (value < LEFT_SERVO_MIN_ANGLE ||
                 value > LEFT_SERVO_MAX_ANGLE)
                return false;
            WING_CONFIG.LEFT_NEUTRAL_ANGLE = value;
            break;

        case RIGHT_NEUTRAL:
            if (value < RIGHT_SERVO_MIN_ANGLE ||
                 value > RIGHT_SERVO_MAX_ANGLE)
                return false;
            WING_CONFIG.RIGHT_NEUTRAL_ANGLE = value;
            break;

        case FLAP_FREQUENCY:
            if (value <= 0.0)
                return false;
            WING_CONFIG.FLAP_FREQUENCY = value;
            break;

        case FLAP_AMPLITUDE:
            if (value < 0.0 ||
                 value > WING_CONFIG.MAX_FLAP_AMPLITUDE)
                return false;
            WING_CONFIG.FLAP_AMPLITUDE = value;
            break;

        case MAX_FLAP_AMPLITUDE:
            if (value < 0.0)
                return false;
            WING_CONFIG.MAX_FLAP_AMPLITUDE = value;

            if (WING_CONFIG.FLAP_AMPLITUDE > value)
                WING_CONFIG.FLAP_AMPLITUDE = value;
            break;

        case STEERING:
            if (value < -MAX_STEERING_INPUT || value > MAX_STEERING_INPUT)
                return false;
            WING_CONFIG.STEERING = value;
            break;

        case FLAP_WAVEFORM:
            if (value == 0.0)
                WING_CONFIG.FLAP_WAVEFORM = TRIANGLE;
            else if (value == 1.0)
                WING_CONFIG.FLAP_WAVEFORM = SINE;
            else
                return false;
            break;

        case LEFT_MULTIPLIER:
            if (value < 0.0)
                return false;
            WING_CONFIG.LEFT_AMPLITUDE_MULTIPLIER = value;
            break;

        case RIGHT_MULTIPLIER:
            if (value < 0.0)
                return false;
            WING_CONFIG.RIGHT_AMPLITUDE_MULTIPLIER = value;
            break;

        case QUICK_RETURN:
            if (value == 0.0)
                WING_CONFIG.QUICK_RETURN = false;
            else if (value == 1.0)
                WING_CONFIG.QUICK_RETURN = true;
            else
                return false;
            break;

        default:
            return false; // Unknown parameter
    }

    return true; // Parameter updated successfully
};

// --------------------------------
// Parse incoming text packet
// --------------------------------

// Packet format: "<id>,<value>"

static bool processPacket(const char* packet)
{
    char packetBuffer[21];

    strncpy(packetBuffer, packet, 20);
    packetBuffer[20] = '\0';

    char* separator = strchr(packetBuffer, ',');

    if (separator == nullptr)
        return false;

    *separator = '\0';

    char* idEnd;
    char* valueEnd;

    long id = strtol(packetBuffer, &idEnd, 10);
    float value = strtof(separator + 1, &valueEnd);

    if (idEnd == packetBuffer || *idEnd != '\0')
        return false;

    if (valueEnd == separator + 1 || *valueEnd != '\0')
        return false;

    if (id < LEFT_NEUTRAL || id > QUICK_RETURN)
        return false;

    return updateParameter((int)id, value);
};

// --------------------------------
// BLE initialization
// --------------------------------

bool initBLE()
{
    Serial.println("Initializing BLE...");

    if(!BLE.begin())
    {
        Serial.println("BLE initialization failed!");
        return false;
    }

    BLE.setLocalName("LenGreenMoth");
    BLE.setAdvertisedService(ornithopterService);

    ornithopterService.addCharacteristic(commandCharacteristic);

    BLE.addService(ornithopterService);

    commandCharacteristic.writeValue("Ready");

    BLE.advertise();

    Serial.println("BLE READY");

    return true;
}

// --------------------------------
// BLE update loop
// --------------------------------

bool updateBLE()
{
    // Process pending BLE events
    BLE.poll();

    if (!commandCharacteristic.written())
        return false;
    
    int packetLength = commandCharacteristic.valueLength();

    if (packetLength <= 0 || packetLength > 20)
    {
        Serial.println("Invalid packet length received.");
        return false;
    }

    commandCharacteristic.readValue(
        (uint8_t*)receivedPacket,
        packetLength
    );

    receivedPacket[packetLength] = '\0';

    Serial.print("BLE packet received: ");
    Serial.println(receivedPacket);

    if(processPacket(receivedPacket))
    {
        Serial.println("Configuration updated!");
        return true;
    }
    else
    {
        Serial.print("Invalid packet received from BLE: ");
        Serial.println(receivedPacket);
        return false;
    }
}