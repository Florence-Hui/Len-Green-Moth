#ifndef WING_CONFIG_H
#define WING_CONFIG_H

// ==========================================
// SERVO CONFIGURATION
// ==========================================

const int LEFT_SERVO_PIN = 9;
const int RIGHT_SERVO_PIN = 10;

// --------------------------------
// Servo angle calibration
// --------------------------------

// Angles corresponding to the calibrated pulse-width endpoints.
// These are the angles used by the motion controller.

const float LEFT_SERVO_MIN_ANGLE = 0.0;
const float LEFT_SERVO_MAX_ANGLE = 180.0;

const float RIGHT_SERVO_MIN_ANGLE = 0.0;
const float RIGHT_SERVO_MAX_ANGLE = 180.0;


// --------------------------------
// Servo pulse-width calibration
// --------------------------------

// For particular servos.
// Units: microseconds (us).

const int LEFT_SERVO_MIN_US = 500;
const int LEFT_SERVO_MAX_US = 2500;

const int RIGHT_SERVO_MIN_US = 500;
const int RIGHT_SERVO_MAX_US = 2500;

// ==========================================
// FLAPPING CONFIGURATION
// ==========================================
// ==========================================
// WAVEFORM CONFIGURATION
// ==========================================

enum FlapWaveform
{
    TRIANGLE,
    SINE
};

struct WingConfig
{
    float LEFT_NEUTRAL_ANGLE;
    float RIGHT_NEUTRAL_ANGLE;
    float FLAP_FREQUENCY;
    float FLAP_AMPLITUDE;
    float MAX_FLAP_AMPLITUDE;
    FlapWaveform FLAP_WAVEFORM;
};

// Declare configuration object
extern WingConfig WING_CONFIG;


// ==========================================
// CONTROL CONFIGURATION
// ==========================================

// Minimum frequency treated as active (Hz)
const float MIN_ACTIVE_FREQUENCY = 0.05;

// Minimum amplitude treated as active (degrees)
const float MIN_ACTIVE_AMPLITUDE = 0.1;

// Minimum pulse-width change before sending another servo command.
const int SERVO_WRITE_DEADBAND_US = 2;

#endif