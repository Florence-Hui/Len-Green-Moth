#include <Arduino.h>
#include "wing_config.h"

static String inputBuffer = "";

// --------------------------------
// Display configuration
// --------------------------------

static void showConfig()
{
    Serial.println("--- WING CONFIGURATION ---");

    Serial.println("Frequency: " + String(WING_CONFIG.FLAP_FREQUENCY));
    Serial.println("Amplitude: " + String(WING_CONFIG.FLAP_AMPLITUDE));
    Serial.println("Left Neutral: " + String(WING_CONFIG.LEFT_NEUTRAL_ANGLE));
    Serial.println("Right Neutral: " + String(WING_CONFIG.RIGHT_NEUTRAL_ANGLE));

    Serial.print("Waveform: ");
    Serial.println(
        WING_CONFIG.FLAP_WAVEFORM == SINE ? "SINE" : "TRIANGLE"
    );
}

// --------------------------------
// Process completed command
// --------------------------------

static void processCommand(String command)
{
    command.trim();

    if (command == "SHOW")
    {
        showConfig();
        return;
    }

    if (!command.startsWith("SET "))
    {
        Serial.println("Unknown command.");
        return;
    }

    int separator = command.indexOf(' ', 4);

    if (separator == -1)
    {
        Serial.println("Invalid command format.");
        return;
    }

    String parameter = command.substring(4, separator);
    String valueString = command.substring(separator + 1);

    valueString.trim();

    if (parameter == "WAVEFORM")
    {
        valueString.toUpperCase();

        if (valueString == "SINE")
            WING_CONFIG.FLAP_WAVEFORM = SINE;
        else if (valueString == "TRIANGLE")
            WING_CONFIG.FLAP_WAVEFORM = TRIANGLE;
        else
        {
            Serial.println("Unknown waveform.");
            return;
        }

        Serial.println("Waveform updated.");
        return;
    }

    // Validate numerical input
    if (valueString.length() == 0)
    {
        Serial.println("Missing value.");
        return;
    }

    char *endPointer;
    float value = strtof(valueString.c_str(), &endPointer);

    if (endPointer == valueString.c_str() || *endPointer != '\0' || !isfinite(value))
    {
        Serial.println("Invalid numerical value.");
        return;
    }

    if (parameter == "FREQ")
    {
        if (value <= 0.0)
        {
            Serial.println("Frequency must be positive.");
            return;
        }

        WING_CONFIG.FLAP_FREQUENCY = value;
    }
    else if (parameter == "AMP")
    {
        if (value < 0.0 || value > WING_CONFIG.MAX_FLAP_AMPLITUDE)
        {
            Serial.println("Amplitude exceeds permitted range.");
            return;
        }

        WING_CONFIG.FLAP_AMPLITUDE = value;
    }
    else if (parameter == "LEFT_NEUTRAL")
    {
        if (value < 0.0 || value > 180.0)
        {
            Serial.println("Invalid angle.");
            return;
        }

        WING_CONFIG.LEFT_NEUTRAL_ANGLE = value;
    }
    else if (parameter == "RIGHT_NEUTRAL")
    {
        if (value < 0.0 || value > 180.0)
        {
            Serial.println("Invalid angle.");
            return;
        }

        WING_CONFIG.RIGHT_NEUTRAL_ANGLE = value;
    }
    else
    {
        Serial.println("Unknown parameter.");
        return;
    }

    Serial.println("Configuration updated.");
}

// --------------------------------
// Nonblocking serial interface
// --------------------------------

void processSerialConfig()
{
    while (Serial.available() > 0)
    {
        char incoming = Serial.read();

        if (incoming == '\n')
        {
            processCommand(inputBuffer);
            inputBuffer = "";
        }
        else if (incoming != '\r')
        {
            if (inputBuffer.length() < 64)
                inputBuffer += incoming;
            else
            {
                inputBuffer = "";
                Serial.println("Command too long.");
            }
        }
    }
}
