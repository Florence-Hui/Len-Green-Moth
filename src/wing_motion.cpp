#include "wing_motion.h"
#include "wing_config.h"

#include <Servo.h>
#include <math.h>

// --------------------------------
// Servo objects
// --------------------------------

static Servo leftServo;
static Servo rightServo;


// --------------------------------
// Motion state
// --------------------------------

static float phase = 0.75;

static unsigned long previousMicros = 0;

static int lastLeftPulse = -1;
static int lastRightPulse = -1;

static bool initialized = false;

// --------------------------------
// Unpack Servo Output Configuration
// --------------------------------

// float LEFT_NEUTRAL_ANGLE = WING_CONFIG.LEFT_NEUTRAL_ANGLE;
// float RIGHT_NEUTRAL_ANGLE = WING_CONFIG.RIGHT_NEUTRAL_ANGLE;
// float FLAP_FREQUENCY = WING_CONFIG.FLAP_FREQUENCY;
// float FLAP_AMPLITUDE = WING_CONFIG.FLAP_AMPLITUDE;
// float MAX_FLAP_AMPLITUDE = WING_CONFIG.MAX_FLAP_AMPLITUDE;
// FlapWaveform FLAP_WAVEFORM = WING_CONFIG.FLAP_WAVEFORM;

// --------------------------------
// Angle and pulse conversion
// --------------------------------

static float clampAngle(
    float angle,
    float minAngle,
    float maxAngle)
{
    if (angle < minAngle)
        return minAngle;

    if (angle > maxAngle)
        return maxAngle;

    return angle;
}


static int angleToPulse(
    float angle,
    float minAngle,
    float maxAngle,
    int minPulse,
    int maxPulse)
{
    angle = clampAngle(angle, minAngle, maxAngle);

    float fraction =
        (angle - minAngle) / (maxAngle - minAngle);

    float pulse =
        minPulse + fraction * (maxPulse - minPulse);

    return (int)roundf(pulse);
}


// --------------------------------
// Waveform generation
// --------------------------------

static float triangleWave(float phase)
{
    phase = phase - floorf(phase);

    if (phase < 0.5)
        return -1.0 + (4.0 * phase);

    return 3.0 - (4.0 * phase);
}


static float sineWave(float phase)
{
    phase = phase - floorf(phase);

    return sinf((2.0 * PI * phase) - (PI / 2.0));
}


// --------------------------------
// Servo output
// --------------------------------

static void writeLeftServoAngle(float angle)
{
    int pulse = angleToPulse(
        angle,
        LEFT_SERVO_MIN_ANGLE,
        LEFT_SERVO_MAX_ANGLE,
        LEFT_SERVO_MIN_US,
        LEFT_SERVO_MAX_US
    );

    if (abs(pulse - lastLeftPulse) > SERVO_WRITE_DEADBAND_US)
    {
        leftServo.writeMicroseconds(pulse);
        lastLeftPulse = pulse;
    }
}


static void writeRightServoAngle(float angle)
{
    int pulse = angleToPulse(
        angle,
        RIGHT_SERVO_MIN_ANGLE,
        RIGHT_SERVO_MAX_ANGLE,
        RIGHT_SERVO_MIN_US,
        RIGHT_SERVO_MAX_US
    );

    if (abs(pulse - lastRightPulse) > SERVO_WRITE_DEADBAND_US)
    {
        rightServo.writeMicroseconds(pulse);
        lastRightPulse = pulse;
    }
}


// --------------------------------
// Initialization
// --------------------------------

void initWingMotion()
{
    leftServo.attach(
        LEFT_SERVO_PIN,
        LEFT_SERVO_MIN_US,
        LEFT_SERVO_MAX_US
    );

    rightServo.attach(
        RIGHT_SERVO_PIN,
        RIGHT_SERVO_MIN_US,
        RIGHT_SERVO_MAX_US
    );

    // Move both servos to their neutral positions.

    writeLeftServoAngle(WING_CONFIG.LEFT_NEUTRAL_ANGLE);
    writeRightServoAngle(WING_CONFIG.RIGHT_NEUTRAL_ANGLE);

    previousMicros = micros();

    phase = 0.75;

    initialized = true;
}


// --------------------------------
// Update wing motion
// --------------------------------

void updateWingMotion()
{
    if (!initialized)
        return;

    // Calculate elapsed time.

    unsigned long now = micros();

    float deltaTime =
        (now - previousMicros) / 1000000.0;

    previousMicros = now;


    // Read user-defined motion parameters.

    float frequency = WING_CONFIG.FLAP_FREQUENCY;
    float amplitude = WING_CONFIG.FLAP_AMPLITUDE;


    // Limit amplitude.

    if (amplitude > WING_CONFIG.MAX_FLAP_AMPLITUDE)
        amplitude = WING_CONFIG.MAX_FLAP_AMPLITUDE;

    if (amplitude < 0.0)
        amplitude = 0.0;


    // Check whether flapping should be active.

    if (frequency < MIN_ACTIVE_FREQUENCY ||
        amplitude < MIN_ACTIVE_AMPLITUDE)
    {
        phase = 0.75;

        writeLeftServoAngle(WING_CONFIG.LEFT_NEUTRAL_ANGLE);
        writeRightServoAngle(WING_CONFIG.RIGHT_NEUTRAL_ANGLE);

        return;
    }


    // Update normalized phase.

    float phaseStep = frequency * deltaTime;

    phase += phaseStep;

    phase = phase - floorf(phase);


    // Generate waveform.

    float wave;

    if (WING_CONFIG.FLAP_WAVEFORM == TRIANGLE)
        wave = triangleWave(phase);
    else
        wave = sineWave(phase);


    // Calculate angular displacement.

    float flapDisplacement = amplitude * wave;


    // Calculate left and right servo angles.

    float leftAngle =
        WING_CONFIG.LEFT_NEUTRAL_ANGLE + flapDisplacement;

    float rightAngle =
        WING_CONFIG.RIGHT_NEUTRAL_ANGLE - flapDisplacement;


    // Send angles to servo output functions.
    // Conversion to microseconds happens internally.

    writeLeftServoAngle(leftAngle);
    writeRightServoAngle(rightAngle);
}