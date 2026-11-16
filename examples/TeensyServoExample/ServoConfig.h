#ifndef SERVO_CONFIG_H
#define SERVO_CONFIG_H

#include <ServoDefinitions.h>

// Starter calibrations: verify wiring, mechanical travel, pulse endpoints,
// direction, and startup pose for every installed servo before use.
static const ServoDefinition SERVO_DEFINITIONS[] = {
  {"servo_0", 0, -90.0f, 90.0f, 1000, 2000, false, 30.0f, 0.0f},
  {"servo_1", 1, -90.0f, 90.0f, 1000, 2000, false, 30.0f, 0.0f},
  {"servo_2", 2, -90.0f, 90.0f, 1000, 2000, false, 30.0f, 0.0f},
  {"servo_3", 3, -90.0f, 90.0f, 1000, 2000, false, 30.0f, 0.0f},
};

static const uint8_t SERVO_COUNT =
    sizeof(SERVO_DEFINITIONS) / sizeof(SERVO_DEFINITIONS[0]);

static const char *const JOINT_NAMES[SERVO_COUNT] = {
  "servo_0", "servo_1", "servo_2", "servo_3"
};

#endif
