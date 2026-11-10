#ifndef WING_MOTION_H
#define WING_MOTION_H

#include <Arduino.h>
#include <Servo.h>
#include "wing_config.h"


// ==========================================
// INITIALIZATION
// ==========================================

void initWingMotion();


// ==========================================
// MAIN CONTROL LOOP
// ==========================================

void updateWingMotion();


// ==========================================
// WAVEFORM GENERATION
// ==========================================

// float triangleWave(float phase);

// float sineWave(float phase);

#endif