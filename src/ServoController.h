#ifndef MICROROS_SERVO_CONTROLLER_H
#define MICROROS_SERVO_CONTROLLER_H

#include <Arduino.h>
#include <PWMServo.h>
#include "ServoDefinitions.h"

class ServoController {
 public:
  ServoController(const ServoDefinition *definitions, uint8_t count);
  bool begin();
  void setTargetsRadians(const float *angles, uint8_t count);
  void update();
  uint8_t count() const { return count_; }
  float currentAngleRadians(uint8_t index) const;

 private:
  const ServoDefinition *definitions_;
  uint8_t count_;
  PWMServo servos_[SERVO_CONTROLLER_MAX_SERVOS];
  float currentDeg_[SERVO_CONTROLLER_MAX_SERVOS];
  float targetDeg_[SERVO_CONTROLLER_MAX_SERVOS];
  bool attached_[SERVO_CONTROLLER_MAX_SERVOS];
  uint32_t lastUpdateMs_;

  uint16_t angleToPulseUs(uint8_t index, float angleDeg) const;
  void writePulse(uint8_t index, uint16_t pulseUs);
  float clampAngle(uint8_t index, float angleDeg) const;
};

#endif
