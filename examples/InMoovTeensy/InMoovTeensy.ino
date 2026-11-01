#include <ServoController.h>
#include <MicroROSController.h>
#include "InMoovServoConfig.h"

ServoController servoController(INMOOV_SERVOS, INMOOV_SERVO_COUNT);

const MicroROSControllerConfig rosConfig = {
  "inmoov_teensy",
  "/inmoov/joint_commands",
  "/inmoov/joint_states",
  INMOOV_JOINT_NAMES,
  42,    // ROS_DOMAIN_ID
  1000,  // command timeout in ms; zero disables timeout hold
  50     // joint-state publish period in ms; zero disables publishing
};

MicroROSController microRosController(servoController, rosConfig);

void setup() {
  servoController.begin();
  microRosController.begin();
}

void loop() {
  microRosController.spinOnce();
  servoController.update();
  delay(2);
}
