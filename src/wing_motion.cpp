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

static float currentSteering = 0.0;

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

    if (WING_CONFIG.QUICK_RETURN)
    {
        // Quick return: downstroke is faster than upstroke
        if (phase < 0.5)
            phase = phase * 4.0/3.0; // Downstroke: 0.0 to 0.66
        else
            phase = 0.5 * 4.0/3.0 + (phase - 0.5) * 0.75; // Upstroke: 0.66 to 1.0
    }

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

    currentSteering = 0.0;

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

        // Reset applied steering while idle
        currentSteering = 0.0;

        writeLeftServoAngle(WING_CONFIG.LEFT_NEUTRAL_ANGLE);
        writeRightServoAngle(WING_CONFIG.RIGHT_NEUTRAL_ANGLE);

        return;
    }

    // --------------------------------
    // Update steering
    // --------------------------------

    float targetSteering = WING_CONFIG.STEERING;

    // Constrain requested steering
    if (targetSteering > MAX_STEERING_INPUT)
        targetSteering = MAX_STEERING_INPUT;

    if (targetSteering < -MAX_STEERING_INPUT)
        targetSteering = -MAX_STEERING_INPUT;

    // Apply deadband
    if (fabsf(targetSteering) < STEERING_DEADBAND)
        targetSteering = 0.0;

    // Gradually move current steering toward target
    float maxSteeringChange =
        STEERING_RAMP_RATE * deltaTime;

    float steeringDifference =
        targetSteering - currentSteering;

    if (steeringDifference > maxSteeringChange)
        currentSteering += maxSteeringChange;

    else if (steeringDifference < -maxSteeringChange)
        currentSteering -= maxSteeringChange;

    else
        currentSteering = targetSteering;

    // Update normalized phase.

    float phaseStep = frequency * deltaTime;

    phase += phaseStep;

    phase = phase - floorf(phase);

    // --------------------------------
    // Choose waveform
    // --------------------------------

    float wave;

    if (WING_CONFIG.FLAP_WAVEFORM == TRIANGLE)
        wave = triangleWave(phase);
    else
        wave = sineWave(phase);


    // --------------------------------
    // Calculate steering differential
    // --------------------------------

    // Maximum permitted amplitude difference
    float maxWingDelta =
        amplitude * STEERING_DIFF_MAX_FRACTION * 0.5;

    // Available amplitude headroom
    float upperHeadroom =
        WING_CONFIG.MAX_FLAP_AMPLITUDE - amplitude;

    float lowerHeadroom = amplitude;

    // Prevent either wing from exceeding amplitude limits
    if (upperHeadroom < maxWingDelta)
        maxWingDelta = upperHeadroom;

    if (lowerHeadroom < maxWingDelta)
        maxWingDelta = lowerHeadroom;

    // Calculate steering-induced amplitude change
    float wingDelta =
        currentSteering * STEERING_GAIN * maxWingDelta;

    // --------------------------------
    // Calculate individual wing motion
    // --------------------------------

    float leftAmplitude = amplitude + wingDelta;
    float rightAmplitude = amplitude - wingDelta;

    float leftFlapDisplacement =
        leftAmplitude * wave;

    float rightFlapDisplacement =
        rightAmplitude * wave;

    // --------------------------------
    // Calculate servo angles
    // --------------------------------

    float leftAngle =
        WING_CONFIG.LEFT_NEUTRAL_ANGLE +
        leftFlapDisplacement;

    float rightAngle =
        WING_CONFIG.RIGHT_NEUTRAL_ANGLE -
        rightFlapDisplacement;

    // --------------------------------
    // Write servo commands
    // --------------------------------

    writeLeftServoAngle(leftAngle);
    writeRightServoAngle(rightAngle);
}