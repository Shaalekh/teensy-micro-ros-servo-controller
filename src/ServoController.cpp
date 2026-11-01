#include "ServoController.h"
#include <math.h>

namespace {
const uint32_t UPDATE_PERIOD_MS = 20;
const float RADIANS_TO_DEGREES = 57.29577951308232f;
const float DEGREES_TO_RADIANS = 0.017453292519943295f;
}

ServoController::ServoController(const ServoDefinition *definitions, uint8_t count)
    : definitions_(definitions),
      count_(count > SERVO_CONTROLLER_MAX_SERVOS ? SERVO_CONTROLLER_MAX_SERVOS : count),
      lastUpdateMs_(0) {
  for (uint8_t i = 0; i < SERVO_CONTROLLER_MAX_SERVOS; ++i) {
    currentDeg_[i] = 0.0f;
    targetDeg_[i] = 0.0f;
    attached_[i] = false;
  }
}

float ServoController::clampAngle(uint8_t index, float angleDeg) const {
  const ServoDefinition &d = definitions_[index];
  if (angleDeg < d.minAngleDeg) return d.minAngleDeg;
  if (angleDeg > d.maxAngleDeg) return d.maxAngleDeg;
  return angleDeg;
}

uint16_t ServoController::angleToPulseUs(uint8_t index, float angleDeg) const {
  const ServoDefinition &d = definitions_[index];
  float fraction = (angleDeg - d.minAngleDeg) / (d.maxAngleDeg - d.minAngleDeg);
  if (d.reverse) fraction = 1.0f - fraction;
  float pulse = d.minPwmUs + fraction * (float)(d.maxPwmUs - d.minPwmUs);
  if (pulse < d.minPwmUs) pulse = d.minPwmUs;
  if (pulse > d.maxPwmUs) pulse = d.maxPwmUs;
  return (uint16_t)(pulse + 0.5f);
}

void ServoController::writePulse(uint8_t index, uint16_t pulseUs) {
  const ServoDefinition &d = definitions_[index];
  float fraction = (float)(pulseUs - d.minPwmUs) /
                   (float)(d.maxPwmUs - d.minPwmUs);
  if (fraction < 0.0f) fraction = 0.0f;
  if (fraction > 1.0f) fraction = 1.0f;
  servos_[index].write((int)(fraction * 180.0f + 0.5f));
}

bool ServoController::begin() {
  if (definitions_ == 0 || count_ == 0) return false;
  bool allAttached = true;
  for (uint8_t i = 0; i < count_; ++i) {
    const ServoDefinition &d = definitions_[i];
    if (d.name == 0 || d.minAngleDeg >= d.maxAngleDeg || d.minPwmUs >= d.maxPwmUs ||
        !isfinite(d.speedDegPerSec) || d.speedDegPerSec <= 0.0f ||
        !isfinite(d.startupAngleDeg) || d.startupAngleDeg < d.minAngleDeg ||
        d.startupAngleDeg > d.maxAngleDeg) {
      allAttached = false;
      continue;
    }
    attached_[i] = servos_[i].attach(d.pin, d.minPwmUs, d.maxPwmUs);
    if (!attached_[i]) {
      allAttached = false;
      continue;
    }
    currentDeg_[i] = clampAngle(i, d.startupAngleDeg);
    targetDeg_[i] = currentDeg_[i];
    writePulse(i, angleToPulseUs(i, currentDeg_[i]));
  }
  lastUpdateMs_ = millis();
  return allAttached;
}

void ServoController::setTargetsRadians(const float *angles, uint8_t count) {
  if (angles == 0 || count != count_ || definitions_ == 0) return;
  for (uint8_t i = 0; i < count_; ++i) {
    if (!isfinite(angles[i])) continue;
    targetDeg_[i] = clampAngle(i, angles[i] * RADIANS_TO_DEGREES);
  }
}

void ServoController::update() {
  if (definitions_ == 0) return;
  const uint32_t now = millis();
  const uint32_t elapsedMs = (uint32_t)(now - lastUpdateMs_);
  if (elapsedMs < UPDATE_PERIOD_MS) return;
  lastUpdateMs_ = now;
  for (uint8_t i = 0; i < count_; ++i) {
    if (!attached_[i]) continue;
    const float maxStepDeg = definitions_[i].speedDegPerSec * ((float)elapsedMs / 1000.0f);
    float delta = targetDeg_[i] - currentDeg_[i];
    if (delta > maxStepDeg) delta = maxStepDeg;
    if (delta < -maxStepDeg) delta = -maxStepDeg;
    currentDeg_[i] = clampAngle(i, currentDeg_[i] + delta);
    writePulse(i, angleToPulseUs(i, currentDeg_[i]));
  }
}

float ServoController::currentAngleRadians(uint8_t index) const {
  if (index >= count_) return 0.0f;
  return currentDeg_[index] * DEGREES_TO_RADIANS;
}
