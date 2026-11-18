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

const int LEFT_SERVO_MIN_US = 800;
const int LEFT_SERVO_MAX_US = 2200;

const int RIGHT_SERVO_MIN_US = 800;
const int RIGHT_SERVO_MAX_US = 2200;

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

    float STEERING;     //Steering factor for differential flapping (-1.0 to 1.0)

    FlapWaveform FLAP_WAVEFORM;

    float LEFT_AMPLITUDE_MULTIPLIER;
    float RIGHT_AMPLITUDE_MULTIPLIER;

    bool QUICK_RETURN; // If true, the downstroke will be faster than the upstroke
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

// ==========================================
// STEERING CONFIGURATION
// ==========================================

const float MAX_STEERING_INPUT = 1.0;

// Maximum total difference between wing amplitudes,
// expressed as a fraction of the base amplitude.
const float STEERING_DIFF_MAX_FRACTION = 0.5;

const float STEERING_GAIN = 1.0;
const float STEERING_DEADBAND = 0.03;

// Maximum normalized steering change per second
const float STEERING_RAMP_RATE = 3.0;

#endif