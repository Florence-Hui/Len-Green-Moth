#include "wing_config.h"

WingConfig WING_CONFIG = {
    90.0,   // LEFT_NEUTRAL_ANGLE
    90.0,   // RIGHT_NEUTRAL_ANGLE
    0.5,    // FLAP_FREQUENCY
    0.0,   // FLAP_AMPLITUDE
    90.0,   // MAX_FLAP_AMPLITUDE
    0.0,    // STEERING (0.0 = straight ahead)
    SINE,   // FLAP_WAVEFORM
    1.0,    // LEFT_AMPLITUDE_MULTIPLIER (Default 100%)
    1.0,    // RIGHT_AMPLITUDE_MULTIPLIER (Default 100%)
    false   // QUICK_RETURN
};