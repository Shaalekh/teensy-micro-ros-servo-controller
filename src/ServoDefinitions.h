#ifndef MICROROS_SERVO_CONTROLLER_DEFINITIONS_H
#define MICROROS_SERVO_CONTROLLER_DEFINITIONS_H

#include <stdint.h>

// Fixed storage keeps the controller usable without heap allocation.
static const uint8_t SERVO_CONTROLLER_MAX_SERVOS = 32;

struct ServoDefinition {
  const char *name;
  uint8_t pin;
  float minAngleDeg;
  float maxAngleDeg;
  uint16_t minPwmUs;
  uint16_t maxPwmUs;
  bool reverse;
  float speedDegPerSec;
  float startupAngleDeg;
};

#endif
